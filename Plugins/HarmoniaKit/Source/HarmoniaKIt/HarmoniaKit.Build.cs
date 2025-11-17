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
                "CoreOnline",
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
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "CommonUI", // For UCommonActivatableWidget
                "RenderCore", // For render target and texture operations
                "RHI", // For reading render target pixels
                "ImageWrapper" // For image processing
            });
    }
}
