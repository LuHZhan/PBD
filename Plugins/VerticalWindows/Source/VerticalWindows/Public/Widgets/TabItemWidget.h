#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabItemWidget.generated.h"

class UWidgetSwitcher;
class UImage;
class UTextBlock;
class UButton;
class UTabManager;
class UTabDragDropOperation;

// 保留事件用于特殊情况（如右键菜单需要UI处理）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabItemRightClicked, const FEditorTabInfo&, TabInfo, FVector2D, ScreenPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabItemDragStarted, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabItemDragEnded, const FEditorTabInfo&, TabInfo, bool, bDropped);

/**
 * Tab Item Widget - 单个标签项
 * 
 * 方案B设计:
 * - Widget 本身接收鼠标事件 (Visibility: Visible)
 * - 移除 RootButton，使用 Native 鼠标事件处理点击
 * - 只保留 CloseButton 单独处理关闭
 * - 直接持有 TabManager 引用，操作直接调用 Manager
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ 数据 ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Item")
	FEditorTabInfo TabData;

	UPROPERTY(BlueprintReadOnly, Category = "Tab Item")
	ETabItemState CurrentState = ETabItemState::Normal;

	// ============ Manager 引用 ============

	/** Tab Manager - 所有操作的入口 */
	UPROPERTY(BlueprintReadWrite, Category = "Tab Item")
	TWeakObjectPtr<UTabManager> TabManager;

	// ============ 组件绑定 ============

	/** 背景切换器 (普通/选中状态) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* BGWidgetSwitcher;

	/** 普通状态背景 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* NoHoveredBG;

	/** 选中/悬停状态背景 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* Background;

	/** 资产图标 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* IconImage;

	/** 显示名称 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* ItemText;

	/** 脏标记 (星号) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UTextBlock* DirtyIndicator;

	/** 关闭按钮 - 单独处理关闭点击 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* CloseButton;

	// ============ 保留的事件（特殊情况） ============

	/** 右键点击 - 需要UI层处理菜单显示 */
	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemRightClicked OnRightClicked;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemDragStarted OnDragStarted;

	UPROPERTY(BlueprintAssignable, Category = "Tab Item")
	FOnTabItemDragEnded OnDragEnded;

	// ============ 拖拽设置 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tab Item|Drag")
	float DragHoldTime = 0.3f;

	UPROPERTY(BlueprintReadOnly, Category = "Tab Item|Drag")
	bool bIsDragging = false;

	// ============ 设置方法 ============

	/** 设置 Manager */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetTabManager(UTabManager* Manager);

	/** 设置数据并刷新UI */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetTabData(const FEditorTabInfo& InData);

	/** 设置选中状态 */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetIsSelected(bool bSelected);

	/** 设置视觉状态 */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void SetItemState(ETabItemState NewState);

	/** 获取当前索引 */
	UFUNCTION(BlueprintPure, Category = "Tab Item")
	int32 GetItemIndex() const;

	/** 取消拖拽 */
	UFUNCTION(BlueprintCallable, Category = "Tab Item")
	void CancelDrag();

	// ============ 蓝图事件 ============

	/** 数据更新时调用 */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Item")
	void OnDataUpdated();

	/** 选中状态变化时调用 */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Item")
	void OnSelectionStateChanged(bool bSelected);

	/** 视觉状态变化时调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Item")
	void OnItemStateChanged(ETabItemState NewState);

	/** 拖拽开始时调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Item")
	void OnDragStartedEvent();

	/** 拖拽结束时调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Item")
	void OnDragEndedEvent(bool bDropped);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============ 鼠标事件重写 (方案B核心) ============

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	// ============ 内部处理 ============

	/** 处理点击（左键释放时） */
	void HandleItemClicked();

	/** 处理关闭按钮点击 */
	UFUNCTION()
	void HandleCloseClicked();

	/** 处理右键点击 */
	void HandleRightClicked(FVector2D ScreenPosition);

	/** 处理悬停进入 */
	void HandleHovered();

	/** 处理悬停离开 */
	void HandleUnhovered();

	/** 开始拖拽操作 */
	void StartDragOperation();

private:
	void BindCloseButton();

	// 拖拽跟踪
	bool bMouseDownForDrag = false;
	float MouseDownTime = 0.0f;
	FVector2D MouseDownPosition;
};