#include "TabManager.h"
#include "TabItemWidget.h"
#include "TabCommands.h"
#include "TabOpenCommand.h"
#include "TabCloseCommand.h"
#include "TabSaveCommand.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "FileHelpers.h"
#include "Styling/SlateIconFinder.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

UTabManager::UTabManager()
{
	InitGroupColors();
}

void UTabManager::Initialize()
{
	// 创建 CommandInvoker 并设置自己为操作接口
	CommandInvoker = NewObject<UTabCommandInvoker>(this);
	CommandInvoker->SetOperations(this);

	// 初始刷新
	RefreshTabs();
}

void UTabManager::Shutdown()
{
	StopAutoRefresh();
	ClearSelection();
	RegisteredWidgets.Empty();
}

void UTabManager::InitGroupColors()
{
	GroupColors.Empty();
	GroupColors.Add(TEXT("Blueprint"), FLinearColor(0.29f, 0.56f, 0.85f, 1.0f));
	GroupColors.Add(TEXT("WidgetBlueprint"), FLinearColor(0.61f, 0.35f, 0.71f, 1.0f));
	GroupColors.Add(TEXT("AnimBlueprint"), FLinearColor(0.75f, 0.22f, 0.17f, 1.0f));
	GroupColors.Add(TEXT("Material"), FLinearColor(0.15f, 0.68f, 0.38f, 1.0f));
	GroupColors.Add(TEXT("MaterialInstanceConstant"), FLinearColor(0.18f, 0.80f, 0.44f, 1.0f));
	GroupColors.Add(TEXT("Texture2D"), FLinearColor(0.90f, 0.49f, 0.13f, 1.0f));
	GroupColors.Add(TEXT("StaticMesh"), FLinearColor(0.20f, 0.60f, 0.86f, 1.0f));
	GroupColors.Add(TEXT("SkeletalMesh"), FLinearColor(0.10f, 0.74f, 0.61f, 1.0f));
	GroupColors.Add(TEXT("AnimSequence"), FLinearColor(0.91f, 0.30f, 0.24f, 1.0f));
	GroupColors.Add(TEXT("AnimMontage"), FLinearColor(0.91f, 0.30f, 0.24f, 1.0f));
	GroupColors.Add(TEXT("PhysicsAsset"), FLinearColor(0.85f, 0.65f, 0.30f, 1.0f));  // 金色/橙色
	GroupColors.Add(TEXT("SoundWave"), FLinearColor(0.95f, 0.61f, 0.07f, 1.0f));
	GroupColors.Add(TEXT("NiagaraSystem"), FLinearColor(0.56f, 0.27f, 0.68f, 1.0f));
	GroupColors.Add(TEXT("World"), FLinearColor(0.17f, 0.24f, 0.31f, 1.0f));
	GroupColors.Add(TEXT("DataTable"), FLinearColor(0.09f, 0.63f, 0.52f, 1.0f));
	GroupColors.Add(TEXT("CurveFloat"), FLinearColor(0.95f, 0.77f, 0.06f, 1.0f));
	GroupColors.Add(TEXT("EditorTools"), FLinearColor(0.65f, 0.65f, 0.65f, 1.0f)); // 银灰色
	GroupColors.Add(TEXT("Other"), FLinearColor(0.50f, 0.55f, 0.55f, 1.0f));
}

// ============ ITabOperations 接口实现 ============

bool UTabManager::ActivateTab(const FString& TabId)
{
	// 🔧 先尝试作为资产编辑器激活
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem)
	{
		TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
		for (UObject* Asset : EditedAssets)
		{
			if (Asset && Asset->GetPathName() == TabId)
			{
				AssetEditorSubsystem->OpenEditorForAsset(Asset);

				// 查找并广播标签信息
				for (const FEditorTabInfo& Tab : CachedTabs)
				{
					if (Tab.TabId == TabId)
					{
						OnTabActivated.Broadcast(Tab);
						break;
					}
				}
				return true;
			}
		}

		// 尝试加载并打开
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TabId));

		if (AssetData.IsValid())
		{
			AssetEditorSubsystem->OpenEditorForAsset(AssetData.GetAsset());
			return true;
		}
	}

	// 🆕 尝试作为工具窗口激活
	TSharedPtr<FTabManager> GlobalTabManager = FGlobalTabmanager::Get();
	if (GlobalTabManager.IsValid())
	{
		// 使用 FindExistingLiveTab 查找
		TSharedPtr<SDockTab> ExistingTab = GlobalTabManager->FindExistingLiveTab(FTabId(FName(*TabId)));

		if (ExistingTab.IsValid())
		{
			// 找到匹配的标签，激活它
			ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
			ExistingTab->DrawAttention();
			return true;
		}
	}

	return false;
}

bool UTabManager::CloseTab(const FString& TabId)
{
	// 🔧 先尝试作为资产编辑器关闭
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (AssetEditorSubsystem)
	{
		TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

		for (UObject* Asset : EditedAssets)
		{
			if (Asset && Asset->GetPathName() == TabId)
			{
				AssetEditorSubsystem->CloseAllEditorsForAsset(Asset);
				return true;
			}
		}
	}

	// 🆕 尝试作为工具窗口关闭
	TSharedPtr<FTabManager> GlobalTabManager = FGlobalTabmanager::Get();
	if (GlobalTabManager.IsValid())
	{
		// 使用 FindExistingLiveTab 查找
		TSharedPtr<SDockTab> ExistingTab = GlobalTabManager->FindExistingLiveTab(FTabId(FName(*TabId)));

		if (ExistingTab.IsValid())
		{
			// 找到匹配的标签，请求关闭
			ExistingTab->RequestCloseTab();
			return true;
		}
	}

	return false;
}

bool UTabManager::SaveAsset(const FString& AssetPath)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return false;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

	for (UObject* Asset : EditedAssets)
	{
		if (Asset && Asset->GetPathName() == AssetPath && Asset->GetPackage()->IsDirty())
		{
			TArray<UPackage*> PackagesToSave;
			PackagesToSave.Add(Asset->GetPackage());
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
			return true;
		}
	}
	return false;
}

void UTabManager::BrowseToAsset(const FString& AssetPath)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(AssetPath));

	if (AssetData.IsValid())
	{
		TArray<FAssetData> Assets;
		Assets.Add(AssetData);
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(Assets);
	}
}

bool UTabManager::AssignTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId)
{
	// 查找或创建自定义群组
	FCustomTabGroup* TargetGroup = nullptr;
	for (FCustomTabGroup& Group : CustomGroups)
	{
		if (Group.GroupId == GroupId)
		{
			TargetGroup = &Group;
			break;
		}
	}

	if (!TargetGroup)
	{
		// 创建新群组
		FCustomTabGroup NewGroup;
		NewGroup.GroupId = GroupId;
		NewGroup.GroupName = GroupId;
		NewGroup.Color = FLinearColor::MakeRandomColor();
		CustomGroups.Add(NewGroup);
		TargetGroup = &CustomGroups.Last();
	}

	// 从其他群组移除标签
	for (const FEditorTabInfo& Tab : Tabs)
	{
		for (FCustomTabGroup& Group : CustomGroups)
		{
			Group.TabIds.Remove(Tab.TabId);
		}
		TargetGroup->TabIds.AddUnique(Tab.TabId);
	}

	RefreshTabs();
	return true;
}

int32 UTabManager::GetTabIndex(const FString& TabId) const
{
	return FindTabIndex(TabId);
}

bool UTabManager::MoveTabToIndex(const FString& TabId, int32 NewIndex)
{
	int32 CurrentIndex = FindTabIndex(TabId);
	if (CurrentIndex < 0 || NewIndex < 0 || NewIndex >= CachedTabs.Num())
	{
		return false;
	}

	FEditorTabInfo Tab = CachedTabs[CurrentIndex];
	CachedTabs.RemoveAt(CurrentIndex);
	CachedTabs.Insert(Tab, NewIndex);

	// 更新显示顺序
	for (int32 i = 0; i < CachedTabs.Num(); ++i)
	{
		CachedTabs[i].DisplayOrder = i;
	}

	OnTabListChanged.Broadcast();
	return true;
}

// ============ 标签数据管理 ============

void UTabManager::RefreshTabs()
{
	InternalRefresh();
}

TArray<FEditorTabInfo> UTabManager::GenerateTabList()
{
	TArray<FEditorTabInfo> Tabs;

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return Tabs;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

	int32 Index = 0;

	// 1️⃣ 获取资产编辑器标签
	for (UObject* Asset : EditedAssets)
	{
		if (!Asset) continue;

		FEditorTabInfo TabInfo;
		TabInfo.TabId = Asset->GetPathName();
		TabInfo.DisplayName = Asset->GetName();
		TabInfo.AssetPath = Asset->GetPathName();
		TabInfo.AssetClassName = Asset->GetClass()->GetName();
		TabInfo.AssetType = GetAssetTypeDisplayName(Asset->GetClass());
		TabInfo.bIsDirty = Asset->GetPackage()->IsDirty();
		TabInfo.bIsActive = false;
		TabInfo.GroupId = TabInfo.AssetType;
		TabInfo.GroupColor = GetAssetTypeColor(TabInfo.AssetType);
		TabInfo.DisplayOrder = Index++;

		// 检查自定义群组分配
		for (const FCustomTabGroup& CustomGroup : CustomGroups)
		{
			if (CustomGroup.TabIds.Contains(TabInfo.TabId))
			{
				TabInfo.CustomGroupId = CustomGroup.GroupId;
				TabInfo.GroupId = CustomGroup.GroupId;
				TabInfo.GroupColor = CustomGroup.Color;
				break;
			}
		}

		// 获取图标
		const FSlateBrush* IconBrush = FSlateIconFinder::FindIconBrushForClass(Asset->GetClass());
		if (IconBrush)
		{
			TabInfo.IconBrush = *IconBrush;
		}

		Tabs.Add(TabInfo);
	}

	// 2️⃣ 🆕 检查预定义的工具窗口
	// 由于 UE 没有提供枚举所有打开标签的 API，使用预定义列表
	TArray<FName> KnownToolTabIds = {
		FName("EditorSettings"), // Editor Preferences
		FName("WidgetReflector"), // Widget Reflector
		FName("OutputLog"), // Output Log
		FName("MessageLog"), // Message Log
		FName("DeviceOutputLog"), // Device OutputLog
		FName("PluginsEditor"), // Plugins
		FName("ProjectSettings"), // Project Settings
		FName("Modules"), // Modules
		FName("LevelEditorPixelInspector"), // Level EditorPixelInspector
		FName("NiagaraDebugger"), // Niagara Debugger
		FName("VisualLogger"), // Visual Logger
		FName("CollisionAnalyzerApp"), // Collision AnalyzerApp
		FName("ChaosVisualDebuggerTab"), // Chaos VisualDebuggerTab
		FName("LevelEditor"), // Level Editor
		FName("BlueprintDebugger"), // Blueprint Debugger
		FName("ReferenceViewer"), // Reference Viewer

	};

	// 已处理的 TabId（用于去重）
	TSet<FString> ProcessedTabIds;
	for (const FEditorTabInfo& Tab : Tabs)
	{
		ProcessedTabIds.Add(Tab.TabId);
	}

	TSharedPtr<FTabManager> GlobalTabManager = FGlobalTabmanager::Get();

	for (const FName& TabId : KnownToolTabIds)
	{
		// 使用 FindExistingLiveTab 检查标签是否打开
		TSharedPtr<SDockTab> ExistingTab = GlobalTabManager->FindExistingLiveTab(FTabId(TabId));

		if (ExistingTab.IsValid())
		{
			FString TabIdStr = TabId.ToString();

			// 去重检查
			if (ProcessedTabIds.Contains(TabIdStr))
			{
				continue;
			}

			// 创建工具窗口标签信息
			FEditorTabInfo ToolTabInfo;
			ToolTabInfo.TabId = TabIdStr;
			ToolTabInfo.DisplayName = ExistingTab->GetTabLabel().ToString();
			ToolTabInfo.AssetPath = TEXT("");
			ToolTabInfo.AssetClassName = TEXT("EditorTool");
			ToolTabInfo.AssetType = TEXT("EditorTools");
			ToolTabInfo.bIsDirty = false;
			ToolTabInfo.bIsActive = ExistingTab->IsForeground();
			ToolTabInfo.GroupId = TEXT("EditorTools");
			ToolTabInfo.GroupColor = GetAssetTypeColor(TEXT("EditorTools"));
			ToolTabInfo.DisplayOrder = Index++;

			const FSlateBrush* IconBrush = FAppStyle::GetBrush(TEXT("Icons.Settings"));
			ToolTabInfo.IconBrush = *IconBrush;

			Tabs.Add(ToolTabInfo);
			ProcessedTabIds.Add(TabIdStr);
		}
	}

	return Tabs;
}

bool UTabManager::HasTabsChanged(const TArray<FEditorTabInfo>& NewTabs)
{
	if (CachedTabs.Num() != NewTabs.Num()) return true;

	for (int32 i = 0; i < CachedTabs.Num(); ++i)
	{
		const FEditorTabInfo& Old = CachedTabs[i];
		const FEditorTabInfo& New = NewTabs[i];

		if (Old.TabId != New.TabId) return true;
		if (Old.bIsDirty != New.bIsDirty) return true;
		if (Old.GroupId != New.GroupId) return true;
	}

	return false;
}

void UTabManager::InternalRefresh()
{
	TArray<FEditorTabInfo> NewTabs = GenerateTabList();

	if (HasTabsChanged(NewTabs))
	{
		CachedTabs = NewTabs;

		// 移除不存在的标签的选择
		TSet<FString> ValidIds;
		for (const FEditorTabInfo& Tab : CachedTabs)
		{
			ValidIds.Add(Tab.TabId);
		}

		TSet<FString> ToRemove;
		for (const FString& Id : SelectedTabIds)
		{
			if (!ValidIds.Contains(Id))
			{
				ToRemove.Add(Id);
			}
		}

		for (const FString& Id : ToRemove)
		{
			SelectedTabIds.Remove(Id);
		}

		OnTabListChanged.Broadcast();
	}
}

TArray<FTabGroupInfo> UTabManager::GetGroupedTabs() const
{
	TMap<FString, FTabGroupInfo> GroupMap;

	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		FString GroupId = Tab.GroupId.IsEmpty() ? TEXT("Other") : Tab.GroupId;

		if (!GroupMap.Contains(GroupId))
		{
			FTabGroupInfo NewGroup;
			NewGroup.GroupId = GroupId;
			NewGroup.GroupName = GroupId;
			NewGroup.Color = GetAssetTypeColor(GroupId);
			NewGroup.bExpanded = true;

			// 检查是否是自定义群组
			for (const FCustomTabGroup& CustomGroup : CustomGroups)
			{
				if (CustomGroup.GroupId == GroupId)
				{
					NewGroup.GroupName = CustomGroup.GroupName;
					NewGroup.Color = CustomGroup.Color;
					NewGroup.bIsCustomGroup = true;
					break;
				}
			}

			GroupMap.Add(GroupId, NewGroup);
		}
		GroupMap[GroupId].Tabs.Add(Tab);
	}

	TArray<FTabGroupInfo> Groups;
	GroupMap.GenerateValueArray(Groups);
	return Groups;
}

TArray<FEditorTabInfo> UTabManager::GetTabsByType(const FString& AssetType) const
{
	TArray<FEditorTabInfo> FilteredTabs;
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		if (Tab.AssetType == AssetType) FilteredTabs.Add(Tab);
	}
	return FilteredTabs;
}

bool UTabManager::ShouldUseGroupedDisplay() const
{
	if (!bEnableGrouping) return false;

	TSet<FString> UniqueGroups;
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		FString GroupId = Tab.GroupId.IsEmpty() ? TEXT("Other") : Tab.GroupId;
		UniqueGroups.Add(GroupId);
		if (UniqueGroups.Num() > 1) return true;
	}

	return false;
}

// ============ 选择管理 ============

void UTabManager::HandleItemClick(const FEditorTabInfo& TabInfo, bool bShiftDown, bool bCtrlDown)
{
	if (bShiftDown && !SelectionAnchorId.IsEmpty())
	{
		SelectRange(TabInfo);
	}
	else if (bCtrlDown)
	{
		ToggleSelection(TabInfo);
	}
	else
	{
		SelectSingle(TabInfo);
	}
}

void UTabManager::SelectSingle(const FEditorTabInfo& TabInfo)
{
	SelectedTabIds.Empty();
	SelectedTabIds.Add(TabInfo.TabId);
	SelectionAnchorId = TabInfo.TabId;

	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabManager::SelectRange(const FEditorTabInfo& TabInfo)
{
	int32 AnchorIndex = FindTabIndex(SelectionAnchorId);
	int32 TargetIndex = FindTabIndex(TabInfo.TabId);

	if (AnchorIndex < 0 || TargetIndex < 0) return;

	SelectedTabIds.Empty();

	int32 StartIndex = FMath::Min(AnchorIndex, TargetIndex);
	int32 EndIndex = FMath::Max(AnchorIndex, TargetIndex);

	for (int32 i = StartIndex; i <= EndIndex; ++i)
	{
		if (CachedTabs.IsValidIndex(i))
		{
			SelectedTabIds.Add(CachedTabs[i].TabId);
		}
	}

	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabManager::ToggleSelection(const FEditorTabInfo& TabInfo)
{
	if (SelectedTabIds.Contains(TabInfo.TabId))
	{
		SelectedTabIds.Remove(TabInfo.TabId);
	}
	else
	{
		SelectedTabIds.Add(TabInfo.TabId);
		SelectionAnchorId = TabInfo.TabId;
	}

	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabManager::ClearSelection()
{
	SelectedTabIds.Empty();
	SelectionAnchorId.Empty();
	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabManager::SelectAll()
{
	SelectedTabIds.Empty();
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		SelectedTabIds.Add(Tab.TabId);
	}
	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

bool UTabManager::IsSelected(const FEditorTabInfo& TabInfo) const
{
	return SelectedTabIds.Contains(TabInfo.TabId);
}

TArray<FEditorTabInfo> UTabManager::GetSelectedTabs() const
{
	TArray<FEditorTabInfo> Result;
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		if (SelectedTabIds.Contains(Tab.TabId))
		{
			Result.Add(Tab);
		}
	}
	return Result;
}

void UTabManager::NotifySelectionChanged()
{
	OnSelectionChanged.Broadcast(GetSelectedTabs());
}

int32 UTabManager::FindTabIndex(const FString& TabId) const
{
	for (int32 i = 0; i < CachedTabs.Num(); ++i)
	{
		if (CachedTabs[i].TabId == TabId)
		{
			return i;
		}
	}
	return -1;
}

// ============ 高层操作（通过 CommandInvoker） ============

bool UTabManager::OpenTab(const FEditorTabInfo& Tab)
{
	if (!CommandInvoker) return false;
	UTabOpenCommand* Command = UTabOpenCommand::Create(this, Tab);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::OpenTabs(const TArray<FEditorTabInfo>& Tabs)
{
	if (!CommandInvoker) return false;
	UTabOpenCommand* Command = UTabOpenCommand::Create(this, Tabs);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::CloseTabByInfo(const FEditorTabInfo& Tab)
{
	if (!CommandInvoker) return false;
	UTabCloseCommand* Command = UTabCloseCommand::Create(this, Tab);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::CloseTabs(const TArray<FEditorTabInfo>& Tabs)
{
	if (!CommandInvoker) return false;
	UTabCloseCommand* Command = UTabCloseCommand::Create(this, Tabs);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::CloseSelectedTabs()
{
	if (!HasSelection()) return false;
	return CloseTabs(GetSelectedTabs());
}

bool UTabManager::SaveTab(const FEditorTabInfo& Tab)
{
	if (!CommandInvoker) return false;
	UTabSaveCommand* Command = UTabSaveCommand::Create(this, Tab);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::SaveTabs(const TArray<FEditorTabInfo>& Tabs)
{
	if (!CommandInvoker) return false;
	UTabSaveCommand* Command = UTabSaveCommand::Create(this, Tabs);
	return CommandInvoker->ExecuteCommand(Command);
}

void UTabManager::SaveAllDirtyTabs()
{
	TArray<FEditorTabInfo> DirtyTabs;
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		if (Tab.bIsDirty)
		{
			DirtyTabs.Add(Tab);
		}
	}

	if (DirtyTabs.Num() > 0)
	{
		SaveTabs(DirtyTabs);
	}
}

void UTabManager::CloseAllTabs()
{
	CloseTabs(CachedTabs);
}

bool UTabManager::BrowseToAssetByInfo(const FEditorTabInfo& Tab)
{
	if (!CommandInvoker) return false;
	UTabBrowseCommand* Command = UTabBrowseCommand::Create(this, Tab);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::AddTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId)
{
	if (!CommandInvoker) return false;
	UTabAddToGroupCommand* Command = UTabAddToGroupCommand::Create(this, Tabs, GroupId);
	return CommandInvoker->ExecuteCommand(Command);
}

bool UTabManager::Undo()
{
	if (!CommandInvoker) return false;
	return CommandInvoker->UndoLastCommand();
}

bool UTabManager::CanUndo() const
{
	return CommandInvoker && CommandInvoker->CanUndo();
}

// ============ 自定义群组管理 ============

void UTabManager::CreateCustomGroup(const FString& GroupName, FLinearColor GroupColor)
{
	FCustomTabGroup NewGroup;
	NewGroup.GroupId = GroupName;
	NewGroup.GroupName = GroupName;
	NewGroup.Color = GroupColor;
	CustomGroups.Add(NewGroup);
}

void UTabManager::DeleteCustomGroup(const FString& GroupId)
{
	CustomGroups.RemoveAll([&GroupId](const FCustomTabGroup& Group)
	{
		return Group.GroupId == GroupId;
	});
	RefreshTabs();
}

// ============ Widget 注册 ============

void UTabManager::RegisterItemWidget(UTabItemWidget* Widget)
{
	if (Widget)
	{
		RegisteredWidgets.AddUnique(Widget);
	}
}

void UTabManager::UnregisterItemWidget(UTabItemWidget* Widget)
{
	RegisteredWidgets.Remove(Widget);
}

void UTabManager::UpdateAllWidgetVisuals()
{
	for (TWeakObjectPtr<UTabItemWidget>& WidgetPtr : RegisteredWidgets)
	{
		if (UTabItemWidget* Widget = WidgetPtr.Get())
		{
			bool bSelected = SelectedTabIds.Contains(Widget->TabData.TabId);
			Widget->SetIsSelected(bSelected);
		}
	}
}

// ============ 自动刷新 ============

void UTabManager::StartAutoRefresh(float IntervalSeconds)
{
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		GEditor->GetEditorWorldContext().World()->GetTimerManager().SetTimer(
			AutoRefreshTimerHandle,
			FTimerDelegate::CreateUObject(this, &UTabManager::InternalRefresh),
			IntervalSeconds, true
		);
	}
}

void UTabManager::StopAutoRefresh()
{
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		GEditor->GetEditorWorldContext().World()->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
	}
}

// ============ 工具方法 ============

FLinearColor UTabManager::GetAssetTypeColor(const FString& AssetType) const
{
	if (GroupColors.Contains(AssetType)) return GroupColors[AssetType];
	return GroupColors[TEXT("Other")];
}

FSlateBrush UTabManager::GetAssetTypeBrush(const FString& AssetClassName) const
{
	UClass* AssetClass = const_cast<UTabManager*>(this)->FindAssetClassByName(AssetClassName);

	if (AssetClass)
	{
		const FSlateBrush* Brush = FSlateIconFinder::FindIconBrushForClass(AssetClass);
		if (Brush)
		{
			return *Brush;
		}
	}

	const FSlateBrush* DefaultBrush = FSlateIconFinder::FindIconBrushForClass(UObject::StaticClass());
	if (DefaultBrush)
	{
		return *DefaultBrush;
	}

	return FSlateBrush();
}

FString UTabManager::GetAssetTypeDisplayName(UClass* AssetClass)
{
	if (!AssetClass) return TEXT("Other");

	FString ClassName = AssetClass->GetName();

	static TMap<FString, FString> TypeNameMap = {
		{TEXT("Blueprint"), TEXT("Blueprint")},
		{TEXT("WidgetBlueprint"), TEXT("WidgetBlueprint")},
		{TEXT("AnimBlueprint"), TEXT("AnimBlueprint")},
		{TEXT("Material"), TEXT("Material")},
		{TEXT("MaterialInstanceConstant"), TEXT("MaterialInstanceConstant")},
		{TEXT("Texture2D"), TEXT("Texture2D")},
		{TEXT("StaticMesh"), TEXT("StaticMesh")},
		{TEXT("SkeletalMesh"), TEXT("SkeletalMesh")},
		{TEXT("AnimSequence"), TEXT("AnimSequence")},
		{TEXT("AnimMontage"), TEXT("AnimMontage")},
		{TEXT("PhysicsAsset"), TEXT("PhysicsAsset")},  
		{TEXT("SoundWave"), TEXT("SoundWave")},
		{TEXT("SoundCue"), TEXT("SoundWave")},
		{TEXT("NiagaraSystem"), TEXT("NiagaraSystem")},
		{TEXT("World"), TEXT("World")},
		{TEXT("DataTable"), TEXT("DataTable")},
		{TEXT("CurveFloat"), TEXT("CurveFloat")},
		{TEXT("EditorUtilityWidgetBlueprint"), TEXT("WidgetBlueprint")},
	};

	if (TypeNameMap.Contains(ClassName)) return TypeNameMap[ClassName];
	return TEXT("Other");
}

UClass* UTabManager::FindAssetClassByName(const FString& ClassName)
{
	static TMap<FString, FString> ClassPathMap = {
		{TEXT("Blueprint"), TEXT("/Script/Engine.Blueprint")},
		{TEXT("WidgetBlueprint"), TEXT("/Script/UMGEditor.WidgetBlueprint")},
		{TEXT("AnimBlueprint"), TEXT("/Script/Engine.AnimBlueprint")},
		{TEXT("Material"), TEXT("/Script/Engine.Material")},
		{TEXT("MaterialInstanceConstant"), TEXT("/Script/Engine.MaterialInstanceConstant")},
		{TEXT("Texture2D"), TEXT("/Script/Engine.Texture2D")},
		{TEXT("StaticMesh"), TEXT("/Script/Engine.StaticMesh")},
		{TEXT("SkeletalMesh"), TEXT("/Script/Engine.SkeletalMesh")},
		{TEXT("AnimSequence"), TEXT("/Script/Engine.AnimSequence")},
		{TEXT("AnimMontage"), TEXT("/Script/Engine.AnimMontage")},
		{TEXT("PhysicsAsset"), TEXT("/Script/Engine.PhysicsAsset")},  // 🆕 新增
		{TEXT("SoundWave"), TEXT("/Script/Engine.SoundWave")},
		{TEXT("SoundCue"), TEXT("/Script/Engine.SoundCue")},
		{TEXT("NiagaraSystem"), TEXT("/Script/Niagara.NiagaraSystem")},
		{TEXT("World"), TEXT("/Script/Engine.World")},
		{TEXT("DataTable"), TEXT("/Script/Engine.DataTable")},
		{TEXT("CurveFloat"), TEXT("/Script/Engine.CurveFloat")},
		{TEXT("EditorUtilityWidgetBlueprint"), TEXT("/Script/Blutility.EditorUtilityWidgetBlueprint")},
	};

	if (ClassPathMap.Contains(ClassName))
	{
		return FindObject<UClass>(nullptr, *ClassPathMap[ClassName]);
	}

	FString FullPath = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
	return FindObject<UClass>(nullptr, *FullPath);
}
