#include "TabSelectionManager.h"
#include "TabItemWidget.h"

UTabSelectionManager::UTabSelectionManager()
{
}

void UTabSelectionManager::HandleItemClick(const FEditorTabInfo& TabInfo, bool bShiftDown, bool bCtrlDown)
{
	if (bShiftDown && !SelectionAnchorId.IsEmpty())
	{
		// Range selection
		SelectRange(TabInfo);
	}
	else if (bCtrlDown)
	{
		// Toggle selection
		ToggleSelection(TabInfo);
	}
	else
	{
		// Single selection
		SelectSingle(TabInfo);
	}
}

void UTabSelectionManager::SelectSingle(const FEditorTabInfo& TabInfo)
{
	SelectedTabIds.Empty();
	SelectedTabIds.Add(TabInfo.TabId);
	SelectionAnchorId = TabInfo.TabId;

	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabSelectionManager::SelectRange(const FEditorTabInfo& TabInfo)
{
	int32 AnchorIndex = FindTabIndex(SelectionAnchorId);
	int32 TargetIndex = FindTabIndex(TabInfo.TabId);

	if (AnchorIndex < 0 || TargetIndex < 0) return;

	// Clear and select range
	SelectedTabIds.Empty();

	int32 StartIndex = FMath::Min(AnchorIndex, TargetIndex);
	int32 EndIndex = FMath::Max(AnchorIndex, TargetIndex);

	for (int32 i = StartIndex; i <= EndIndex; ++i)
	{
		if (AllTabs.IsValidIndex(i))
		{
			SelectedTabIds.Add(AllTabs[i].TabId);
		}
	}

	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabSelectionManager::ToggleSelection(const FEditorTabInfo& TabInfo)
{
	if (SelectedTabIds.Contains(TabInfo.TabId))
	{
		SelectedTabIds.Remove(TabInfo.TabId);
	}
	else
	{
		SelectedTabIds.Add(TabInfo.TabId);
		// Update anchor to last clicked
		SelectionAnchorId = TabInfo.TabId;
	}

	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabSelectionManager::AddToSelection(const FEditorTabInfo& TabInfo)
{
	SelectedTabIds.Add(TabInfo.TabId);
	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabSelectionManager::RemoveFromSelection(const FEditorTabInfo& TabInfo)
{
	SelectedTabIds.Remove(TabInfo.TabId);
	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabSelectionManager::ClearSelection()
{
	SelectedTabIds.Empty();
	SelectionAnchorId.Empty();
	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

void UTabSelectionManager::SelectAll()
{
	SelectedTabIds.Empty();
	for (const FEditorTabInfo& Tab : AllTabs)
	{
		SelectedTabIds.Add(Tab.TabId);
	}
	NotifySelectionChanged();
	UpdateAllWidgetVisuals();
}

bool UTabSelectionManager::IsSelected(const FEditorTabInfo& TabInfo) const
{
	return SelectedTabIds.Contains(TabInfo.TabId);
}

TArray<FEditorTabInfo> UTabSelectionManager::GetSelectedTabs() const
{
	TArray<FEditorTabInfo> Result;
	for (const FEditorTabInfo& Tab : AllTabs)
	{
		if (SelectedTabIds.Contains(Tab.TabId))
		{
			Result.Add(Tab);
		}
	}
	return Result;
}

int32 UTabSelectionManager::GetSelectionCount() const
{
	return SelectedTabIds.Num();
}

bool UTabSelectionManager::HasSelection() const
{
	return SelectedTabIds.Num() > 0;
}

bool UTabSelectionManager::IsMultiSelection() const
{
	return SelectedTabIds.Num() > 1;
}

void UTabSelectionManager::SetTabList(const TArray<FEditorTabInfo>& InTabs)
{
	AllTabs = InTabs;
}

void UTabSelectionManager::UpdateTabList(const TArray<FEditorTabInfo>& InTabs)
{
	AllTabs = InTabs;

	// Remove selections for tabs that no longer exist
	TSet<FString> ValidIds;
	for (const FEditorTabInfo& Tab : InTabs)
	{
		ValidIds.Add(Tab.TabId);
	}

	TSet<FString> ToRemove;
	for (const FString& Id : SelectedTabIds)
	{
		if (!ValidIds.Contains(Id))
		{
			ToRemove.Add(Id);
		}
	}

	for (const FString& Id : ToRemove)
	{
		SelectedTabIds.Remove(Id);
	}

	if (ToRemove.Num() > 0)
	{
		NotifySelectionChanged();
	}
}

void UTabSelectionManager::RegisterItemWidget(UTabItemWidget* Widget)
{
	if (Widget)
	{
		RegisteredWidgets.AddUnique(Widget);
	}
}

void UTabSelectionManager::UnregisterItemWidget(UTabItemWidget* Widget)
{
	RegisteredWidgets.Remove(Widget);
}

void UTabSelectionManager::UpdateAllWidgetVisuals()
{
	for (TWeakObjectPtr<UTabItemWidget>& WidgetPtr : RegisteredWidgets)
	{
		if (UTabItemWidget* Widget = WidgetPtr.Get())
		{
			bool bSelected = SelectedTabIds.Contains(Widget->TabData.TabId);
			Widget->SetIsSelected(bSelected);
		}
	}
}

int32 UTabSelectionManager::FindTabIndex(const FString& TabId) const
{
	for (int32 i = 0; i < AllTabs.Num(); ++i)
	{
		if (AllTabs[i].TabId == TabId)
		{
			return i;
		}
	}
	return -1;
}

void UTabSelectionManager::NotifySelectionChanged()
{
	OnSelectionChanged.Broadcast(GetSelectedTabs());
}
