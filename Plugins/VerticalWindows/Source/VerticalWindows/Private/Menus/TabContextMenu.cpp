#include "TabContextMenu.h"
#include "TabManager.h"
#include "TabGroupSubMenu.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTabContextMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 🔧 初始状态设置为隐藏，避免在错误位置显示
	SetVisibility(ESlateVisibility::Collapsed);
	
	// 绑定所有按钮事件
	BindButtons();
}

void UTabContextMenu::NativeDestruct()
{
	// 关闭子菜单
	if (ActiveGroupSubMenu)
	{
		ActiveGroupSubMenu->CloseSubMenu();
		ActiveGroupSubMenu->RemoveFromParent();
		ActiveGroupSubMenu = nullptr;
	}
	
	Super::NativeDestruct();
}

void UTabContextMenu::BindButtons()
{
	// 绑定必需按钮
	if (OpenButton)
	{
		OpenButton->OnClicked.AddDynamic(this, &UTabContextMenu::HandleOpenClicked);
	}
	
	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &UTabContextMenu::HandleSaveClicked);
	}
	
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UTabContextMenu::HandleCloseClicked);
	}
	
	if (GroupButton)
	{
		GroupButton->OnClicked.AddDynamic(this, &UTabContextMenu::HandleGroupClicked);
	}
	
	// 绑定可选按钮
	if (BrowseButton)
	{
		BrowseButton->OnClicked.AddDynamic(this, &UTabContextMenu::HandleBrowseClicked);
	}
}

void UTabContextMenu::InitializeMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs)
{
	TabManagerRef = Manager;
	TargetTabs = Tabs;
	bIsMultiSelection = Tabs.Num() > 1;

	// 更新按钮状态
	// UpdateButtonStates();
	
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Initialized with %d tabs, IsMultiSelection=%d"), 
		Tabs.Num(), bIsMultiSelection);
}

void UTabContextMenu::UpdateButtonStates()
{
	TArray<FEditorTabInfo> ValidTabs = GetValidTabs();
	bool bHasValidTabs = ValidTabs.Num() > 0;
	bool bHasDirty = HasDirtyTabs();
	
	// Save 按钮：只在有脏标签时启用
	if (SaveButton)
	{
		SaveButton->SetIsEnabled(bHasValidTabs && bHasDirty);
	}
	
	// Browse 按钮：只在单选时显示
	if (BrowseButton)
	{
		if (bIsMultiSelection)
		{
			BrowseButton->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			BrowseButton->SetVisibility(ESlateVisibility::Visible);
			BrowseButton->SetIsEnabled(bHasValidTabs);
		}
	}
	
	// Open, Close, Group 按钮：有有效标签时启用
	if (OpenButton)
	{
		OpenButton->SetIsEnabled(bHasValidTabs);
	}
	
	if (CloseButton)
	{
		CloseButton->SetIsEnabled(bHasValidTabs);
	}
	
	if (GroupButton)
	{
		GroupButton->SetIsEnabled(bHasValidTabs);
	}
	
	// 更新标题文本
	if (TitleText)
	{
		FString TitleString;
		if (bIsMultiSelection)
		{
			TitleString = FString::Printf(TEXT("Selected: %d tabs"), ValidTabs.Num());
		}
		else if (ValidTabs.Num() > 0)
		{
			TitleString = ValidTabs[0].DisplayName;
		}
		else
		{
			TitleString = TEXT("No valid tabs");
		}
		
		TitleText->SetText(FText::FromString(TitleString));
	}
	
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Updated states: ValidTabs=%d, HasDirty=%d, IsMulti=%d"),
		ValidTabs.Num(), bHasDirty, bIsMultiSelection);
}

void UTabContextMenu::ShowAtPosition(FVector2D ScreenPosition)
{
	// 在固定位置显示（屏幕空间）
	SetPositionInViewport(ScreenPosition, false);
	SetVisibility(ESlateVisibility::Visible);
	
	UE_LOG(LogTemp, Log, TEXT("[TabContextMenu] Shown at position (%.1f, %.1f)"), 
		ScreenPosition.X, ScreenPosition.Y);
}

void UTabContextMenu::CloseMenu()
{
	// 关闭子菜单
	if (ActiveGroupSubMenu)
	{
		ActiveGroupSubMenu->CloseSubMenu();
		ActiveGroupSubMenu->RemoveFromParent();
		ActiveGroupSubMenu = nullptr;
	}
	
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
	
	// 如果有 GroupSubMenuClass，创建子菜单
	if (GroupSubMenuClass && TabManagerRef.IsValid())
	{
		// 关闭现有子菜单
		if (ActiveGroupSubMenu)
		{
			ActiveGroupSubMenu->CloseSubMenu();
			ActiveGroupSubMenu->RemoveFromParent();
		}
		
		TArray<FEditorTabInfo> ValidTabs = GetValidTabs();
		
		if (ValidTabs.Num() > 0)
		{
			ActiveGroupSubMenu = CreateWidget<UTabGroupSubMenu>(this, GroupSubMenuClass);
			if (ActiveGroupSubMenu)
			{
				ActiveGroupSubMenu->InitializeSubMenu(TabManagerRef.Get(), ValidTabs);
				ActiveGroupSubMenu->AddToViewport(101);
				ActiveGroupSubMenu->ShowAtPosition(MousePosition);
				
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
