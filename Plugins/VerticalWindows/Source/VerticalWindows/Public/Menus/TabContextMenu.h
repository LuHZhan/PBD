#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabContextMenu.generated.h"

class UTabManager;
class UTabGroupSubMenu;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuItemClicked, const FString&, MenuItemId);

/**
 * Tab Context Menu - 右键菜单
 * 
 * 设计原则:
 * - 蓝图只负责搭建UI（摆放按钮、设置样式）
 * - C++ 负责所有逻辑（绑定事件、状态管理、操作执行）
 * - 通过 BindWidget 自动关联蓝图中的组件
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabContextMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ UI 组件（通过 BindWidget 关联蓝图） ============

	/** Open 按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* OpenButton;

	/** Save 按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* SaveButton;

	/** Close 按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* CloseButton;

	/** Browse to Asset 按钮 (可选，单选时显示) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* BrowseButton;

	/** Create/Add Group 按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* GroupButton;

	// ============ 可选的文本组件 ============

	/** 标题文本 (可选，显示选中的标签数量) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UTextBlock* TitleText;

	// ============ 数据 ============

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TArray<FEditorTabInfo> TargetTabs;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	bool bIsMultiSelection;

	/** Manager 引用 */
	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TWeakObjectPtr<UTabManager> TabManagerRef;

	// ============ 子菜单类引用 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu|Classes")
	TSubclassOf<UTabGroupSubMenu> GroupSubMenuClass;

	// ============ 事件委托（供蓝图监听） ============

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnMenuClosed OnMenuClosed;

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnMenuItemClicked OnMenuItemClicked;

	// ============ 公共方法 ============

	/** 初始化菜单 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void InitializeMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs);

	/** 显示在指定位置 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ShowAtPosition(FVector2D ScreenPosition);

	/** 关闭菜单 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void CloseMenu();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ============ 按钮点击处理 ============

	UFUNCTION()
	void HandleOpenClicked();

	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleBrowseClicked();

	UFUNCTION()
	void HandleGroupClicked();

	// ============ 内部方法 ============

	/** 绑定所有按钮事件 */
	void BindButtons();

	/** 更新按钮状态（启用/禁用/可见性） */
	void UpdateButtonStates();

	/** 获取仍然有效的 Tab（过滤已关闭的） */
	TArray<FEditorTabInfo> GetValidTabs() const;

	/** 是否有脏 Tab */
	bool HasDirtyTabs() const;

	/** 活动的群组子菜单 */
	UPROPERTY()
	UTabGroupSubMenu* ActiveGroupSubMenu;
};
