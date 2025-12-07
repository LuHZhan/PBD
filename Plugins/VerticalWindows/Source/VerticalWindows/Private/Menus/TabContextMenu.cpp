#include "TabContextMenu.h"
#include "TabManager.h"
#include "TabGroupSubMenu.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTabContextMenu::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTabContextMenu::InitializeMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs)
{
	TabManagerRef = Manager;
	TargetTabs = Tabs;
	bIsMultiSelection = Tabs.Num() > 1;

	BuildMenuItems();
	OnMenuInitialized();
	OnPopulateMenuItems(MenuItems);
}

void UTabContextMenu::ShowAtPosition(FVector2D ScreenPosition)
{
	SetPositionInViewport(ScreenPosition, false);
	SetVisibility(ESlateVisibility::Visible);
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
}

void UTabContextMenu::BuildMenuItems()
{
	MenuItems.Empty();

	// Open
	FTabMenuItemData OpenItem;
	OpenItem.ItemId = TEXT("Open");
	OpenItem.DisplayText = FText::FromString(TEXT("Open"));
	OpenItem.bEnabled = true;
	MenuItems.Add(OpenItem);

	// Close
	FTabMenuItemData CloseItem;
	CloseItem.ItemId = TEXT("Close");
	CloseItem.DisplayText = FText::FromString(TEXT("Close"));
	CloseItem.bEnabled = true;
	MenuItems.Add(CloseItem);

	// Save (only if dirty)
	bool bHasDirty = false;
	for (const FEditorTabInfo& Tab : TargetTabs)
	{
		if (Tab.bIsDirty)
		{
			bHasDirty = true;
			break;
		}
	}

	FTabMenuItemData SaveItem;
	SaveItem.ItemId = TEXT("Save");
	SaveItem.DisplayText = FText::FromString(TEXT("Save"));
	SaveItem.bEnabled = bHasDirty;
	MenuItems.Add(SaveItem);

	// Browse (only for single selection)
	if (!bIsMultiSelection)
	{
		FTabMenuItemData BrowseItem;
		BrowseItem.ItemId = TEXT("BrowseToAsset");
		BrowseItem.DisplayText = FText::FromString(TEXT("Browse to Asset"));
		BrowseItem.bEnabled = true;
		MenuItems.Add(BrowseItem);
	}

	// Add to group
	FTabMenuItemData AddToGroupItem;
	AddToGroupItem.ItemId = TEXT("AddToGroup");
	AddToGroupItem.DisplayText = FText::FromString(TEXT("Add to Group"));
	AddToGroupItem.bHasSubMenu = true;
	AddToGroupItem.bEnabled = true;
	MenuItems.Add(AddToGroupItem);
}

TArray<FTabMenuItemData> UTabContextMenu::GetMenuItems() const
{
	return MenuItems;
}

void UTabContextMenu::ExecuteMenuAction(const FString& ActionId)
{
	if (ActionId == TEXT("Open"))
	{
		MenuAction_Open();
	}
	else if (ActionId == TEXT("Close"))
	{
		MenuAction_Close();
	}
	else if (ActionId == TEXT("Save"))
	{
		MenuAction_Save();
	}
	else if (ActionId == TEXT("BrowseToAsset"))
	{
		MenuAction_BrowseToAsset();
	}
	else if (ActionId == TEXT("AddToGroup"))
	{
		MenuAction_ShowGroupSubMenu();
	}

	OnMenuItemClicked.Broadcast(ActionId);
}

void UTabContextMenu::MenuAction_Open()
{
	if (!TabManagerRef.IsValid()) return;

	TabManagerRef->OpenTabs(TargetTabs);
	CloseMenu();
}

void UTabContextMenu::MenuAction_Close()
{
	if (!TabManagerRef.IsValid()) return;

	TabManagerRef->CloseTabs(TargetTabs);
	CloseMenu();
}

void UTabContextMenu::MenuAction_Save()
{
	if (!TabManagerRef.IsValid()) return;

	TabManagerRef->SaveTabs(TargetTabs);
	CloseMenu();
}

void UTabContextMenu::MenuAction_BrowseToAsset()
{
	if (!TabManagerRef.IsValid() || TargetTabs.Num() == 0) return;

	TabManagerRef->BrowseToAssetByInfo(TargetTabs[0]);
	CloseMenu();
}

void UTabContextMenu::MenuAction_ShowGroupSubMenu()
{
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
		
		ActiveGroupSubMenu = CreateWidget<UTabGroupSubMenu>(this, GroupSubMenuClass);
		if (ActiveGroupSubMenu)
		{
			ActiveGroupSubMenu->InitializeSubMenu(TabManagerRef.Get(), TargetTabs);
			ActiveGroupSubMenu->AddToViewport(101);
			ActiveGroupSubMenu->ShowAtPosition(MousePosition);
		}
	}
	else
	{
		// 蓝图处理
		OnShowSubMenu(TEXT("GroupSubMenu"), MousePosition);
	}
}
