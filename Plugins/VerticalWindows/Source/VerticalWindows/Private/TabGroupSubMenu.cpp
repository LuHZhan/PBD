#include "TabGroupSubMenu.h"
#include "EUW_Windows.h"
#include "TabCommands.h"
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

void UTabGroupSubMenu::InitializeSubMenu(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs)
{
	WindowsRef = Windows;
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

	if (!WindowsRef.IsValid()) return;

	// 获取已存在的群组
	TArray<FTabGroupInfo> ExistingGroups = WindowsRef->GetGroupedTabs();

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
	TArray<FCustomTabGroup> CustomGroups = WindowsRef->GetCustomGroups();
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
	if (!WindowsRef.IsValid()) return;

	// 添加标签到群组
	UTabAddToGroupCommand* Command = UTabAddToGroupCommand::Create(
		WindowsRef.Get(), TargetTabs, GroupId);

	if (Command)
	{
		Command->Execute();
	}

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
	if (!WindowsRef.IsValid() || GroupName.IsEmpty()) return;

	// 创建群组
	WindowsRef->CreateCustomGroup(GroupName);

	// 设置群组颜色（更新CustomGroups）
	for (FCustomTabGroup& Group : WindowsRef->CustomGroups)
	{
		if (Group.GroupId == GroupName)
		{
			Group.Color = GroupColor;
			break;
		}
	}

	// 添加标签到群组
	UTabAddToGroupCommand* Command = UTabAddToGroupCommand::Create(
		WindowsRef.Get(), TargetTabs, GroupName);

	if (Command)
	{
		Command->Execute();
	}

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
