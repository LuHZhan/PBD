#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabGroupSubMenu.generated.h"

class UTabManager;
class UVerticalBox;
class UButton;
class UTabGroupItem;
class UTabCreateGroupDialog;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroupSelected, const FString&, GroupId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSubMenuClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewGroupCreated, const FString&, GroupName, FLinearColor, GroupColor);

/**
 * Group Info for SubMenu display
 */
USTRUCT(BlueprintType)
struct FGroupMenuItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString GroupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FLinearColor GroupColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bIsNewGroup = false;
};

/**
 * Tab Group SubMenu - Shows available groups to add tabs to
 * 
 * 重构后：使用 TabManager 而不是 UEUW_Windows
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabGroupSubMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ Data ============

	/** Target tabs to be added to group */
	UPROPERTY(BlueprintReadOnly, Category = "Group SubMenu")
	TArray<FEditorTabInfo> TargetTabs;

	/** Available groups */
	UPROPERTY(BlueprintReadOnly, Category = "Group SubMenu")
	TArray<FGroupMenuItemData> AvailableGroups;

	/** Manager reference */
	UPROPERTY(BlueprintReadOnly, Category = "Group SubMenu")
	TWeakObjectPtr<UTabManager> TabManagerRef;

	// ============ Component Bindings ============

	/** Container for group items */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* GroupItemContainer;

	/** Create new group button */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UButton* CreateGroupButton;

	// ============ Class References ============

	/** GroupItem widget class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group SubMenu")
	TSubclassOf<UTabGroupItem> GroupItemClass;

	/** Create Group Dialog class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group SubMenu")
	TSubclassOf<UTabCreateGroupDialog> CreateGroupDialogClass;

	// ============ Events ============

	UPROPERTY(BlueprintAssignable, Category = "Group SubMenu")
	FOnGroupSelected OnGroupSelected;

	UPROPERTY(BlueprintAssignable, Category = "Group SubMenu")
	FOnSubMenuClosed OnSubMenuClosed;

	UPROPERTY(BlueprintAssignable, Category = "Group SubMenu")
	FOnNewGroupCreated OnNewGroupCreated;

	// ============ Methods ============

	/** Initialize submenu with TabManager */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void InitializeSubMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs);

	/** Show at position */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void ShowAtPosition(FVector2D ScreenPosition);

	/** Close submenu */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void CloseSubMenu();

	/** Select a group */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void SelectGroup(const FString& GroupId);

	/** Show create group dialog */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void ShowCreateGroupDialog();

	/** Create new group with name and color */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void CreateNewGroup(const FString& GroupName, FLinearColor GroupColor);

	/** Get available groups */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	TArray<FGroupMenuItemData> GetAvailableGroups() const;

	/** Rebuild group item list */
	UFUNCTION(BlueprintCallable, Category = "Group SubMenu")
	void RebuildGroupItems();

	// ============ Blueprint Events ============

	/** Called when submenu is initialized */
	UFUNCTION(BlueprintImplementableEvent, Category = "Group SubMenu")
	void OnSubMenuInitialized();

	/** Called to populate group items */
	UFUNCTION(BlueprintImplementableEvent, Category = "Group SubMenu")
	void OnPopulateGroupItems(const TArray<FGroupMenuItemData>& Groups);

protected:
	virtual void NativeConstruct() override;

	/** Build available groups list */
	void BuildGroupList();

	UFUNCTION()
	void HandleGroupItemClicked(const FString& GroupId);

	UFUNCTION()
	void HandleCreateGroupClicked();

	UFUNCTION()
	void HandleGroupCreated(const FString& GroupName, FLinearColor GroupColor);

	UFUNCTION()
	void HandleDialogCancelled();

private:
	/** Active create group dialog */
	UPROPERTY()
	UTabCreateGroupDialog* ActiveDialog;

	/** Created group item widgets */
	UPROPERTY()
	TArray<UTabGroupItem*> GroupItemWidgets;
};
