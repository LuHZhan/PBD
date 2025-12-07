#include "TabGroupSubMenu.h"
#include "TabManager.h"
#include "TabGroupItem.h"
#include "TabCreateGroupDialog.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

void UTabGroupSubMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// 绑定创建群组按钮
	if (CreateGroupButton)
	{
		CreateGroupButton->OnClicked.AddDynamic(this, &UTabGroupSubMenu::HandleCreateGroupClicked);
	}
}

void UTabGroupSubMenu::InitializeSubMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs)
{
	TabManagerRef = Manager;
	TargetTabs = Tabs;

	BuildGroupList();
	RebuildGroupItems();
	OnSubMenuInitialized();
	OnPopulateGroupItems(AvailableGroups);
}

void UTabGroupSubMenu::ShowAtPosition(FVector2D ScreenPosition)
{
	SetPositionInViewport(ScreenPosition, false);
	SetVisibility(ESlateVisibility::Visible);
}

void UTabGroupSubMenu::CloseSubMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
	
	// 关闭对话框
	if (ActiveDialog)
	{
		ActiveDialog->CloseDialog();
		ActiveDialog->RemoveFromParent();
		ActiveDialog = nullptr;
	}
	
	OnSubMenuClosed.Broadcast();
}

void UTabGroupSubMenu::BuildGroupList()
{
	AvailableGroups.Empty();

	if (!TabManagerRef.IsValid()) return;

	// 获取已存在的群组
	TArray<FTabGroupInfo> ExistingGroups = TabManagerRef->GetGroupedTabs();

	for (const FTabGroupInfo& Group : ExistingGroups)
	{
		FGroupMenuItemData Item;
		Item.GroupId = Group.GroupId;
		Item.GroupName = Group.GroupName;
		Item.GroupColor = Group.Color;
		Item.bIsNewGroup = false;
		AvailableGroups.Add(Item);
	}

	// 添加自定义群组（如果有）
	TArray<FCustomTabGroup> CustomGroups = TabManagerRef->GetCustomGroups();
	for (const FCustomTabGroup& CustomGroup : CustomGroups)
	{
		// 检查是否已经存在
		bool bExists = false;
		for (const FGroupMenuItemData& Item : AvailableGroups)
		{
			if (Item.GroupId == CustomGroup.GroupId)
			{
				bExists = true;
				break;
			}
		}

		if (!bExists)
		{
			FGroupMenuItemData Item;
			Item.GroupId = CustomGroup.GroupId;
			Item.GroupName = CustomGroup.GroupName;
			Item.GroupColor = CustomGroup.Color;
			Item.bIsNewGroup = false;
			AvailableGroups.Add(Item);
		}
	}
}

void UTabGroupSubMenu::RebuildGroupItems()
{
	if (!GroupItemContainer) return;

	// 清空现有控件
	GroupItemContainer->ClearChildren();
	GroupItemWidgets.Empty();

	if (!GroupItemClass) return;

	// 创建群组项
	for (const FGroupMenuItemData& GroupData : AvailableGroups)
	{
		UTabGroupItem* GroupItem = CreateWidget<UTabGroupItem>(this, GroupItemClass);
		if (GroupItem)
		{
			GroupItem->SetGroupData(GroupData.GroupId, GroupData.GroupName, GroupData.GroupColor);
			GroupItem->OnClicked.AddDynamic(this, &UTabGroupSubMenu::HandleGroupItemClicked);
			
			GroupItemContainer->AddChildToVerticalBox(GroupItem);
			GroupItemWidgets.Add(GroupItem);
		}
	}
}

TArray<FGroupMenuItemData> UTabGroupSubMenu::GetAvailableGroups() const
{
	return AvailableGroups;
}

void UTabGroupSubMenu::SelectGroup(const FString& GroupId)
{
	if (!TabManagerRef.IsValid()) return;

	// 通过 TabManager 添加到群组
	TabManagerRef->AddTabsToGroup(TargetTabs, GroupId);

	OnGroupSelected.Broadcast(GroupId);
	CloseSubMenu();
}

void UTabGroupSubMenu::ShowCreateGroupDialog()
{
	if (!CreateGroupDialogClass) return;

	// 关闭现有对话框
	if (ActiveDialog)
	{
		ActiveDialog->RemoveFromParent();
	}

	// 创建新对话框
	ActiveDialog = CreateWidget<UTabCreateGroupDialog>(this, CreateGroupDialogClass);
	if (ActiveDialog)
	{
		ActiveDialog->OnGroupCreated.AddDynamic(this, &UTabGroupSubMenu::HandleGroupCreated);
		ActiveDialog->OnCancelled.AddDynamic(this, &UTabGroupSubMenu::HandleDialogCancelled);
		ActiveDialog->AddToViewport(101); // 比菜单层级更高
		ActiveDialog->ShowDialog();
	}
}

void UTabGroupSubMenu::CreateNewGroup(const FString& GroupName, FLinearColor GroupColor)
{
	if (!TabManagerRef.IsValid() || GroupName.IsEmpty()) return;

	// 通过 TabManager 创建群组
	TabManagerRef->CreateCustomGroup(GroupName, GroupColor);

	// 添加标签到群组
	TabManagerRef->AddTabsToGroup(TargetTabs, GroupName);

	OnNewGroupCreated.Broadcast(GroupName, GroupColor);
	CloseSubMenu();
}

void UTabGroupSubMenu::HandleGroupItemClicked(const FString& GroupId)
{
	SelectGroup(GroupId);
}

void UTabGroupSubMenu::HandleCreateGroupClicked()
{
	ShowCreateGroupDialog();
}

void UTabGroupSubMenu::HandleGroupCreated(const FString& GroupName, FLinearColor GroupColor)
{
	CreateNewGroup(GroupName, GroupColor);
}

void UTabGroupSubMenu::HandleDialogCancelled()
{
	// 对话框取消，保持子菜单打开
	if (ActiveDialog)
	{
		ActiveDialog->RemoveFromParent();
		ActiveDialog = nullptr;
	}
}
