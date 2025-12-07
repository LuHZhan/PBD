#include "EUW_Windows.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "FileHelpers.h"
#include "TabGroupWidget.h"
#include "TabItemWidget.h"
#include "TabSelectionManager.h"
#include "TabCommands.h"
#include "TabContextMenu.h"
#include "TabGroupSubMenu.h"
#include "Components/VerticalBox.h"
#include "Styling/SlateIconFinder.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

UEUW_Windows::UEUW_Windows()
{
	InitGroupColors();
}

void UEUW_Windows::NativeConstruct()
{
	Super::NativeConstruct();

	// Create selection manager
	SelectionManager = NewObject<UTabSelectionManager>(this);

	// Create command invoker
	CommandInvoker = NewObject<UTabCommandInvoker>(this);

	// Bind default UI generation logic
	OnTabsChanged.AddDynamic(this, &UEUW_Windows::RebuildUI);

	RefreshTabs();
	StartAutoRefresh(1.0f);
}

void UEUW_Windows::NativeDestruct()
{
	StopAutoRefresh();
	CloseContextMenu();
	Super::NativeDestruct();
}

void UEUW_Windows::InitGroupColors()
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
	GroupColors.Add(TEXT("SoundWave"), FLinearColor(0.95f, 0.61f, 0.07f, 1.0f));
	GroupColors.Add(TEXT("NiagaraSystem"), FLinearColor(0.56f, 0.27f, 0.68f, 1.0f));
	GroupColors.Add(TEXT("World"), FLinearColor(0.17f, 0.24f, 0.31f, 1.0f));
	GroupColors.Add(TEXT("DataTable"), FLinearColor(0.09f, 0.63f, 0.52f, 1.0f));
	GroupColors.Add(TEXT("CurveFloat"), FLinearColor(0.95f, 0.77f, 0.06f, 1.0f));
	GroupColors.Add(TEXT("Other"), FLinearColor(0.50f, 0.55f, 0.55f, 1.0f));
}

TArray<FEditorTabInfo> UEUW_Windows::RefreshAllOpenTabs()
{
	InternalRefresh();
	return CachedTabs;
}

TArray<FEditorTabInfo> UEUW_Windows::GetAllOpenTabs()
{
	return CachedTabs;
}

TArray<FEditorTabInfo> UEUW_Windows::GenerateTabList()
{
	TArray<FEditorTabInfo> Tabs;

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return Tabs;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

	int32 Index = 0;
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

		// Check for custom group assignment
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

		// Get icon brush for asset class
		const FSlateBrush* IconBrush = FSlateIconFinder::FindIconBrushForClass(Asset->GetClass());
		if (IconBrush)
		{
			TabInfo.IconBrush = *IconBrush;
		}

		Tabs.Add(TabInfo);
	}

	return Tabs;
}

bool UEUW_Windows::HasTabsChanged(const TArray<FEditorTabInfo>& NewTabs)
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

TArray<FTabGroupInfo> UEUW_Windows::GetGroupedTabs()
{
	TArray<FEditorTabInfo> AllTabs = CachedTabs;
	TMap<FString, FTabGroupInfo> GroupMap;

	for (const FEditorTabInfo& Tab : AllTabs)
	{
		FString GroupId = Tab.GroupId.IsEmpty() ? TEXT("Other") : Tab.GroupId;

		if (!GroupMap.Contains(GroupId))
		{
			FTabGroupInfo NewGroup;
			NewGroup.GroupId = GroupId;
			NewGroup.GroupName = GroupId;
			NewGroup.Color = GetAssetTypeColor(GroupId);
			NewGroup.bExpanded = true;

			// Check if it's a custom group
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

bool UEUW_Windows::ShouldUseGroupedDisplay() const
{
	if (!bEnableGrouping) return false;

	TSet<FString> UniqueGroups;
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		FString GroupId = Tab.GroupId.IsEmpty() ? TEXT("Other") : Tab.GroupId;
		UniqueGroups.Add(GroupId);

		// More than one group, use grouped display
		if (UniqueGroups.Num() > 1) return true;
	}

	// Only one or zero groups, no need for grouping
	return false;
}

TArray<FEditorTabInfo> UEUW_Windows::GetTabsByType(const FString& AssetType)
{
	TArray<FEditorTabInfo> FilteredTabs;
	for (const FEditorTabInfo& Tab : CachedTabs)
	{
		if (Tab.AssetType == AssetType) FilteredTabs.Add(Tab);
	}
	return FilteredTabs;
}

bool UEUW_Windows::ActivateTab(const FString& TabId)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return false;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
	for (UObject* Asset : EditedAssets)
	{
		if (Asset && Asset->GetPathName() == TabId)
		{
			AssetEditorSubsystem->OpenEditorForAsset(Asset);

			// Find and broadcast tab info
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

	// Try to load and open
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TabId));

	if (AssetData.IsValid())
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(AssetData.GetAsset());
		return true;
	}

	return false;
}

bool UEUW_Windows::CloseTab(const FString& TabId)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return false;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

	for (UObject* Asset : EditedAssets)
	{
		if (Asset && Asset->GetPathName() == TabId)
		{
			AssetEditorSubsystem->CloseAllEditorsForAsset(Asset);
			return true;
		}
	}
	return false;
}

void UEUW_Windows::BrowseToAsset(const FString& AssetPath)
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

bool UEUW_Windows::SaveAsset(const FString& AssetPath)
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

void UEUW_Windows::CloseAllTabs()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

	for (UObject* Asset : EditedAssets)
	{
		if (Asset)
		{
			AssetEditorSubsystem->CloseAllEditorsForAsset(Asset);
		}
	}
	RefreshTabs();
}

void UEUW_Windows::SaveAllDirtyAssets()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return;

	TArray<UObject*> EditedAssets = AssetEditorSubsystem->GetAllEditedAssets();
	TArray<UPackage*> PackagesToSave;

	for (UObject* Asset : EditedAssets)
	{
		if (Asset && Asset->GetPackage()->IsDirty())
		{
			PackagesToSave.Add(Asset->GetPackage());
		}
	}

	if (PackagesToSave.Num() > 0)
	{
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
		RefreshTabs();
	}
}

void UEUW_Windows::RefreshTabs()
{
	InternalRefresh();
}

void UEUW_Windows::SetEnableGrouping(bool bEnable)
{
	if (bEnableGrouping != bEnable)
	{
		bEnableGrouping = bEnable;
		RebuildUI();
	}
}

void UEUW_Windows::StartAutoRefresh(float IntervalSeconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			AutoRefreshTimerHandle,
			FTimerDelegate::CreateUObject(this, &UEUW_Windows::InternalRefresh),
			IntervalSeconds, true
		);
	}
}

void UEUW_Windows::StopAutoRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoRefreshTimerHandle);
	}
}

void UEUW_Windows::InternalRefresh()
{
	TArray<FEditorTabInfo> NewTabs = GenerateTabList();

	if (HasTabsChanged(NewTabs))
	{
		CachedTabs = NewTabs;

		// Update selection manager's tab list
		if (SelectionManager)
		{
			SelectionManager->UpdateTabList(CachedTabs);
		}

		OnTabsChanged.Broadcast();
	}
}

void UEUW_Windows::RebuildUI()
{
	if (!ItemContainer) return;

	ItemContainer->ClearChildren();

	if (!TabItemClass) return;

	// Check if we should use grouped display
	if (ShouldUseGroupedDisplay() && TabGroupClass)
	{
		// Grouped display mode
		TArray<FTabGroupInfo> Groups = GetGroupedTabs();

		for (const FTabGroupInfo& Group : Groups)
		{
			UTabGroupWidget* GroupWidget = CreateWidget<UTabGroupWidget>(this, TabGroupClass);
			if (GroupWidget)
			{
				GroupWidget->SetGroupData(Group);
				GroupWidget->SetSelectionManager(SelectionManager);

				// Bind group events
				GroupWidget->OnItemClicked.AddDynamic(this, &UEUW_Windows::HandleGroupItemClicked);
				GroupWidget->OnItemClosed.AddDynamic(this, &UEUW_Windows::HandleGroupItemClosed);
				GroupWidget->OnItemRightClicked.AddDynamic(this, &UEUW_Windows::HandleGroupItemRightClicked);

				ItemContainer->AddChildToVerticalBox(GroupWidget);

				for (const FEditorTabInfo& Tab : Group.Tabs)
				{
					UTabItemWidget* TabWidget = CreateWidget<UTabItemWidget>(this, TabItemClass);
					if (TabWidget)
					{
						TabWidget->SetTabData(Tab);
						GroupWidget->AddChildWidget(TabWidget);
					}
				}
			}
		}
	}
	else
	{
		// Flat display mode - show all items directly without grouping
		for (const FEditorTabInfo& Tab : CachedTabs)
		{
			UTabItemWidget* TabWidget = CreateWidget<UTabItemWidget>(this, TabItemClass);
			if (TabWidget)
			{
				TabWidget->SetSelectionManager(SelectionManager);
				TabWidget->SetTabData(Tab);

				// Bind events directly
				TabWidget->OnClicked.AddDynamic(this, &UEUW_Windows::HandleItemClicked);
				TabWidget->OnClosed.AddDynamic(this, &UEUW_Windows::HandleItemClosed);
				TabWidget->OnRightClicked.AddDynamic(this, &UEUW_Windows::HandleItemRightClicked);

				ItemContainer->AddChildToVerticalBox(TabWidget);
			}
		}
	}
}

// ============ NEW: Event Handlers ============

void UEUW_Windows::HandleItemClicked(const FEditorTabInfo& TabInfo)
{
	ActivateTab(TabInfo.TabId);
}

void UEUW_Windows::HandleItemClosed(const FEditorTabInfo& TabInfo)
{
	// If multi-selection, close all selected
	if (SelectionManager && SelectionManager->IsMultiSelection() && SelectionManager->IsSelected(TabInfo))
	{
		TArray<FEditorTabInfo> SelectedTabs = SelectionManager->GetSelectedTabs();
		for (const FEditorTabInfo& Tab : SelectedTabs)
		{
			CloseTab(Tab.TabId);
		}
		SelectionManager->ClearSelection();
	}
	else
	{
		CloseTab(TabInfo.TabId);
	}
}

void UEUW_Windows::HandleItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition)
{
	TArray<FEditorTabInfo> TabsForMenu;

	// If multi-selection and this tab is selected, use all selected tabs
	if (SelectionManager && SelectionManager->IsMultiSelection() && SelectionManager->IsSelected(TabInfo))
	{
		TabsForMenu = SelectionManager->GetSelectedTabs();
	}
	else
	{
		TabsForMenu.Add(TabInfo);
	}

	ShowContextMenu(TabsForMenu, ScreenPosition);
}

void UEUW_Windows::HandleGroupItemClicked(const FTabGroupInfo& GroupData, const FEditorTabInfo& TabInfo)
{
	HandleItemClicked(TabInfo);
}

void UEUW_Windows::HandleGroupItemClosed(const FTabGroupInfo& GroupData, const FEditorTabInfo& TabInfo)
{
	HandleItemClosed(TabInfo);
}

void UEUW_Windows::HandleGroupItemRightClicked(const FTabGroupInfo& GroupData, const FEditorTabInfo& TabInfo, FVector2D ScreenPosition)
{
	HandleItemRightClicked(TabInfo, ScreenPosition);
}

// ============ NEW: Custom Group Methods ============

bool UEUW_Windows::AssignTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId)
{
	// Find or create custom group
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
		// Create new custom group
		FCustomTabGroup NewGroup;
		NewGroup.GroupId = GroupId;
		NewGroup.GroupName = GroupId;
		NewGroup.Color = FLinearColor::MakeRandomColor();
		CustomGroups.Add(NewGroup);
		TargetGroup = &CustomGroups.Last();
	}

	// Remove tabs from other custom groups
	for (const FEditorTabInfo& Tab : Tabs)
	{
		for (FCustomTabGroup& Group : CustomGroups)
		{
			Group.TabIds.Remove(Tab.TabId);
		}

		// Add to target group
		TargetGroup->TabIds.AddUnique(Tab.TabId);
	}

	// Refresh to update UI
	RefreshTabs();
	return true;
}

int32 UEUW_Windows::GetTabIndex(const FString& TabId) const
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

bool UEUW_Windows::MoveTabToIndex(const FString& TabId, int32 NewIndex)
{
	int32 CurrentIndex = GetTabIndex(TabId);
	if (CurrentIndex < 0 || NewIndex < 0 || NewIndex >= CachedTabs.Num())
	{
		return false;
	}

	// Reorder in cached tabs
	FEditorTabInfo Tab = CachedTabs[CurrentIndex];
	CachedTabs.RemoveAt(CurrentIndex);
	CachedTabs.Insert(Tab, NewIndex);

	// Update display orders
	for (int32 i = 0; i < CachedTabs.Num(); ++i)
	{
		CachedTabs[i].DisplayOrder = i;
	}

	RebuildUI();
	return true;
}

void UEUW_Windows::CreateCustomGroup(const FString& GroupName)
{
	FCustomTabGroup NewGroup;
	NewGroup.GroupId = GroupName;
	NewGroup.GroupName = GroupName;
	NewGroup.Color = FLinearColor::MakeRandomColor();
	CustomGroups.Add(NewGroup);
}

void UEUW_Windows::DeleteCustomGroup(const FString& GroupId)
{
	CustomGroups.RemoveAll([&GroupId](const FCustomTabGroup& Group) {
		return Group.GroupId == GroupId;
	});

	RefreshTabs();
}

// ============ NEW: Context Menu Methods ============

void UEUW_Windows::ShowContextMenu(const TArray<FEditorTabInfo>& Tabs, FVector2D ScreenPosition)
{
	CloseContextMenu();

	// Broadcast event for Blueprint handling
	OnContextMenuRequested.Broadcast(Tabs, ScreenPosition);

	// Create context menu if class is set
	if (ContextMenuClass)
	{
		ActiveContextMenu = CreateWidget<UTabContextMenu>(this, ContextMenuClass);
		if (ActiveContextMenu)
		{
			ActiveContextMenu->InitializeMenu(this, Tabs);
			ActiveContextMenu->AddToViewport(100);
			ActiveContextMenu->ShowAtPosition(ScreenPosition);
		}
	}
}

void UEUW_Windows::ShowContextMenuForSelection(FVector2D ScreenPosition)
{
	if (SelectionManager && SelectionManager->HasSelection())
	{
		ShowContextMenu(SelectionManager->GetSelectedTabs(), ScreenPosition);
	}
}

void UEUW_Windows::CloseContextMenu()
{
	if (ActiveContextMenu)
	{
		ActiveContextMenu->CloseMenu();
		ActiveContextMenu->RemoveFromParent();
		ActiveContextMenu = nullptr;
	}
}

// ============ NEW: Selection Methods ============

TArray<FEditorTabInfo> UEUW_Windows::GetSelectedTabs() const
{
	if (SelectionManager)
	{
		return SelectionManager->GetSelectedTabs();
	}
	return TArray<FEditorTabInfo>();
}

void UEUW_Windows::ClearSelection()
{
	if (SelectionManager)
	{
		SelectionManager->ClearSelection();
	}
}

void UEUW_Windows::SelectAllTabs()
{
	if (SelectionManager)
	{
		SelectionManager->SelectAll();
	}
}

// ============ Existing helper methods (unchanged) ============

UClass* UEUW_Windows::FindAssetClassByName(const FString& ClassName)
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
	UClass* FoundClass = FindObject<UClass>(nullptr, *FullPath);

	return FoundClass;
}

UTexture2D* UEUW_Windows::GetAssetTypeIcon(const FString& AssetClassName)
{
	UClass* AssetClass = FindAssetClassByName(AssetClassName);

	if (AssetClass)
	{
		const FSlateBrush* Brush = FSlateIconFinder::FindIconBrushForClass(AssetClass);
		if (Brush && Brush->GetResourceObject())
		{
			return Cast<UTexture2D>(Brush->GetResourceObject());
		}
	}

	const FSlateBrush* DefaultBrush = FSlateIconFinder::FindIconBrushForClass(UObject::StaticClass());
	if (DefaultBrush && DefaultBrush->GetResourceObject())
	{
		return Cast<UTexture2D>(DefaultBrush->GetResourceObject());
	}

	return nullptr;
}

FSlateBrush UEUW_Windows::GetAssetTypeBrush(const FString& AssetClassName)
{
	UClass* AssetClass = FindAssetClassByName(AssetClassName);

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

TMap<FString, FLinearColor> UEUW_Windows::GetGroupColorMap()
{
	return GroupColors;
}

FLinearColor UEUW_Windows::ParseHexColor(const FString& HexColor)
{
	if (HexColor.StartsWith(TEXT("#")))
	{
		return FLinearColor(FColor::FromHex(HexColor.RightChop(1)));
	}
	return FLinearColor::White;
}

FString UEUW_Windows::GetAssetTypeDisplayName(UClass* AssetClass)
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

FLinearColor UEUW_Windows::GetAssetTypeColor(const FString& AssetType)
{
	if (GroupColors.Contains(AssetType)) return GroupColors[AssetType];
	return GroupColors[TEXT("Other")];
}
