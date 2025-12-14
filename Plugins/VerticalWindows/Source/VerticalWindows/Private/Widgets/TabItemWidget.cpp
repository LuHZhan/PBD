#include "TabItemWidget.h"
#include "TabManager.h"
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
	
	// 只绑定 CloseButton
	BindCloseButton();
	
	// 注册到 Manager
	if (TabManager.IsValid())
	{
		TabManager->RegisterItemWidget(this);
	}
}

void UTabItemWidget::NativeDestruct()
{
	// 从 Manager 注销
	if (TabManager.IsValid())
	{
		TabManager->UnregisterItemWidget(this);
	}

	Super::NativeDestruct();
}

void UTabItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 检查长按触发拖拽
	if (bMouseDownForDrag && !bIsDragging)
	{
		MouseDownTime += InDeltaTime;

		if (MouseDownTime >= DragHoldTime)
		{
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
				// 鼠标移动太远，取消拖拽准备
				bMouseDownForDrag = false;
			}
		}
	}
}

// ============ 鼠标事件处理 (方案B核心) ============

FReply UTabItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 注意：如果点击的是 CloseButton，这个函数不会被调用
	// 因为 CloseButton 会先接收事件并返回 Handled
	
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 左键：开始跟踪潜在拖拽
		bMouseDownForDrag = true;
		MouseDownTime = 0.0f;
		MouseDownPosition = InMouseEvent.GetScreenSpacePosition();
		return FReply::Handled();
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 右键：记录位置，等待 MouseUp 时统一处理
		// 不在这里处理，保持和左键一致的时机
		MouseDownPosition = InMouseEvent.GetScreenSpacePosition();
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
			// 左键点击（非拖拽）
			bMouseDownForDrag = false;
			
			// 执行标签激活和选择逻辑
			HandleItemClicked();
			return FReply::Handled();
		}
		bMouseDownForDrag = false;
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		// 右键点击：先执行和左键相同的逻辑，然后显示菜单
		
		// 1. 先激活标签和处理选择（和左键相同）
		HandleItemClicked();
		
		// 2. 然后显示右键菜单（区别于左键的额外操作）
		HandleRightClicked(InMouseEvent.GetScreenSpacePosition());
		
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UTabItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	HandleHovered();
}

void UTabItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	HandleUnhovered();
}

void UTabItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
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

// ============ 内部绑定 ============

void UTabItemWidget::BindCloseButton()
{
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UTabItemWidget::HandleCloseClicked);
	}
}

// ============ 设置方法 ============

void UTabItemWidget::SetTabManager(UTabManager* Manager)
{
	// 从旧 Manager 注销
	if (TabManager.IsValid())
	{
		TabManager->UnregisterItemWidget(this);
	}

	TabManager = Manager;

	// 注册到新 Manager
	if (TabManager.IsValid())
	{
		TabManager->RegisterItemWidget(this);
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

	// 更新背景
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
	return TabData.DisplayOrder;
}

void UTabItemWidget::CancelDrag()
{
	bMouseDownForDrag = false;
	bIsDragging = false;
	MouseDownTime = 0.0f;
}

// ============ 核心事件处理（直接调用 TabManager） ============

void UTabItemWidget::HandleItemClicked()
{
	if (!TabManager.IsValid()) return;

	// 1. 先处理选择（带修饰键）
	bool bShift = UTabInputFunctionLibrary::IsShiftKeyDown();
	bool bCtrl = UTabInputFunctionLibrary::IsCtrlKeyDown();
	TabManager->HandleItemClick(TabData, bShift, bCtrl);

	// 2. 打开标签
	TabManager->OpenTab(TabData);
}

void UTabItemWidget::HandleCloseClicked()
{
	if (!TabManager.IsValid()) return;

	// 如果是多选且当前标签被选中，关闭所有选中的
	if (TabManager->IsMultiSelection() && TabManager->IsSelected(TabData))
	{
		TabManager->CloseSelectedTabs();
	}
	else
	{
		TabManager->CloseTabByInfo(TabData);
	}
}

void UTabItemWidget::HandleRightClicked(FVector2D ScreenPosition)
{
	if (!TabManager.IsValid()) return;

	// 如果未选中，先单选
	if (!TabManager->IsSelected(TabData))
	{
		bool bShift = UTabInputFunctionLibrary::IsShiftKeyDown();
		bool bCtrl = UTabInputFunctionLibrary::IsCtrlKeyDown();

		if (!bShift && !bCtrl)
		{
			TabManager->SelectSingle(TabData);
		}
	}

	// 广播事件给 UI 层处理菜单显示
	OnRightClicked.Broadcast(TabData, ScreenPosition);
}

void UTabItemWidget::HandleHovered()
{
	if (CurrentState != ETabItemState::Selected && CurrentState != ETabItemState::Dragging)
	{
		SetItemState(ETabItemState::Hovered);
	}
}

void UTabItemWidget::HandleUnhovered()
{
	if (CurrentState == ETabItemState::Hovered)
	{
		SetItemState(ETabItemState::Normal);
	}
}

void UTabItemWidget::StartDragOperation()
{
	bIsDragging = true;
	bMouseDownForDrag = false;
	SetItemState(ETabItemState::Dragging);

	OnDragStarted.Broadcast(TabData);
	OnDragStartedEvent();

	// 创建拖拽操作
	UTabDragDropOperation* DragOp = UTabDragDropOperation::CreateTabDragOperation(
		this, this, TabData, GetItemIndex());

	if (DragOp)
	{
		UWidgetBlueprintLibrary::CreateDragDropOperation(UTabDragDropOperation::StaticClass());
	}
}

// ============ 蓝图事件默认实现 ============

void UTabItemWidget::OnDataUpdated_Implementation()
{
	if (ItemText)
	{
		ItemText->SetText(FText::FromString(TabData.DisplayName));
	}

	if (Background)
	{
		Background->SetColorAndOpacity(TabData.GroupColor);
	}

	if (IconImage)
	{
		IconImage->SetBrush(TabData.IconBrush);
		// 🔧 移除图标染色，让图标保持原色
		// IconImage->SetColorAndOpacity(TabData.GroupColor);
		IconImage->SetColorAndOpacity(FLinearColor::White);  // 使用白色（原色）
	}

	if (DirtyIndicator)
	{
		DirtyIndicator->SetVisibility(TabData.bIsDirty ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UTabItemWidget::OnSelectionStateChanged_Implementation(bool bSelected)
{
	if (BGWidgetSwitcher)
	{
		BGWidgetSwitcher->SetActiveWidgetIndex(bSelected ? 1 : 0);
	}
}