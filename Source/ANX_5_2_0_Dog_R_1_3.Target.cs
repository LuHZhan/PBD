// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class ANX_5_2_0_Dog_R_1_3Target : TargetRules
{
	public ANX_5_2_0_Dog_R_1_3Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "ANX_5_2_0_Dog_R_1_3" } );
	}
}
