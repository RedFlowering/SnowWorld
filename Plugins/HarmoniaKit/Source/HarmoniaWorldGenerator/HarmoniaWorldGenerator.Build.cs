// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaWorldGenerator : ModuleRules
{
    public HarmoniaWorldGenerator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Landscape",
                "Foliage",
                "HarmoniaLoadManager",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "RenderCore",
                "RHI",
            });

        // Editor-only modules for landscape editing
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "LandscapeEditor",
                    "UnrealEd",
                });
        }
    }
}
