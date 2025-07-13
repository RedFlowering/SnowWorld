// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaInstancingSystem: ModuleRules
{
    public HarmoniaInstancingSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "HarmoniaCore",
                "HarmoniaLoadManager",
            });
    }
}
