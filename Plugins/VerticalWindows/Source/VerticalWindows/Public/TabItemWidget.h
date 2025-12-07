#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabItemWidget.generated.h"

class UWidgetSwitcher;
class UImage;
class UTextBlock;
class UButton;
class UTabSelectionManager;
class UTabDragDropOperation;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabItemClicked, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabItemClosed, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabItemHovered, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabItemUnhovered, const FEditorTabInfo&, TabInfo);

// ============ NEW: Additional delegates ============
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabItemRightClicked, const FEditorTabInfo&, TabInfo, FVector2D, ScreenPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabItemDragStarted, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabItemDragEnded, const FEditorTabInfo&, TabInfo, bool, bDropped);

/**
 * Vertical Tab - Single Tab Item Base Class
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ Data ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Item")
	FEditorTabInfo TabData;

	// ============ NEW: Item State ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Item")
	ETabItemState CurrentState = ETabItemState::Normal;

	// ============ Component Bindings (Native) ============

	/** Widget switcher for selected/unselected background */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* BGWidgetSwitcher;

	/** Background when not selected */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* NoHoveredBG;

	/** Background when selected */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* Background;

	/** Main clickable button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* RootButton;

	/** Asset type icon */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* IconImage;

	/** Display name text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemText;

	// ============ Event Callbacks (Original) ============

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemClicked OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemClosed OnClosed;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemHovered OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemUnhovered OnUnhovered;

	// ============ NEW: Additional Event Callbacks ============

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemRightClicked OnRightClicked;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemDragStarted OnDragStarted;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemDragEnded OnDragEnded;

	// ============ NEW: Selection Manager Reference ============

	UPROPERTY(BlueprintReadWrite, Category = "Tab Item")
	TWeakObjectPtr<UTabSelectionManager> SelectionManager;

	// ============ NEW: Drag Settings ============

	/** Time to hold before drag starts (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Item|Drag")
	float DragHoldTime = 0.3f;

	/** Is drag currently in progress */
	UPROPERTY(BlueprintReadOnly, Category = "Tab Item|Drag")
	bool bIsDragging = false;

	// ============ Methods (Original) ============

	/** Set data and refresh UI */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetTabData(const FEditorTabInfo& InData);

	/** Toggle selection state */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetIsSelected(bool bSelected);

	/** Blueprint-implemented UI refresh logic */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Item")
	void OnDataUpdated();

	/** Blueprint-implemented selection state update */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Item")
	void OnSelectionStateChanged(bool bSelected);

	// ============ NEW: Additional Methods ============

	/** Set the item visual state */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetItemState(ETabItemState NewState);

	/** Get current item index in parent container */
	UFUNCTION(BlueprintPure, Category = "Tab Item")
	int32 GetItemIndex() const;

	/** Set selection manager */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetSelectionManager(UTabSelectionManager* Manager);

	/** Cancel any pending drag operation */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void CancelDrag();

	// ============ NEW: Blueprint Events for state changes ============

	/** Called when item state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Item")
	void OnItemStateChanged(ETabItemState NewState);

	/** Called when drag starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Item")
	void OnDragStartedEvent();

	/** Called when drag ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Item")
	void OnDragEndedEvent(bool bDropped);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============ NEW: Mouse Event Overrides for drag/right-click ============

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// ============ UI Event Bindings (Original) ============

	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void HandleItemClicked();

	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void HandleCloseClicked();

	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void HandleHovered();

	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void HandleUnhovered();

	// ============ NEW: Additional handlers ============

	/** Handle right click */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void HandleRightClicked(FVector2D ScreenPosition);

	/** Start drag operation */
	void StartDragOperation();

private:
	void BindButtonEvents();

	// ============ NEW: Drag tracking ============

	bool bMouseDownForDrag = false;
	float MouseDownTime = 0.0f;
	FVector2D MouseDownPosition;
};
