#include "EUW_Windows.h"

#include "EditorUtilitySubsystem.h"
#include "TabManager.h"
#include "TabGroupWidget.h"
#include "TabItemWidget.h"
#include "TabContextMenu.h"
#include "TabGroupSubMenu.h"
#include "TabPopupManager.h"
#include "Components/VerticalBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

UEUW_Windows::UEUW_Windows()
{
}

void UEUW_Windows::NativeConstruct()
{
	Super::NativeConstruct();

	// 创建 TabManager
	TabManager = NewObject<UTabManager>(this);
	TabManager->Initialize();

	// 绑定 Manager 事件
	TabManager->OnTabListChanged.AddDynamic(this, &UEUW_Windows::RebuildUI);

	// 启动自动刷新
	TabManager->StartAutoRefresh(0.15f);

	// 初始构建
	RebuildUI();
}

void UEUW_Windows::NativeDestruct()
{
	CloseContextMenu();

	if (TabManager)
	{
		TabManager->Shutdown();
	}

	Super::NativeDestruct();
}

// ============ UI 构建 ============

void UEUW_Windows::RefreshUI()
{
	if (TabManager)
	{
		TabManager->RefreshTabs();
	}
}

void UEUW_Windows::RebuildUI()
{
	if (!ItemContainer || !TabItemClass) return;

	ItemContainer->ClearChildren();

	if (TabManager && TabManager->ShouldUseGroupedDisplay() && TabGroupClass)
	{
		BuildGroupedList();
	}
	else
	{
		BuildFlatList();
	}
}

void UEUW_Windows::BuildFlatList()
{
	if (!TabManager) return;

	TArray<FEditorTabInfo> AllTabs = TabManager->GetAllTabs();

	for (const FEditorTabInfo& Tab : AllTabs)
	{
		UTabItemWidget* TabWidget = CreateWidget<UTabItemWidget>(this, TabItemClass);
		if (TabWidget)
		{
			// 设置 Manager 引用
			TabWidget->SetTabManager(TabManager);
			TabWidget->SetTabData(Tab);

			// 绑定事件（用于右键菜单等UI操作）
			TabWidget->OnRightClicked.AddDynamic(this, &UEUW_Windows::HandleItemRightClicked);

			ItemContainer->AddChildToVerticalBox(TabWidget);
		}
	}
}

void UEUW_Windows::BuildGroupedList()
{
	if (!TabManager) return;

	TArray<FTabGroupInfo> Groups = TabManager->GetGroupedTabs();

	for (const FTabGroupInfo& Group : Groups)
	{
		UTabGroupWidget* GroupWidget = CreateWidget<UTabGroupWidget>(this, TabGroupClass);
		if (GroupWidget)
		{
			GroupWidget->SetGroupData(Group);
			GroupWidget->SetTabManager(TabManager);

			// 绑定群组事件
			GroupWidget->OnItemRightClicked.AddDynamic(this, &UEUW_Windows::HandleGroupItemRightClicked);

			ItemContainer->AddChildToVerticalBox(GroupWidget);

			// 创建子项
			for (const FEditorTabInfo& Tab : Group.Tabs)
			{
				UTabItemWidget* TabWidget = CreateWidget<UTabItemWidget>(this, TabItemClass);
				if (TabWidget)
				{
					TabWidget->SetTabManager(TabManager);
					TabWidget->SetTabData(Tab);

					// 🔧 修复：绑定右键点击事件
					TabWidget->OnRightClicked.AddDynamic(this, &UEUW_Windows::HandleItemRightClicked);

					GroupWidget->AddChildWidget(TabWidget);
				}
			}
		}
	}
}

// ============ 事件处理 ============

void UEUW_Windows::HandleItemClicked(const FEditorTabInfo& TabInfo)
{
	// TabItemWidget 现在直接调用 TabManager，这里只用于蓝图扩展
}

void UEUW_Windows::HandleItemClosed(const FEditorTabInfo& TabInfo)
{
	// TabItemWidget 现在直接调用 TabManager，这里只用于蓝图扩展
}

void UEUW_Windows::HandleItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition)
{
	if (!TabManager) return;

	TArray<FEditorTabInfo> TabsForMenu;

	// 如果是多选且当前标签被选中，使用所有选中的标签
	if (TabManager->IsMultiSelection() && TabManager->IsSelected(TabInfo))
	{
		TabsForMenu = TabManager->GetSelectedTabs();
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

// 放在任何你方便调用的地方，比如 EUW_Windows.cpp 中添加一个测试函数

void UEUW_Windows::TestPopupWindow()
{
	FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();

	TSharedRef<SWindow> TestWindow = SNew(SWindow)
		.Type(EWindowType::Menu)
		.IsPopupWindow(true)
		.SizingRule(ESizingRule::Autosized)
		.ScreenPosition(CursorPos)
		.FocusWhenFirstShown(true)
		.ActivationPolicy(EWindowActivationPolicy::Always)
		[
			// 简单的测试内容：一个带背景的文本
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Menu.Background"))
			.Padding(10.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString("Test Popup Window"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(SButton)
					.Text(FText::FromString("Click Me"))
					.OnClicked_Lambda([]()
					{
						UE_LOG(LogTemp, Warning, TEXT("Button Clicked!"));
						return FReply::Handled();
					})
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString("This is a borderless window"))
				]
			]
		];

	FSlateApplication::Get().AddWindow(TestWindow);

	UE_LOG(LogTemp, Warning, TEXT("Popup window created at: %s"), *CursorPos.ToString());
}

// ============ 右键菜单 ============

void UEUW_Windows::ShowContextMenu(const TArray<FEditorTabInfo>& Tabs, FVector2D ScreenPosition)
{
	CloseContextMenu();

	// 广播事件（用于蓝图扩展）
	OnContextMenuRequested.Broadcast(Tabs, ScreenPosition);

	// 使用 TabPopupManager 创建菜单
	if (ContextMenuClass && TabManager)
	{
		// 🔧 使用 TabPopupManager 显示弹窗
		ActiveContextMenu = Cast<UTabContextMenu>(
			UTabPopupManager::ShowPopup(
				ContextMenuClass,
				ScreenPosition,
				FVector2D::ZeroVector, // 自动大小
				true // 点击外部关闭
			)
		);

		if (ActiveContextMenu)
		{
			// 传递 GroupSubMenuClass
			ActiveContextMenu->GroupSubMenuClass = GroupSubMenuClass;

			// 初始化菜单
			ActiveContextMenu->InitializeMenu(TabManager, Tabs);

			UE_LOG(LogTemp, Log, TEXT("[EUW_Windows] Context menu shown at (%.1f, %.1f)"),
			       ScreenPosition.X, ScreenPosition.Y);
		}

		FSlateApplication::Get().AddWindow(ActiveContextMenu);
	}
}

void UEUW_Windows::CloseContextMenu()
{
	if (ActiveContextMenu)
	{
		UTabPopupManager::ClosePopup(ActiveContextMenu);
		ActiveContextMenu = nullptr;
	}
}


void UEUW_Windows::ShowContextMenuForSelection(FVector2D ScreenPosition)
{
	if (TabManager && TabManager->HasSelection())
	{
		ShowContextMenu(TabManager->GetSelectedTabs(), ScreenPosition);
	}
}

// ============ 便捷访问 ============

TArray<FEditorTabInfo> UEUW_Windows::GetAllTabs() const
{
	return TabManager ? TabManager->GetAllTabs() : TArray<FEditorTabInfo>();
}

TArray<FTabGroupInfo> UEUW_Windows::GetGroupedTabs() const
{
	return TabManager ? TabManager->GetGroupedTabs() : TArray<FTabGroupInfo>();
}

TArray<FEditorTabInfo> UEUW_Windows::GetSelectedTabs() const
{
	return TabManager ? TabManager->GetSelectedTabs() : TArray<FEditorTabInfo>();
}

bool UEUW_Windows::IsGroupingEnabled() const
{
	return TabManager ? TabManager->bEnableGrouping : false;
}

void UEUW_Windows::SetEnableGrouping(bool bEnable)
{
	if (TabManager)
	{
		TabManager->bEnableGrouping = bEnable;
		RebuildUI();
	}
}
