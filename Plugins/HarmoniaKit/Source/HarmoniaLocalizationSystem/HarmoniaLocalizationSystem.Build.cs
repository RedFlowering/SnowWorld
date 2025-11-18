// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaLocalizationSystem: ModuleRules
{
    public HarmoniaLocalizationSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Json",
            "JsonUtilities",
        });
    }
}
