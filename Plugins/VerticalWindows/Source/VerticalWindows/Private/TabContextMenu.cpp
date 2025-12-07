#include "TabContextMenu.h"
#include "EUW_Windows.h"
#include "TabCommands.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UTabContextMenu::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTabContextMenu::InitializeMenu(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs)
{
	WindowsRef = Windows;
	TargetTabs = Tabs;
	bIsMultiSelection = Tabs.Num() > 1;

	BuildMenuItems();
	OnMenuInitialized();
	OnPopulateMenuItems(MenuItems);
}

void UTabContextMenu::ShowAtPosition(FVector2D ScreenPosition)
{
	// Set position - implement in Blueprint for proper viewport handling
	SetPositionInViewport(ScreenPosition, false);
	SetVisibility(ESlateVisibility::Visible);
}

void UTabContextMenu::CloseMenu()
{
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

	// Browse to asset (only for single selection)
	if (!bIsMultiSelection)
	{
		FTabMenuItemData BrowseItem;
		BrowseItem.ItemId = TEXT("BrowseToAsset");
		BrowseItem.DisplayText = FText::FromString(TEXT("Browse to Asset"));
		BrowseItem.bEnabled = true;
		MenuItems.Add(BrowseItem);
	}

	// Add to group (has submenu)
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
	if (!WindowsRef.IsValid()) return;

	UTabOpenCommand* Command = UTabOpenCommand::Create(WindowsRef.Get(), TargetTabs);
	if (Command)
	{
		Command->Execute();
	}

	CloseMenu();
}

void UTabContextMenu::MenuAction_Close()
{
	if (!WindowsRef.IsValid()) return;

	UTabCloseCommand* Command = UTabCloseCommand::Create(WindowsRef.Get(), TargetTabs);
	if (Command)
	{
		Command->Execute();
	}

	CloseMenu();
}

void UTabContextMenu::MenuAction_Save()
{
	if (!WindowsRef.IsValid()) return;

	UTabSaveCommand* Command = UTabSaveCommand::Create(WindowsRef.Get(), TargetTabs);
	if (Command)
	{
		Command->Execute();
	}

	CloseMenu();
}

void UTabContextMenu::MenuAction_BrowseToAsset()
{
	if (!WindowsRef.IsValid() || TargetTabs.Num() == 0) return;

	UTabBrowseCommand* Command = UTabBrowseCommand::Create(WindowsRef.Get(), TargetTabs[0]);
	if (Command)
	{
		Command->Execute();
	}

	CloseMenu();
}

void UTabContextMenu::MenuAction_ShowGroupSubMenu()
{
	// Get cursor position for submenu
	FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());

	// Notify Blueprint to show submenu
	OnShowSubMenu(TEXT("GroupSubMenu"), MousePosition);
}
