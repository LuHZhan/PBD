#include "TabItemWidget.h"
#include "TabSelectionManager.h"
#include "TabDragDropOperation.h"
#include "TabInputHandler.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Framework/Application/SlateApplication.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UTabItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindButtonEvents();

	// Register with selection manager if available
	if (SelectionManager.IsValid())
	{
		SelectionManager->RegisterItemWidget(this);
	}
}

void UTabItemWidget::NativeDestruct()
{
	// Unregister from selection manager
	if (SelectionManager.IsValid())
	{
		SelectionManager->UnregisterItemWidget(this);
	}

	Super::NativeDestruct();
}

void UTabItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Check for long press to start drag
	if (bMouseDownForDrag && !bIsDragging)
	{
		MouseDownTime += InDeltaTime;

		if (MouseDownTime >= DragHoldTime)
		{
			// Check if mouse moved too much (cancel if so)
			FVector2D CurrentPos;
			if (FSlateApplication::IsInitialized())
			{
				CurrentPos = FSlateApplication::Get().GetCursorPos();
			}
			float Distance = FVector2D::Distance(MouseDownPosition, CurrentPos);

			if (Distance < 10.0f)
			{
				StartDragOperation();
			}
			else
			{
				bMouseDownForDrag = false;
			}
		}
	}
}

FReply UTabItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// Start tracking for potential drag
		bMouseDownForDrag = true;
		MouseDownTime = 0.0f;
		MouseDownPosition = InMouseEvent.GetScreenSpacePosition();

		return FReply::Handled();
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// Right click - show context menu
		HandleRightClicked(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UTabItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bMouseDownForDrag && !bIsDragging)
		{
			// This was a click, not a drag
			bMouseDownForDrag = false;

			// Handle selection with modifier keys
			if (SelectionManager.IsValid())
			{
				bool bShift = UTabInputFunctionLibrary::IsShiftKeyDown();
				bool bCtrl = UTabInputFunctionLibrary::IsCtrlKeyDown();

				SelectionManager->HandleItemClick(TabData, bShift, bCtrl);
			}

			// Fire click event
			HandleItemClicked();

			return FReply::Handled();
		}

		bMouseDownForDrag = false;
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UTabItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// This is called by UE when drag is detected via DetectDrag
	// We handle our own drag logic in NativeTick, so this may not be used
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

void UTabItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	bIsDragging = false;
	bMouseDownForDrag = false;
	SetItemState(ETabItemState::Normal);
	OnDragEnded.Broadcast(TabData, false);
	OnDragEndedEvent(false);

	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
}

void UTabItemWidget::BindButtonEvents()
{
	if (RootButton)
	{
		RootButton->OnHovered.AddDynamic(this, &UTabItemWidget::HandleHovered);
		RootButton->OnUnhovered.AddDynamic(this, &UTabItemWidget::HandleUnhovered);
		
		// Bind OnClicked directly as UButton consumes mouse events preventing NativeOnMouseButtonUp from firing
		RootButton->OnClicked.AddDynamic(this, &UTabItemWidget::HandleItemClicked);
	}
}

void UTabItemWidget::SetTabData(const FEditorTabInfo& InData)
{
	TabData = InData;
	OnDataUpdated();
}

void UTabItemWidget::SetIsSelected(bool bSelected)
{
	if (bSelected)
	{
		SetItemState(ETabItemState::Selected);
	}
	else if (CurrentState == ETabItemState::Selected)
	{
		SetItemState(ETabItemState::Normal);
	}

	OnSelectionStateChanged(bSelected);
}

void UTabItemWidget::SetItemState(ETabItemState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	OnItemStateChanged(NewState);

	// Update visuals based on state
	if (BGWidgetSwitcher)
	{
		switch (NewState)
		{
		case ETabItemState::Normal:
			BGWidgetSwitcher->SetActiveWidgetIndex(0);
			break;
		case ETabItemState::Hovered:
		case ETabItemState::Selected:
		case ETabItemState::Dragging:
			BGWidgetSwitcher->SetActiveWidgetIndex(1);
			break;
		}
	}
}

int32 UTabItemWidget::GetItemIndex() const
{
	// This would need to be set externally or queried from parent
	return TabData.DisplayOrder;
}

void UTabItemWidget::SetSelectionManager(UTabSelectionManager* Manager)
{
	// Unregister from old manager
	if (SelectionManager.IsValid())
	{
		SelectionManager->UnregisterItemWidget(this);
	}

	SelectionManager = Manager;

	// Register with new manager
	if (SelectionManager.IsValid())
	{
		SelectionManager->RegisterItemWidget(this);
	}
}

void UTabItemWidget::CancelDrag()
{
	bMouseDownForDrag = false;
	bIsDragging = false;
	MouseDownTime = 0.0f;
}

void UTabItemWidget::StartDragOperation()
{
	bIsDragging = true;
	bMouseDownForDrag = false;
	SetItemState(ETabItemState::Dragging);

	OnDragStarted.Broadcast(TabData);
	OnDragStartedEvent();

	// Create drag operation
	UTabDragDropOperation* DragOp = UTabDragDropOperation::CreateTabDragOperation(
		this, this, TabData, GetItemIndex());

	if (DragOp)
	{
		// Start the UMG drag
		UWidgetBlueprintLibrary::CreateDragDropOperation(UTabDragDropOperation::StaticClass());
	}
}

void UTabItemWidget::OnDataUpdated_Implementation()
{
	// Update title text
	if (ItemText)
	{
		ItemText->SetText(FText::FromString(TabData.DisplayName));
	}

	// Update background color based on group color
	if (Background)
	{
		Background->SetColorAndOpacity(TabData.GroupColor);
	}

	if (IconImage)
	{
		IconImage->SetBrush(TabData.IconBrush);
		IconImage->SetColorAndOpacity(TabData.GroupColor);
	}
}

void UTabItemWidget::OnSelectionStateChanged_Implementation(bool bSelected)
{
	// Switch between selected/unselected background
	if (BGWidgetSwitcher)
	{
		BGWidgetSwitcher->SetActiveWidgetIndex(bSelected ? 1 : 0);
	}
}

void UTabItemWidget::HandleItemClicked()
{
	// Handle selection with modifier keys
	
	if (SelectionManager.IsValid())
	{
		bool bShift = UTabInputFunctionLibrary::IsShiftKeyDown();
		bool bCtrl = UTabInputFunctionLibrary::IsCtrlKeyDown();

		SelectionManager->HandleItemClick(TabData, bShift, bCtrl);
	}

	OnClicked.Broadcast(TabData);
}

void UTabItemWidget::HandleCloseClicked()
{
	OnClosed.Broadcast(TabData);
}

void UTabItemWidget::HandleHovered()
{
	if (CurrentState != ETabItemState::Selected && CurrentState != ETabItemState::Dragging)
	{
		SetItemState(ETabItemState::Hovered);
	}
	OnHovered.Broadcast(TabData);
}

void UTabItemWidget::HandleUnhovered()
{
	if (CurrentState == ETabItemState::Hovered)
	{
		SetItemState(ETabItemState::Normal);
	}
	OnUnhovered.Broadcast(TabData);
}

void UTabItemWidget::HandleRightClicked(FVector2D ScreenPosition)
{
	// If not already selected, select this item first
	if (SelectionManager.IsValid() && !SelectionManager->IsSelected(TabData))
	{
		bool bShift = UTabInputFunctionLibrary::IsShiftKeyDown();
		bool bCtrl = UTabInputFunctionLibrary::IsCtrlKeyDown();

		if (!bShift && !bCtrl)
		{
			// Single selection on right click if no modifiers
			SelectionManager->SelectSingle(TabData);
		}
	}

	OnRightClicked.Broadcast(TabData, ScreenPosition);
}
