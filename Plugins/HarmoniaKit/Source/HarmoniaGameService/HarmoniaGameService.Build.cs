// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaGameService Module Build Rules
 * 
 * Provides unified platform service integration for:
 * - Achievements (unlock, progress tracking, queries)
 * - Cloud Saves (upload, download, sync, conflict resolution)
 * - Leaderboards (score upload, queries, friend rankings)
 * - Player Statistics (playtime, kills, deaths, custom stats)
 * - DLC/Content Ownership (entitlement checks)
 * - Cross-Play Sessions (create, join, find multiplayer sessions)
 * 
 * Supported Platforms:
 * - Steam (via OnlineSubsystemSteam)
 * - Epic Games Store (via OnlineSubsystemEOS)
 * - Xbox (via OnlineSubsystemGDK)
 * - PlayStation (via OnlineSubsystemPS5)
 * - Nintendo Switch (via OnlineSubsystemSwitch)
 * - GOG (via OnlineSubsystemGOG)
 */

using UnrealBuildTool;

public class HarmoniaGameService : ModuleRules
{
    public HarmoniaGameService(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public dependencies - exposed to other modules
        PublicDependencyModuleNames.AddRange(new string[]
        {
            // Core modules
            "Core",
            "CoreUObject",
            "Engine",
            
            // Gameplay systems
            "GameplayTags",              // For DLC unlock tags
            
            // Online subsystem (platform abstraction)
            "OnlineSubsystem",           // Base online interface
            "OnlineSubsystemUtils"       // Online utilities
        });

        // Private dependencies - internal to this module
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // Serialization
            "Json",                      // JSON parsing
            "JsonUtilities",             // JSON helpers
            
            // Networking
            "HTTP",                      // HTTP requests for REST APIs
            "Sockets",                   // Socket communication
            "Networking"                 // Network utilities
        });

        // Platform-specific online subsystems
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Steam and Epic Games Store support on Windows
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
            PrivateDependencyModuleNames.Add("OnlineSubsystemEOS");
        }
        // Note: Console platform subsystems (GDK, PS5, Switch, etc.) are added automatically
        // by the respective platform SDKs when building for those platforms
    }
}
