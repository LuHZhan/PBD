// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VerticalWindows : ModuleRules
{
	public VerticalWindows(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"InputCore", 
				"JsEnv"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"EditorStyle",
				"EditorSubsystem",
				"AssetTools",
				"AssetRegistry",
				"ContentBrowser",
				"LevelEditor",
				"WorkspaceMenuStructure",
				"UMGEditor",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"Blutility",
				"UMG"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
