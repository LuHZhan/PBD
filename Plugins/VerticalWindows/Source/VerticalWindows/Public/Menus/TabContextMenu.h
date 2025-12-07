#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TabTypes.h"
#include "TabContextMenu.generated.h"

class UTabManager;
class UTabGroupSubMenu;
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
 * Tab Context Menu - 右键菜单
 * 
 * 重构后的设计:
 * - 使用 TabManager 而不是 UEUW_Windows
 * - 所有操作通过 TabManager 执行
 */
UCLASS(BlueprintType, Blueprintable)
class VERTICALWINDOWS_API UTabContextMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============ 数据 ============

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TArray<FEditorTabInfo> TargetTabs;

	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	bool bIsMultiSelection;

	/** Manager 引用 */
	UPROPERTY(BlueprintReadOnly, Category = "Context Menu")
	TWeakObjectPtr<UTabManager> TabManagerRef;

	// ============ 组件 ============

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UVerticalBox* MenuItemContainer;

	// ============ 子菜单类引用 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context Menu|Classes")
	TSubclassOf<UTabGroupSubMenu> GroupSubMenuClass;

	// ============ 事件 ============

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnMenuClosed OnMenuClosed;

	UPROPERTY(BlueprintAssignable, Category = "Context Menu")
	FOnMenuItemClicked OnMenuItemClicked;

	// ============ 方法 ============

	/** 初始化菜单 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void InitializeMenu(UTabManager* Manager, const TArray<FEditorTabInfo>& Tabs);

	/** 显示在指定位置 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ShowAtPosition(FVector2D ScreenPosition);

	/** 关闭菜单 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void CloseMenu();

	/** 获取菜单项 */
	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	TArray<FTabMenuItemData> GetMenuItems() const;

	// ============ 菜单操作 ============

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ExecuteMenuAction(const FString& ActionId);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_Open();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_Close();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_Save();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_BrowseToAsset();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void MenuAction_ShowGroupSubMenu();

	// ============ 蓝图事件 ============

	UFUNCTION(BlueprintImplementableEvent, Category = "Context Menu")
	void OnMenuInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "Context Menu")
	void OnPopulateMenuItems(const TArray<FTabMenuItemData>& Items);

	UFUNCTION(BlueprintImplementableEvent, Category = "Context Menu")
	void OnShowSubMenu(const FString& SubMenuId, FVector2D Position);

protected:
	virtual void NativeConstruct() override;

	void BuildMenuItems();

	UPROPERTY()
	TArray<FTabMenuItemData> MenuItems;

	/** 活动的群组子菜单 */
	UPROPERTY()
	UTabGroupSubMenu* ActiveGroupSubMenu;
};
