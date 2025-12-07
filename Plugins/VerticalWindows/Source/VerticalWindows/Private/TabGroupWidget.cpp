#include "TabGroupWidget.h"
#include "TabItemWidget.h"
#include "TabSelectionManager.h"

void UTabGroupWidget::SetGroupData(const FTabGroupInfo& InData)
{
	GroupData = InData;
	OnGroupDataUpdated();

	// Initial state
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

		// If it's a TabItemWidget, track and bind it
		if (UTabItemWidget* TabItem = Cast<UTabItemWidget>(Widget))
		{
			AddTabItemWidget(TabItem);
		}
	}
}

void UTabGroupWidget::AddTabItemWidget(UTabItemWidget* ItemWidget)
{
	if (!ItemWidget) return;

	ChildItemWidgets.Add(ItemWidget);

	// Set selection manager if available
	if (SelectionManager.IsValid())
	{
		ItemWidget->SetSelectionManager(SelectionManager.Get());
	}

	// Bind events for forwarding
	ItemWidget->OnClicked.AddDynamic(this, &UTabGroupWidget::HandleChildItemClicked);
	ItemWidget->OnClosed.AddDynamic(this, &UTabGroupWidget::HandleChildItemClosed);
	ItemWidget->OnRightClicked.AddDynamic(this, &UTabGroupWidget::HandleChildItemRightClicked);
}

void UTabGroupWidget::SetSelectionManager(UTabSelectionManager* Manager)
{
	SelectionManager = Manager;

	// Update all child items
	for (UTabItemWidget* ItemWidget : ChildItemWidgets)
	{
		if (ItemWidget)
		{
			ItemWidget->SetSelectionManager(Manager);
		}
	}
}

void UTabGroupWidget::ClearTabs()
{
	GroupData.Tabs.Empty();

	// Clear child widget tracking
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

	// Also control container visibility
	if (ItemContainer)
	{
		ItemContainer->SetVisibility(bExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UTabGroupWidget::OnGroupDataUpdated_Implementation()
{
	// Default implementation - can be overridden in Blueprint
}

void UTabGroupWidget::HandleHeaderClicked()
{
	SetExpanded(!GroupData.bExpanded);
}

// ============ NEW: Event Forwarding Handlers ============

void UTabGroupWidget::HandleChildItemClicked(const FEditorTabInfo& TabInfo)
{
	// Forward to parent with group context
	OnItemClicked.Broadcast(GroupData, TabInfo);
}

void UTabGroupWidget::HandleChildItemClosed(const FEditorTabInfo& TabInfo)
{
	// Forward to parent with group context
	OnItemClosed.Broadcast(GroupData, TabInfo);
}

void UTabGroupWidget::HandleChildItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition)
{
	// Forward to parent with group context
	OnItemRightClicked.Broadcast(GroupData, TabInfo, ScreenPosition);
}
