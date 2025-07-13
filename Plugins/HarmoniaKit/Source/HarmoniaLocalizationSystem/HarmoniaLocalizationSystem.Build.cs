// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaLocalizationSystem: ModuleRules
{
    public HarmoniaLocalizationSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
