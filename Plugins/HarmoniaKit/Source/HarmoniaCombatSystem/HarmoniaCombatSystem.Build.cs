// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaCombatSystem: ModuleRules
{
    public HarmoniaCombatSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
