// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaSettingsSystem: ModuleRules
{
    public HarmoniaSettingsSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
