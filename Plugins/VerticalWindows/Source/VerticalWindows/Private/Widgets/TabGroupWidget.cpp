#include "TabGroupWidget.h"
#include "TabItemWidget.h"
#include "TabManager.h"

void UTabGroupWidget::SetTabManager(UTabManager* Manager)
{
	TabManager = Manager;

	// 更新所有子项
	for (UTabItemWidget* ItemWidget : ChildItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetTabManager(Manager);
		}
	}
}

void UTabGroupWidget::SetGroupData(const FTabGroupInfo& InData)
{
	GroupData = InData;
	OnGroupDataUpdated();
	SetExpanded(InData.bExpanded);
}

void UTabGroupWidget::AddTab(const FEditorTabInfo& TabInfo)
{
	GroupData.Tabs.Add(TabInfo);
	OnTabAdded(TabInfo);
}

void UTabGroupWidget::AddChildWidget(UUserWidget* Widget)
{
	if (ItemContainer && Widget)
	{
		ItemContainer->AddChildToVerticalBox(Widget);

		// 如果是 TabItemWidget，跟踪并设置
		if (UTabItemWidget* TabItem = Cast<UTabItemWidget>(Widget))
		{
			ChildItemWidgets.Add(TabItem);

			// 设置 Manager
			if (TabManager.IsValid())
			{
				TabItem->SetTabManager(TabManager.Get());
			}

			// 绑定右键事件（转发给 UI 层）
			TabItem->OnRightClicked.AddDynamic(this, &UTabGroupWidget::HandleChildItemRightClicked);
		}
	}
}

void UTabGroupWidget::ClearTabs()
{
	GroupData.Tabs.Empty();
	ChildItemWidgets.Empty();

	if (ItemContainer)
	{
		ItemContainer->ClearChildren();
	}
	OnTabsCleared();
}

void UTabGroupWidget::SetExpanded(bool bExpanded)
{
	GroupData.bExpanded = bExpanded;
	OnExpansionStateChanged(bExpanded);

	if (OnToggle.IsBound())
	{
		OnToggle.Broadcast(GroupData, bExpanded);
	}

	if (ItemContainer)
	{
		ItemContainer->SetVisibility(bExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UTabGroupWidget::OnGroupDataUpdated_Implementation()
{
	// 默认实现 - 蓝图可重写
}

void UTabGroupWidget::HandleHeaderClicked()
{
	SetExpanded(!GroupData.bExpanded);
}

void UTabGroupWidget::HandleChildItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition)
{
	// 转发到 UI 层
	OnItemRightClicked.Broadcast(GroupData, TabInfo, ScreenPosition);
}
