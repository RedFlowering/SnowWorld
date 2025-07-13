// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaSkillSystem: ModuleRules
{
    public HarmoniaSkillSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] {"Core", "CoreUObject", "Engine"});
    }
}
