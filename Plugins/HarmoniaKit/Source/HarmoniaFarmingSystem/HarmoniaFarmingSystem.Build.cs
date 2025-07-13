// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaFarmingSystem: ModuleRules
{
    public HarmoniaFarmingSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
