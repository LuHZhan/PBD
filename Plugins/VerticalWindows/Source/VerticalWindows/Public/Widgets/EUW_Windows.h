#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Components/VerticalBox.h"
#include "TabTypes.h"
#include "EUW_Windows.generated.h"

class UTabManager;
class UTabContextMenu;
class UTabGroupSubMenu;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContextMenuRequested, const TArray<FEditorTabInfo>&, Tabs, FVector2D, ScreenPosition);

/**
 * Vertical Windows - 纯UI层
 * 
 * 职责:
 * - 创建和管理 Widget 实例
 * - 绑定事件到 TabManager
 * - 显示右键菜单
 * 
 * 不负责:
 * - 业务逻辑（委托给 TabManager）
 * - 选择状态管理（委托给 TabManager）
 * - 标签操作（委托给 TabManager）
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UEUW_Windows : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UEUW_Windows();

	// ============ 核心引用 ============

	/** Tab Manager - 所有业务逻辑的入口 */
	UPROPERTY(BlueprintReadOnly, Category = "Vertical Windows")
	UTabManager* TabManager;

	// ============ UI 容器 ============

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* ItemContainer;

	// ============ Widget 类引用 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical Windows|Classes")
	TSubclassOf<class UTabGroupWidget> TabGroupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical Windows|Classes")
	TSubclassOf<class UTabItemWidget> TabItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical Windows|Menus")
	TSubclassOf<UTabContextMenu> ContextMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical Windows|Menus")
	TSubclassOf<UTabGroupSubMenu> GroupSubMenuClass;

	// ============ 事件（用于蓝图扩展） ============

	UPROPERTY(BlueprintAssignable, Category = "Vertical Windows")
	FOnContextMenuRequested OnContextMenuRequested;

	// ============ UI 操作 ============

	/** 强制刷新UI */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void RefreshUI();

	/** 显示右键菜单 */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void ShowContextMenu(const TArray<FEditorTabInfo>& Tabs, FVector2D ScreenPosition);

	/** 显示选中标签的右键菜单 */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void ShowContextMenuForSelection(FVector2D ScreenPosition);

	/** 关闭右键菜单 */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void CloseContextMenu();

	// ============ 便捷访问（委托到 TabManager） ============

	/** 获取所有标签 */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	TArray<FEditorTabInfo> GetAllTabs() const;

	/** 获取分组标签 */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	TArray<FTabGroupInfo> GetGroupedTabs() const;

	/** 获取选中的标签 */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	TArray<FEditorTabInfo> GetSelectedTabs() const;

	/** 是否启用分组 */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	bool IsGroupingEnabled() const;

	/** 设置是否启用分组 */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void SetEnableGrouping(bool bEnable);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// ============ UI 构建 ============

	UFUNCTION()
	void RebuildUI();

	void BuildFlatList();
	void BuildGroupedList();

	// ============ 事件处理（转发到 TabManager） ============

	UFUNCTION()
	void HandleItemClicked(const FEditorTabInfo& TabInfo); 

	UFUNCTION()
	void HandleItemClosed(const FEditorTabInfo& TabInfo);

	UFUNCTION()
	void HandleItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition);

	UFUNCTION()
	void HandleGroupItemClicked(const FTabGroupInfo& GroupData, const FEditorTabInfo& TabInfo);

	UFUNCTION()
	void HandleGroupItemClosed(const FTabGroupInfo& GroupData, const FEditorTabInfo& TabInfo);

	UFUNCTION()
	void HandleGroupItemRightClicked(const FTabGroupInfo& GroupData, const FEditorTabInfo& TabInfo, FVector2D ScreenPosition);
	void TestPopupWindow();

	// ============ 菜单状态 ============

	UPROPERTY()
	UTabContextMenu* ActiveContextMenu;
	
	// 🔧 Slate Window 引用（用于编辑器菜单）
	TWeakPtr<SWindow> ActiveMenuWindow;
};
