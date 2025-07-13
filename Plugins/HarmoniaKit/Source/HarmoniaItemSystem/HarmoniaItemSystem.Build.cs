// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaItemSystem: ModuleRules
{
    public HarmoniaItemSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
        new string[]
        {
            "HarmoniaCore",
            "HarmoniaInventorySystem",
        });

        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine",
            });
    }
}
