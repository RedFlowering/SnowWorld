// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaCraftingSystem: ModuleRules
{
    public HarmoniaCraftingSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
