// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VerticalWindowsStyle.h"

class FVerticalWindowsCommands : public TCommands<FVerticalWindowsCommands>
{
public:

	FVerticalWindowsCommands()
		: TCommands<FVerticalWindowsCommands>(TEXT("VerticalWindows"), NSLOCTEXT("Contexts", "VerticalWindows", "VerticalWindows Plugin"), NAME_None, FVerticalWindowsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};