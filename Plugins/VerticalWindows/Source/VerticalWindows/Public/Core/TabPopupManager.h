#pragma once

#include "CoreMinimal.h"
#include "TabPopupManager.generated.h"

class UUserWidget;

/**
 * Popup Window Manager - Creates borderless SWindow with UMG Widget inside
 * 
 * 用于管理编辑器中的弹出窗口，提供统一的创建、显示、关闭接口
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabPopupManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Show a popup window with UMG widget
	 * @param WidgetClass - The UMG widget blueprint class to display
	 * @param ScreenPosition - Screen position to show the popup
	 * @param Size - Optional size (0,0 = auto size)
	 * @param bCloseOnClickOutside - Auto close when clicking outside (default: true)
	 * @return The created widget instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Popup")
	static UUserWidget* ShowPopup(
		TSubclassOf<UUserWidget> WidgetClass,
		FVector2D ScreenPosition,
		FVector2D Size = FVector2D::ZeroVector,
		bool bCloseOnClickOutside = true
	);

	/**
	 * Show popup at cursor position
	 */
	UFUNCTION(BlueprintCallable, Category = "Popup")
	static UUserWidget* ShowPopupAtCursor(
		TSubclassOf<UUserWidget> WidgetClass,
		FVector2D Size = FVector2D::ZeroVector,
		bool bCloseOnClickOutside = true
	);

	/**
	 * Close popup window containing the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Popup")
	static void ClosePopup(UUserWidget* Widget);

	/**
	 * Close all popup windows
	 */
	UFUNCTION(BlueprintCallable, Category = "Popup")
	static void CloseAllPopups();
	
	/**
	 * Check if a widget is currently in a popup
	 */
	UFUNCTION(BlueprintPure, Category = "Popup")
	static bool IsPopupActive(UUserWidget* Widget);

private:
	// Track active popup windows
	static TMap<UUserWidget*, TWeakPtr<SWindow>> ActivePopups;
};
