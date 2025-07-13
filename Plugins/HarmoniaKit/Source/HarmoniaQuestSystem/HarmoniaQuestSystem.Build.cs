// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaQuestSystem: ModuleRules
{
    public HarmoniaQuestSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
