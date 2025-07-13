// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaSaveGameSystem: ModuleRules
{
    public HarmoniaSaveGameSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
