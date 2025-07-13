// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaInteractionSystem: ModuleRules
{
    public HarmoniaInteractionSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
