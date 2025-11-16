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
                "GameplayAbilities", // Gameplay Ability System
                "GameplayTags", // Gameplay Tag System
                "GameplayTasks", // Gameplay Task System (used by GAS)
                "LyraGame", // LyraGame module for base ability classes
            });
    }
}
