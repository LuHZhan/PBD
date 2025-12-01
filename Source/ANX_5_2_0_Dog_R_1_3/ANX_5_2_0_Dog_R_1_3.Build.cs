using UnrealBuildTool;

public class ANX_5_2_0_Dog_R_1_3 : ModuleRules
{
    public ANX_5_2_0_Dog_R_1_3(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Puerts",
            "JsEnv",
            "Json",           // ← 添加这个
            "JsonUtilities"   // ← 添加这个
        });

        // 编辑器模块只在编辑器构建时添加
        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "BlueprintGraph"
            });
        }

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
