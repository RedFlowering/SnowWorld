// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaModSystem: ModuleRules
{
    public HarmoniaModSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
