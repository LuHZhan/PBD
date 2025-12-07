#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Components/VerticalBox.h"
#include "TabTypes.h"
#include "EUW_Windows.generated.h"

class UTexture2D;
class UTabSelectionManager;
class UTabCommandInvoker;
class UTabContextMenu;
class UTabGroupSubMenu;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEditorTabsChanged);

// ============ NEW: Additional delegates ============
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTabActivated, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnContextMenuRequested, const TArray<FEditorTabInfo>&, Tabs, FVector2D, ScreenPosition);

UCLASS(BlueprintType)
class VERTICALWINDOWS_API UEUW_Windows : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UEUW_Windows();

	// ============ Get Tabs ============

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	TArray<FEditorTabInfo> RefreshAllOpenTabs();

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	TArray<FEditorTabInfo> GetAllOpenTabs();

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	TArray<FTabGroupInfo> GetGroupedTabs();

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	TArray<FEditorTabInfo> GetTabsByType(const FString& AssetType);

	// ============ Tab Operations ============

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	bool ActivateTab(const FString& TabId);

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	bool CloseTab(const FString& TabId);

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void BrowseToAsset(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	bool SaveAsset(const FString& AssetPath);

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void CloseAllTabs();

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void SaveAllDirtyAssets();

	// ============ Refresh ============

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void RefreshTabs();

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void StartAutoRefresh(float IntervalSeconds = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void StopAutoRefresh();

	// ============ Grouping ============

	/** Enable or disable grouping display */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void SetEnableGrouping(bool bEnable);

	/** Check if grouping is enabled */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	bool IsGroupingEnabled() const { return bEnableGrouping; }

	/** Check if grouped display should be used (more than one group exists) */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	bool ShouldUseGroupedDisplay() const;

	// ============ Asset Icons ============

	/** Get icon texture for asset class name */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	UTexture2D* GetAssetTypeIcon(const FString& AssetClassName);

	/** Get icon brush for asset class (returns FSlateBrush as struct) */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	FSlateBrush GetAssetTypeBrush(const FString& AssetClassName);

	// ============ Utilities ============

	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	TMap<FString, FLinearColor> GetGroupColorMap();

	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	static FLinearColor ParseHexColor(const FString& HexColor);

	// ============ Events (Original) ============

	UPROPERTY(BlueprintAssignable, Category = "Vertical Windows")
	FOnEditorTabsChanged OnTabsChanged;

	// ============ NEW: Additional Events ============

	/** Fired when a tab is activated (opened) */
	UPROPERTY(BlueprintAssignable, Category = "Vertical Windows")
	FOnTabActivated OnTabActivated;

	/** Fired when context menu is requested */
	UPROPERTY(BlueprintAssignable, Category = "Vertical Windows")
	FOnContextMenuRequested OnContextMenuRequested;

	// ============ UI Class References ============

	UPROPERTY(BlueprintReadWrite, Category = "Vertical Windows")
	TSubclassOf<class UTabGroupWidget> TabGroupClass;

	UPROPERTY(BlueprintReadWrite, Category = "Vertical Windows")
	TSubclassOf<class UTabItemWidget> TabItemClass;

	// ============ NEW: Menu Class References ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical Windows|Menus")
	TSubclassOf<UTabContextMenu> ContextMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical Windows|Menus")
	TSubclassOf<UTabGroupSubMenu> GroupSubMenuClass;

	// ============ Settings ============

	/** Whether to enable grouping display. If false, always show flat list */
	UPROPERTY(BlueprintReadWrite, Category = "Vertical Windows")
	bool bEnableGrouping = true;

	// ============ Component Bindings (Native) ============

	/**
	 * Must create a VerticalBox named ItemContainer in Blueprint
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* ItemContainer;

	// ============ NEW: Selection Manager ============

	/** Selection manager for multi-select support */
	UPROPERTY(BlueprintReadOnly, Category = "Vertical Windows")
	UTabSelectionManager* SelectionManager;

	/** Command invoker for undo support */
	UPROPERTY(BlueprintReadOnly, Category = "Vertical Windows")
	UTabCommandInvoker* CommandInvoker;

	// ============ NEW: Custom Groups ============

	/** User-defined custom groups */
	UPROPERTY(BlueprintReadWrite, Category = "Vertical Windows|Groups")
	TArray<FCustomTabGroup> CustomGroups;

	// ============ NEW: Methods for Command Pattern support ============

	/** Assign tabs to a group (for command pattern) */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	bool AssignTabsToGroup(const TArray<FEditorTabInfo>& Tabs, const FString& GroupId);

	/** Get tab index by ID */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	int32 GetTabIndex(const FString& TabId) const;

	/** Move tab to index */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	bool MoveTabToIndex(const FString& TabId, int32 NewIndex);

	/** Create a custom group */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void CreateCustomGroup(const FString& GroupName);

	/** Delete a custom group */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void DeleteCustomGroup(const FString& GroupId);

	/** Get available custom groups */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	TArray<FCustomTabGroup> GetCustomGroups() const { return CustomGroups; }

	// ============ NEW: Context Menu Methods ============

	/** Show context menu for tabs */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void ShowContextMenu(const TArray<FEditorTabInfo>& Tabs, FVector2D ScreenPosition);

	/** Show context menu for selected tabs */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void ShowContextMenuForSelection(FVector2D ScreenPosition);

	/** Close any open context menu */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void CloseContextMenu();

	// ============ NEW: Selection Methods ============

	/** Get selected tabs */
	UFUNCTION(BlueprintPure, Category = "Vertical Windows")
	TArray<FEditorTabInfo> GetSelectedTabs() const;

	/** Clear selection */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void ClearSelection();

	/** Select all tabs */
	UFUNCTION(BlueprintCallable, Category = "Vertical Windows")
	void SelectAllTabs();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void InternalRefresh();

	// Smart refresh helpers
	TArray<FEditorTabInfo> GenerateTabList();
	bool HasTabsChanged(const TArray<FEditorTabInfo>& NewTabs);

	UFUNCTION()
	void RebuildUI();

	FString GetAssetTypeDisplayName(UClass* AssetClass);
	FLinearColor GetAssetTypeColor(const FString& AssetType);
	void InitGroupColors();

	/** Find UClass by class name string */
	UClass* FindAssetClassByName(const FString& ClassName);

	// ============ NEW: Event Handlers ============

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

	FTimerHandle AutoRefreshTimerHandle;
	TArray<FEditorTabInfo> CachedTabs;
	TMap<FString, FLinearColor> GroupColors;

	// ============ NEW: Active context menu ============

	UPROPERTY()
	UTabContextMenu* ActiveContextMenu;
};
