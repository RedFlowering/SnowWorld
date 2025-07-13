// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaActivitySystem: ModuleRules
{
    public HarmoniaActivitySystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
