#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabContextMenu.generated.h"

class UEUW_Windows;
class UTabCommandInvoker;
class UButton;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuItemClicked, const FString&, MenuItemId);

/**
 * Menu Item Data
 */
USTRUCT(BlueprintType)
struct FTabMenuItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bHasSubMenu = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString SubMenuWidgetClass;
};

/**
 * Tab Context Menu - Right-click menu for tab items
 * Base class for Blueprint implementation
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabContextMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ Data ============

	/** Target tabs for this menu */
	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TArray<FEditorTabInfo> TargetTabs;

	/** Is multi-selection mode */
	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	bool bIsMultiSelection;

	/** Windows reference */
	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TWeakObjectPtr<UEUW_Windows> WindowsRef;

	// ============ Menu Items ============

	/** Menu items container - bind in Blueprint */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* MenuItemContainer;

	// ============ Events ============

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnMenuClosed OnMenuClosed;

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnMenuItemClicked OnMenuItemClicked;

	// ============ Methods ============

	/** Initialize menu with target tabs */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void InitializeMenu(UEUW_Windows* Windows, const TArray<FEditorTabInfo>& Tabs);

	/** Show the menu at position */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ShowAtPosition(FVector2D ScreenPosition);

	/** Close the menu */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void CloseMenu();

	/** Get available menu items */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	TArray<FTabMenuItemData> GetMenuItems() const;

	// ============ Menu Actions ============

	/** Execute menu action by ID */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ExecuteMenuAction(const FString& ActionId);

	/** Open selected tabs */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_Open();

	/** Close selected tabs */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_Close();

	/** Save selected tabs */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_Save();

	/** Browse to asset in content browser */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_BrowseToAsset();

	/** Show add to group submenu */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_ShowGroupSubMenu();

	// ============ Blueprint Events ============

	/** Called when menu is initialized - implement in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Context Menu")
	void OnMenuInitialized();

	/** Called to populate menu items - implement in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Context Menu")
	void OnPopulateMenuItems(const TArray<FTabMenuItemData>& Items);

	/** Called when submenu should be shown - implement in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Context Menu")
	void OnShowSubMenu(const FString& SubMenuId, FVector2D Position);

protected:
	virtual void NativeConstruct() override;

	/** Build default menu items */
	void BuildMenuItems();

	UPROPERTY()
	TArray<FTabMenuItemData> MenuItems;
};
