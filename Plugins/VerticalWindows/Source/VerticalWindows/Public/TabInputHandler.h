#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "TabInputHandler.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedInputComponent;

/**
 * Input State - tracks modifier key states
 */
USTRUCT(BlueprintType)
struct FTabInputState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bShiftDown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bCtrlDown = false;

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bAltDown = false;
};

/**
 * Tab Input Handler
 * Uses Enhanced Input System to track modifier key states
 * Singleton-like pattern for global input state access
 */
UCLASS(BlueprintType)
class VERTICALWINDOWS_API UTabInputHandler : public UObject
{
	GENERATED_BODY()

public:
	UTabInputHandler();

	// ============ Singleton Access ============

	/** Get or create the global input handler */
	UFUNCTION(BlueprintCallable, Category = "Tab Input", meta = (WorldContext = "WorldContextObject"))
	static UTabInputHandler* GetInputHandler(UObject* WorldContextObject);

	// ============ State Query ============

	/** Get current input state */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	FTabInputState GetInputState() const;

	/** Is Shift key down */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	bool IsShiftDown() const;

	/** Is Ctrl key down */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	bool IsCtrlDown() const;

	/** Is Alt key down */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	bool IsAltDown() const;

	// ============ Manual Update ============

	/** Update modifier state from Slate application (fallback method) */
	UFUNCTION(BlueprintCallable, Category = "Tab Input")
	void UpdateFromSlate();

	/** Manually set shift state */
	UFUNCTION(BlueprintCallable, Category = "Tab Input")
	void SetShiftDown(bool bDown);

	/** Manually set ctrl state */
	UFUNCTION(BlueprintCallable, Category = "Tab Input")
	void SetCtrlDown(bool bDown);

private:
	UPROPERTY()
	FTabInputState CurrentState;

	static TWeakObjectPtr<UTabInputHandler> GlobalInstance;
};

/**
 * Helper function library for input state
 */
UCLASS()
class VERTICALWINDOWS_API UTabInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Check if Shift is currently pressed (uses Slate) */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	static bool IsShiftKeyDown();

	/** Check if Ctrl is currently pressed (uses Slate) */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	static bool IsCtrlKeyDown();

	/** Check if Alt is currently pressed (uses Slate) */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	static bool IsAltKeyDown();

	/** Get all modifier states at once */
	UFUNCTION(BlueprintPure, Category = "Tab Input")
	static FTabInputState GetCurrentModifierState();
};
