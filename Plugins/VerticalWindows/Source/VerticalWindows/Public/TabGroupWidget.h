#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "TabTypes.h"
#include "TabGroupWidget.generated.h"

class UTabItemWidget;
class UTabSelectionManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupToggle, const FTabGroupInfo&, GroupData, bool, bExpanded);

// ============ NEW: Event forwarding delegates ============
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabInGroupClicked, const FTabGroupInfo&, GroupData, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupItemClosed, const FTabGroupInfo&, GroupData, const FEditorTabInfo&, TabInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGroupItemRightClicked, const FTabGroupInfo&, GroupData, const FEditorTabInfo&, TabInfo, FVector2D, ScreenPosition);

/**
 * Vertical Tab - Group Widget Base Class
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabGroupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ Data ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Group")
	FTabGroupInfo GroupData;

	// ============ Component Bindings (Native) ============

	/**
	 * Must create a VerticalBox named ItemContainer in Blueprint.
	 * This is more efficient and standard than manual registration.
	 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* ItemContainer;

	// ============ Event Callbacks (Original) ============

	UPROPERTY(BlueprintAssignable, Category = "Tab Group")
	FOnGroupToggle OnToggle;

	// ============ NEW: Event Forwarding Callbacks ============

	/** Fired when any item in this group is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Tab Group")
	FOnTabInGroupClicked OnItemClicked;

	/** Fired when any item in this group is closed */
	UPROPERTY(BlueprintAssignable, Category = "Tab Group")
	FOnGroupItemClosed OnItemClosed;

	/** Fired when any item in this group is right-clicked */
	UPROPERTY(BlueprintAssignable, Category = "Tab Group")
	FOnGroupItemRightClicked OnItemRightClicked;

	// ============ NEW: Selection Manager Reference ============

	UPROPERTY(BlueprintReadWrite, Category = "Tab Group")
	TWeakObjectPtr<UTabSelectionManager> SelectionManager;

	// ============ NEW: Child item widgets ============

	UPROPERTY(BlueprintReadOnly, Category = "Tab Group")
	TArray<UTabItemWidget*> ChildItemWidgets;

	// ============ Methods (Original) ============

	/** Set group data */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void SetGroupData(const FTabGroupInfo& InData);

	/** Add tab data (data only) */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void AddTab(const FEditorTabInfo& TabInfo);

	/** Add child widget to container */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void AddChildWidget(UUserWidget* Widget);

	/** Clear tabs */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void ClearTabs();

	/** Set expanded state */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void SetExpanded(bool bExpanded);

	/** Blueprint-implemented UI refresh logic */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab Group")
	void OnGroupDataUpdated();

	/** Blueprint-implemented add tab logic */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Group")
	void OnTabAdded(const FEditorTabInfo& TabInfo);

	/** Blueprint-implemented clear logic */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Group")
	void OnTabsCleared();

	/** Blueprint-implemented expanded state update */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tab Group")
	void OnExpansionStateChanged(bool bExpanded);

	// ============ NEW: Additional Methods ============

	/** Add a TabItemWidget and bind its events */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void AddTabItemWidget(UTabItemWidget* ItemWidget);

	/** Set selection manager for all child items */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void SetSelectionManager(UTabSelectionManager* Manager);

	/** Get all child item widgets */
	UFUNCTION(BlueprintPure, Category = "Tab Group")
	TArray<UTabItemWidget*> GetChildItemWidgets() const { return ChildItemWidgets; }

protected:
	// ============ UI Event Bindings ============

	/** Bind to header click */
	UFUNCTION(BlueprintCallable, Category = "Tab Group")
	void HandleHeaderClicked();

	// ============ NEW: Item Event Handlers ============

	UFUNCTION()
	void HandleChildItemClicked(const FEditorTabInfo& TabInfo);

	UFUNCTION()
	void HandleChildItemClosed(const FEditorTabInfo& TabInfo);

	UFUNCTION()
	void HandleChildItemRightClicked(const FEditorTabInfo& TabInfo, FVector2D ScreenPosition);
};
