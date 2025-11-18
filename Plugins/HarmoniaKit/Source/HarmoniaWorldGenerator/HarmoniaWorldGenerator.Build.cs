// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaWorldGenerator: ModuleRules
{
    public HarmoniaWorldGenerator(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Landscape",
                "Foliage",
            });
    }
}
