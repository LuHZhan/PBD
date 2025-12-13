#include "TabContextMenu.h"
#include "TabManager.h"
#include "TabGroupSubMenu.h"
#include "TabPopupManager.h"
#include "CommonLuBtn.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTabContextMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定所有按钮事件
	BindButtons();
}

void UTabContextMenu::NativePreConstruct()
{
	Super::NativePreConstruct();
	Btns.Add(OpenBtn);
	Btns.Add(CloseBtn);
	Btns.Add(SaveBtn);
	Btns.Add(GroupOperatorBtn);
	for (auto Element : Btns)
	{
		Element->ApplyStyle();
	}
}

void UTabContextMenu::NativeDestruct()
{
	// 关闭子菜单
	if (ActiveGroupSubMenu)
	{
		UTabPopupManager::ClosePopup(ActiveGroupSubMenu);
		ActiveGroupSubMenu = nullptr;
	}

	Super::NativeDestruct();
}

void UTabContextMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bEnableFocusLostDetection)
	{
		return;
	}

	// 累计时间
	AccumulatedTime += InDeltaTime;

	// 每 0.1 秒检测一次
	if (AccumulatedTime >= FocusCheckDelay)
	{
		AccumulatedTime = 0.0f;

		// 🔧 检查1：检查 UE 编辑器是否失去焦点
		TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
		if (!ActiveWindow.IsValid() || !FSlateApplication::Get().IsActive())
		{
			// UE 编辑器失去焦点（切换到其他应用）
			UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Editor lost focus, closing menu"));
			CloseMenu();
			return;
		}

		// 🔧 检查2：检查鼠标是否在菜单区域外且按下了鼠标
		FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

		// 检查鼠标是否在菜单范围内
		bool bIsMouseInside = MyGeometry.IsUnderLocation(MousePosition);

		// 检查是否有鼠标按下
		bool bIsMouseButtonDown = FSlateApplication::Get().GetPressedMouseButtons().Num() > 0;

		// 如果鼠标在菜单外且按下了按钮，关闭菜单
		if (!bIsMouseInside && bIsMouseButtonDown)
		{
			UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Mouse clicked outside menu, closing"));
			CloseMenu();
		}
	}
}

void UTabContextMenu::BindButtons()
{
	// 绑定 LuBtn 的点击委托
	if (OpenBtn)
	{
		OpenBtn->OnClickedDelegate_Post.AddDynamic(this, &UTabContextMenu::HandleOpenClicked);
	}

	if (SaveBtn)
	{
		SaveBtn->OnClickedDelegate_Post.AddDynamic(this, &UTabContextMenu::HandleSaveClicked);
	}

	if (CloseBtn)
	{
		CloseBtn->OnClickedDelegate_Post.AddDynamic(this, &UTabContextMenu::HandleCloseClicked);
	}

	if (GroupOperatorBtn)
	{
		GroupOperatorBtn->OnClickedDelegate_Post.AddDynamic(this, &UTabContextMenu::HandleGroupClicked);
	}
}

void UTabContextMenu::InitializeMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs)
{
	TabManagerRef = Manager;
	TargetTabs = Tabs;
	bIsMultiSelection = Tabs.Num() > 1;

	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Initialized with %d tabs, IsMultiSelection=%d"),
	       Tabs.Num(), bIsMultiSelection);
}

void UTabContextMenu::CloseMenu()
{
	// 先关闭子菜单
	if (ActiveGroupSubMenu)
	{
		UTabPopupManager::ClosePopup(ActiveGroupSubMenu);
		ActiveGroupSubMenu = nullptr;
	}

	// 关闭自己（通过 PopupManager）
	UTabPopupManager::ClosePopup(this);

	SetVisibility(ESlateVisibility::Collapsed);
	OnMenuClosed.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Menu closed"));
}

// ============ 按钮点击处理 ============

void UTabContextMenu::HandleOpenClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Open clicked"));

	if (!TabManagerRef.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] TabManager is invalid"));
		return;
	}

	TArray<FEditorTabInfo> ValidTabs = GetValidTabs();

	if (ValidTabs.Num() > 0)
	{
		TabManagerRef->OpenTabs(ValidTabs);
		OnMenuItemClicked.Broadcast(TEXT("Open"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] No valid tabs to open"));
	}

	CloseMenu();
}

void UTabContextMenu::HandleSaveClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Save clicked"));

	if (!TabManagerRef.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] TabManager is invalid"));
		return;
	}

	TArray<FEditorTabInfo> ValidTabs = GetValidTabs();

	if (ValidTabs.Num() > 0)
	{
		TabManagerRef->SaveTabs(ValidTabs);
		OnMenuItemClicked.Broadcast(TEXT("Save"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] No valid tabs to save"));
	}

	CloseMenu();
}

void UTabContextMenu::HandleCloseClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Close clicked"));

	if (!TabManagerRef.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] TabManager is invalid"));
		return;
	}

	TArray<FEditorTabInfo> ValidTabs = GetValidTabs();

	if (ValidTabs.Num() > 0)
	{
		TabManagerRef->CloseTabs(ValidTabs);
		OnMenuItemClicked.Broadcast(TEXT("Close"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] No valid tabs to close"));
	}

	CloseMenu();
}

void UTabContextMenu::HandleBrowseClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Browse clicked"));

	if (!TabManagerRef.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] TabManager is invalid"));
		return;
	}

	TArray<FEditorTabInfo> ValidTabs = GetValidTabs();

	if (ValidTabs.Num() > 0)
	{
		TabManagerRef->BrowseToAssetByInfo(ValidTabs[0]);
		OnMenuItemClicked.Broadcast(TEXT("Browse"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] No valid tab to browse"));
	}

	CloseMenu();
}

void UTabContextMenu::HandleGroupClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Group clicked"));

	FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

	if (GroupSubMenuClass && TabManagerRef.IsValid())
	{
		// 关闭现有子菜单
		if (ActiveGroupSubMenu)
		{
			UTabPopupManager::ClosePopup(ActiveGroupSubMenu);
			ActiveGroupSubMenu = nullptr;
		}

		TArray<FEditorTabInfo> ValidTabs = GetValidTabs();

		if (ValidTabs.Num() > 0)
		{
			// 🔧 使用 TabPopupManager 显示子菜单
			// 位置设置为主菜单右侧
			FVector2D SubMenuPosition = MousePosition;
			SubMenuPosition.X += 300.0f; // 向右偏移300像素

			ActiveGroupSubMenu = Cast<UTabGroupSubMenu>(
				UTabPopupManager::ShowPopup(
					GroupSubMenuClass,
					SubMenuPosition,
					FVector2D::ZeroVector, // 自动大小
					true // 点击外部关闭
				)
			);

			if (ActiveGroupSubMenu)
			{
				// 初始化子菜单
				ActiveGroupSubMenu->InitializeSubMenu(TabManagerRef.Get(), ValidTabs);

				OnMenuItemClicked.Broadcast(TEXT("Group"));

				UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Group submenu shown"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] No valid tabs for group submenu"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[TabContextMenu] GroupSubMenuClass not set or TabManager invalid"));
	}
}

// ============ 辅助方法 ============

TArray<FEditorTabInfo> UTabContextMenu::GetValidTabs() const
{
	if (!TabManagerRef.IsValid())
	{
		return TArray<FEditorTabInfo>();
	}

	// 获取当前所有打开的 Tab
	TArray<FEditorTabInfo> AllCurrentTabs = TabManagerRef->GetAllTabs();

	// 创建一个 TabId 的集合用于快速查找
	TSet<FString> CurrentTabIds;
	for (const FEditorTabInfo& Tab : AllCurrentTabs)
	{
		CurrentTabIds.Add(Tab.TabId);
	}

	// 过滤出仍然存在的 Tab
	TArray<FEditorTabInfo> ValidTabs;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		if (CurrentTabIds.Contains(Tab.TabId))
		{
			ValidTabs.Add(Tab);
		}
	}

	return ValidTabs;
}

bool UTabContextMenu::HasDirtyTabs() const
{
	TArray<FEditorTabInfo> ValidTabs = GetValidTabs();

	for (const FEditorTabInfo& Tab : ValidTabs)
	{
		if (Tab.bIsDirty)
		{
			return true;
		}
	}

	return false;
}
