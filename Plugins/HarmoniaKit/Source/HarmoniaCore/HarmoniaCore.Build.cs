// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaCore: ModuleRules
{
    public HarmoniaCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine"
            });
    }
}
