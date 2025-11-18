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
                "ALS", // Advanced Locomotion System (required by LyraCharacter)
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "CommonUI", // For UCommonActivatableWidget
                "NavigationSystem", // For waypoint pathfinding
                "AIModule", // For AI navigation
                "RenderCore", // For GWhiteTexture in FogOfWarRenderer (client-side rendering)
                "Niagara" // For Crystal Resonator VFX
                // Note: RHI, ImageWrapper are in HarmoniaEditor module (editor-only texture capture)
            });
    }
}
