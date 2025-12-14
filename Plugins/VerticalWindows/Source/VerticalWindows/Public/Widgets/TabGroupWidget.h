#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/ExpandableArea.h"
#include "Components/TextBlock.h"
#include "TabTypes.h"
#include "Components/Image.h"
#include "TabGroupWidget.generated.h"

class UTabItemWidget;
class UTabManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupToggle, const FTabGroupInfo&, GroupData, bool, bExpanded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGroupItemRightClicked, const FTabGroupInfo&, GroupData, const FEditorTabInfo&, TabInfo, FVector2D, ScreenPosition);

/**
 * Tab Group Widget - 群组容器
 * 
 * 重构后的设计:
 * - 持有 TabManager 引用并传递给子项
 * - 只转发右键点击事件（用于菜单显示）
 * - 其他操作由子项直接调用 TabManager
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabGroupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ 数据 ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Group")
	FTabGroupInfo GroupData;

	// ============ Manager 引用 ============

	UPROPERTY(BlueprintReadWrite, Category = "Tab Group")
	TWeakObjectPtr<UTabManager> TabManager;

	// ============ 组件绑定 ============

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* ItemContainer;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* HeaderButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UExpandableArea* GroupExpandableArea;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* GroupNameText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UImage* ExpandableAreaBG;

	// ============ 事件 ============

	UPROPERTY(BlueprintAssignable, Category = "Tab Group")
	FOnGroupToggle OnToggle;

	/** 右键点击事件（转发给 UI 层处理菜单） */
	UPROPERTY(BlueprintAssignable, Category = "Tab Group")
	FOnGroupItemRightClicked OnItemRightClicked;

	// ============ 子项跟踪 ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Group")
	TArray<UTabItemWidget*> ChildItemWidgets;

	// ============ 方法 ============

	/** 设置 Manager */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void SetTabManager(UTabManager* Manager);

	/** 设置群组数据 */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void SetGroupData(const FTabGroupInfo& InData);

	/** 添加标签数据（仅数据） */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void AddTab(const FEditorTabInfo& TabInfo);

	/** 添加子 Widget */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void AddChildWidget(UUserWidget* Widget);

	/** 清除标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void ClearTabs();

	/** 设置展开状态 */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void SetExpanded(bool bExpanded);

	/** 获取子项 */
	UFUNCTION(BlueprintPure, Category = "Tab Group")
	TArray<UTabItemWidget*> GetChildItemWidgets() const { return ChildItemWidgets; }

	// ============ 蓝图事件 ============

	UFUNCTION(BlueprintNativeEvent, Category = "Tab Group")
	void OnGroupDataUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Group")
	void OnTabAdded(const FEditorTabInfo& TabInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Group")
	void OnTabsCleared();

	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Group")
	void OnExpansionStateChanged(bool bExpanded);

protected:
	virtual void NativeConstruct() override;
	
	// 🆕 新增方法
	UFUNCTION()
	void HandleHeaderButtonClicked();
	
	/** 更新背景颜色 */
	void UpdateExpandableAreaColor();
	
	/** 更新群组名称 */
	void UpdateGroupNameText();
	
	// 原有方法
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void HandleHeaderClicked();

	UFUNCTION()
	void HandleChildItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition);
};
