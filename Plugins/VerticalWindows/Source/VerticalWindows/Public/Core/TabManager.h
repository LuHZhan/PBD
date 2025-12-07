#pragma once

#include "CoreMinimal.h"
#include "TabTypes.h"
#include "ITabOperations.h"
#include "TabManager.generated.h"

class UTabItemWidget;
class UTabCommandInvoker;

// ============ 事件委托 ============
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTabListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, const TArray<FEditorTabInfo>&, SelectedTabs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabActivated, const FEditorTabInfo&, TabInfo);

/**
 * Tab Manager - 核心业务管理器
 * 
 * 职责:
 * - 实现 ITabOperations 接口（真正执行操作的地方）
 * - 管理标签数据（刷新、缓存）
 * - 管理选择状态（单选、多选、范围选）
 * - 管理自定义群组
 * - 持有 CommandInvoker
 * 
 * 设计原则:
 * - UI层（EUW_Windows）只负责显示，所有业务逻辑在这里
 * - TabItemWidget 直接调用 Manager 的方法
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabManager : public UObject, public ITabOperations
{
	GENERATED_BODY()

public:
	UTabManager();

	/** 初始化 Manager */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	void Initialize();

	/** 清理资源 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	void Shutdown();

	// ============ ITabOperations 接口实现 ============

	virtual bool ActivateTab(const FString& TabId) override;
	virtual bool CloseTab(const FString& TabId) override;
	virtual bool SaveAsset(const FString& AssetPath) override;
	virtual void BrowseToAsset(const FString& AssetPath) override;
	virtual bool AssignTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId) override;
	virtual int32 GetTabIndex(const FString& TabId) const override;
	virtual bool MoveTabToIndex(const FString& TabId, int32 NewIndex) override;

	// ============ 标签数据管理 ============

	/** 刷新标签列表（从编辑器获取） */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	void RefreshTabs();

	/** 获取所有已打开的标签 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager")
	TArray<FEditorTabInfo> GetAllTabs() const { return CachedTabs; }

	/** 获取分组后的标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	TArray<FTabGroupInfo> GetGroupedTabs() const;

	/** 按类型获取标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	TArray<FEditorTabInfo> GetTabsByType(const FString& AssetType) const;

	/** 是否应该使用分组显示 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager")
	bool ShouldUseGroupedDisplay() const;

	// ============ 选择管理 ============

	/** 处理标签点击（带修饰键支持） */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Selection")
	void HandleItemClick(const FEditorTabInfo& TabInfo, bool bShiftDown, bool bCtrlDown);

	/** 单选 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Selection")
	void SelectSingle(const FEditorTabInfo& TabInfo);

	/** 范围选择 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Selection")
	void SelectRange(const FEditorTabInfo& TabInfo);

	/** 切换选择状态 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Selection")
	void ToggleSelection(const FEditorTabInfo& TabInfo);

	/** 清除选择 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Selection")
	void ClearSelection();

	/** 全选 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Selection")
	void SelectAll();

	/** 是否已选中 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager|Selection")
	bool IsSelected(const FEditorTabInfo& TabInfo) const;

	/** 获取选中的标签 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager|Selection")
	TArray<FEditorTabInfo> GetSelectedTabs() const;

	/** 是否多选 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager|Selection")
	bool IsMultiSelection() const { return SelectedTabIds.Num() > 1; }

	/** 是否有选择 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager|Selection")
	bool HasSelection() const { return SelectedTabIds.Num() > 0; }

	// ============ 高层操作（通过 CommandInvoker） ============

	/** 打开标签（通过命令模式） */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool OpenTab(const FEditorTabInfo& Tab);

	/** 打开多个标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool OpenTabs(const TArray<FEditorTabInfo>& Tabs);

	/** 关闭标签（通过命令模式） */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool CloseTabByInfo(const FEditorTabInfo& Tab);

	/** 关闭多个标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool CloseTabs(const TArray<FEditorTabInfo>& Tabs);

	/** 关闭选中的标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool CloseSelectedTabs();

	/** 保存标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool SaveTab(const FEditorTabInfo& Tab);

	/** 保存多个标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool SaveTabs(const TArray<FEditorTabInfo>& Tabs);

	/** 保存所有脏标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	void SaveAllDirtyTabs();

	/** 关闭所有标签 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	void CloseAllTabs();

	/** 定位到资产 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool BrowseToAssetByInfo(const FEditorTabInfo& Tab);

	/** 添加到群组 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool AddTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId);

	/** 撤销上一个操作 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Operations")
	bool Undo();

	/** 是否可以撤销 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager|Operations")
	bool CanUndo() const;

	// ============ 自定义群组管理 ============

	/** 创建自定义群组 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Groups")
	void CreateCustomGroup(const FString& GroupName, FLinearColor GroupColor);

	/** 删除自定义群组 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager|Groups")
	void DeleteCustomGroup(const FString& GroupId);

	/** 获取自定义群组 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager|Groups")
	TArray<FCustomTabGroup> GetCustomGroups() const { return CustomGroups; }

	// ============ Widget 注册（用于批量更新UI） ============

	void RegisterItemWidget(UTabItemWidget* Widget);
	void UnregisterItemWidget(UTabItemWidget* Widget);
	void UpdateAllWidgetVisuals();

	// ============ 自动刷新 ============

	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	void StartAutoRefresh(float IntervalSeconds = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	void StopAutoRefresh();

	// ============ 设置 ============

	UPROPERTY(BlueprintReadWrite, Category = "Tab Manager")
	bool bEnableGrouping = true;

	// ============ 事件 ============

	/** 标签列表变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Tab Manager")
	FOnTabListChanged OnTabListChanged;

	/** 选择变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Tab Manager")
	FOnSelectionChanged OnSelectionChanged;

	/** 标签被激活时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Tab Manager")
	FOnTabActivated OnTabActivated;

	// ============ 工具方法 ============

	/** 获取资产类型颜色 */
	UFUNCTION(BlueprintPure, Category = "Tab Manager")
	FLinearColor GetAssetTypeColor(const FString& AssetType) const;

	/** 获取资产图标 */
	UFUNCTION(BlueprintCallable, Category = "Tab Manager")
	FSlateBrush GetAssetTypeBrush(const FString& AssetClassName) const;

protected:
	// ============ 内部方法 ============

	/** 生成标签列表 */
	TArray<FEditorTabInfo> GenerateTabList();

	/** 检查标签是否变化 */
	bool HasTabsChanged(const TArray<FEditorTabInfo>& NewTabs);

	/** 内部刷新 */
	void InternalRefresh();

	/** 通知选择变化 */
	void NotifySelectionChanged();

	/** 查找标签索引 */
	int32 FindTabIndex(const FString& TabId) const;

	/** 获取资产类型显示名 */
	FString GetAssetTypeDisplayName(UClass* AssetClass);

	/** 初始化群组颜色 */
	void InitGroupColors();

	/** 查找资产类 */
	UClass* FindAssetClassByName(const FString& ClassName);

private:
	// ============ 数据缓存 ============

	UPROPERTY()
	TArray<FEditorTabInfo> CachedTabs;

	UPROPERTY()
	TArray<FCustomTabGroup> CustomGroups;

	TMap<FString, FLinearColor> GroupColors;

	// ============ 选择状态 ============

	TSet<FString> SelectedTabIds;
	FString SelectionAnchorId;

	// ============ Widget 引用 ============

	UPROPERTY()
	TArray<TWeakObjectPtr<UTabItemWidget>> RegisteredWidgets;

	// ============ 命令系统 ============

	UPROPERTY()
	UTabCommandInvoker* CommandInvoker;

	// ============ 定时器 ============

	FTimerHandle AutoRefreshTimerHandle;
};
