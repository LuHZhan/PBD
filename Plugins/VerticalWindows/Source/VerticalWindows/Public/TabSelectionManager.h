#pragma once

#include "CoreMinimal.h"
#include "TabTypes.h"
#include "TabSelectionManager.generated.h"

class UTabItemWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectionChanged, const TArray<FEditorTabInfo>&, SelectedTabs);

/**
 * Selection Mode
 */
UENUM(BlueprintType)
enum class ETabSelectionMode : uint8
{
	/** Single selection - click replaces selection */
	Single,
	/** Range selection - Shift+click selects range */
	Range,
	/** Toggle selection - Ctrl+click toggles individual items */
	Toggle
};

/**
 * Tab Selection Manager
 * Manages multi-selection state with Shift/Ctrl modifier support
 * Uses Mediator pattern - centralized selection management
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabSelectionManager : public UObject
{
	GENERATED_BODY()

public:
	UTabSelectionManager();

	// ============ Selection Operations ============

	/** Handle item click with modifier keys */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void HandleItemClick(const FEditorTabInfo& TabInfo, bool bShiftDown, bool bCtrlDown);

	/** Select single item (clears other selections) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectSingle(const FEditorTabInfo& TabInfo);

	/** Select range from anchor to target */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectRange(const FEditorTabInfo& TabInfo);

	/** Toggle item selection */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ToggleSelection(const FEditorTabInfo& TabInfo);

	/** Add to selection */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void AddToSelection(const FEditorTabInfo& TabInfo);

	/** Remove from selection */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void RemoveFromSelection(const FEditorTabInfo& TabInfo);

	/** Clear all selections */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSelection();

	/** Select all tabs */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectAll();

	// ============ Query ============

	/** Check if tab is selected */
	UFUNCTION(BlueprintPure, Category = "Selection")
	bool IsSelected(const FEditorTabInfo& TabInfo) const;

	/** Get selected tabs */
	UFUNCTION(BlueprintPure, Category = "Selection")
	TArray<FEditorTabInfo> GetSelectedTabs() const;

	/** Get selection count */
	UFUNCTION(BlueprintPure, Category = "Selection")
	int32 GetSelectionCount() const;

	/** Has any selection */
	UFUNCTION(BlueprintPure, Category = "Selection")
	bool HasSelection() const;

	/** Is multi-selection active */
	UFUNCTION(BlueprintPure, Category = "Selection")
	bool IsMultiSelection() const;

	// ============ Tab List Management ============

	/** Set the full tab list (needed for range selection) */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SetTabList(const TArray<FEditorTabInfo>& InTabs);

	/** Update tab list */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void UpdateTabList(const TArray<FEditorTabInfo>& InTabs);

	// ============ Widget Registration ============

	/** Register a tab item widget */
	void RegisterItemWidget(UTabItemWidget* Widget);

	/** Unregister a tab item widget */
	void UnregisterItemWidget(UTabItemWidget* Widget);

	/** Update visual state of all registered widgets */
	void UpdateAllWidgetVisuals();

	// ============ Events ============

	UPROPERTY(BlueprintAssignable, Category = "Selection")
	FOnSelectionChanged OnSelectionChanged;

private:
	/** Find tab index in list */
	int32 FindTabIndex(const FString& TabId) const;

	/** Notify selection changed */
	void NotifySelectionChanged();

	/** Current selection anchor (for range selection) */
	UPROPERTY()
	FString SelectionAnchorId;

	/** Selected tab IDs */
	UPROPERTY()
	TSet<FString> SelectedTabIds;

	/** Full tab list (for range selection) */
	UPROPERTY()
	TArray<FEditorTabInfo> AllTabs;

	/** Registered item widgets */
	UPROPERTY()
	TArray<TWeakObjectPtr<UTabItemWidget>> RegisteredWidgets;
};
