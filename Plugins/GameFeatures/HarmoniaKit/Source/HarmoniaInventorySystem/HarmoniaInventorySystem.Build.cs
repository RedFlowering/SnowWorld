// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaInventorySystem: ModuleRules
{
    public HarmoniaInventorySystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
