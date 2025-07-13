// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaNPCSystem: ModuleRules
{
    public HarmoniaNPCSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
