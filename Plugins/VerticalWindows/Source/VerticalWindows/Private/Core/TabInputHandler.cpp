#include "TabInputHandler.h"
#include "Framework/Application/SlateApplication.h"

TWeakObjectPtr<UTabInputHandler> UTabInputHandler::GlobalInstance = nullptr;

UTabInputHandler::UTabInputHandler()
{
}

UTabInputHandler* UTabInputHandler::GetInputHandler(UObject* WorldContextObject)
{
	if (!GlobalInstance.IsValid())
	{
		GlobalInstance = NewObject<UTabInputHandler>();
		GlobalInstance->AddToRoot(); // Prevent GC
	}
	return GlobalInstance.Get();
}

FTabInputState UTabInputHandler::GetInputState() const
{
	// Always get fresh state from Slate for accuracy in editor
	FTabInputState State;

	if (FSlateApplication::IsInitialized())
	{
		FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
		State.bShiftDown = ModifierKeys.IsShiftDown();
		State.bCtrlDown = ModifierKeys.IsControlDown();
		State.bAltDown = ModifierKeys.IsAltDown();
	}

	return State;
}

bool UTabInputHandler::IsShiftDown() const
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	}
	return CurrentState.bShiftDown;
}

bool UTabInputHandler::IsCtrlDown() const
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetModifierKeys().IsControlDown();
	}
	return CurrentState.bCtrlDown;
}

bool UTabInputHandler::IsAltDown() const
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetModifierKeys().IsAltDown();
	}
	return CurrentState.bAltDown;
}

void UTabInputHandler::UpdateFromSlate()
{
	if (FSlateApplication::IsInitialized())
	{
		FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
		CurrentState.bShiftDown = ModifierKeys.IsShiftDown();
		CurrentState.bCtrlDown = ModifierKeys.IsControlDown();
		CurrentState.bAltDown = ModifierKeys.IsAltDown();
	}
}

void UTabInputHandler::SetShiftDown(bool bDown)
{
	CurrentState.bShiftDown = bDown;
}

void UTabInputHandler::SetCtrlDown(bool bDown)
{
	CurrentState.bCtrlDown = bDown;
}

// ============ Function Library ============

bool UTabInputFunctionLibrary::IsShiftKeyDown()
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetModifierKeys().IsShiftDown();
	}
	return false;
}

bool UTabInputFunctionLibrary::IsCtrlKeyDown()
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetModifierKeys().IsControlDown();
	}
	return false;
}

bool UTabInputFunctionLibrary::IsAltKeyDown()
{
	if (FSlateApplication::IsInitialized())
	{
		return FSlateApplication::Get().GetModifierKeys().IsAltDown();
	}
	return false;
}

FTabInputState UTabInputFunctionLibrary::GetCurrentModifierState()
{
	FTabInputState State;

	if (FSlateApplication::IsInitialized())
	{
		FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
		State.bShiftDown = ModifierKeys.IsShiftDown();
		State.bCtrlDown = ModifierKeys.IsControlDown();
		State.bAltDown = ModifierKeys.IsAltDown();
	}

	return State;
}
