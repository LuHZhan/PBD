// Copyright Epic Games, Inc. All Rights Reserved.

#include "VerticalWindowsCommands.h"

#define LOCTEXT_NAMESPACE "FVerticalWindowsModule"

void FVerticalWindowsCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "VerticalWindows", "Bring up VerticalWindows window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
