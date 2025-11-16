// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaKit : ModuleRules
{
    public HarmoniaKit(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "EnhancedInput",
                "UMG",
                "Slate",
                "SlateCore",
                "HarmoniaLoadManager",
                "HarmoniaWorldGenerator",
                "SenseSystem", // Sense-based interaction system
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "LyraGame", // For LyraGameplayAbility base class
            });
    }
}
