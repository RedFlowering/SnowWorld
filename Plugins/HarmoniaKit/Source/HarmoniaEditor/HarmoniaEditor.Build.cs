// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaEditor Module Build Rules
 * 
 * Main editor module for Harmonia system providing:
 * - World Generator editor tools and visualization
 * - Map capture functionality
 * - Terrain editing utilities
 * - Editor utility widgets for world configuration
 * 
 * This module is editor-only and extends the runtime HarmoniaKit functionality
 * with visual debugging, preview generation, and export capabilities.
 */

using UnrealBuildTool;

public class HarmoniaEditor : ModuleRules
{
    public HarmoniaEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Public dependencies - exposed to other modules
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                // Core modules
                "Core",
                "CoreUObject",
                "Engine",
                
                // Harmonia runtime modules
                "HarmoniaKit",
                "HarmoniaWorldGenerator",   // World generator runtime
                
                // Editor framework
                "UnrealEd",                 // Editor core functionality
                "EditorFramework",
                "EditorSubsystem",
                
                // UI modules
                "Slate",
                "SlateCore",
                "UMG",
                "UMGEditor",
                "Blutility",                // Editor utility widgets
                
                // Asset and content modules
                "AssetTools",
                "ContentBrowser",
                
                // Rendering modules
                "RenderCore",
                "RHI",
                "ImageWrapper",             // Image export functionality
                
                // Editor tools
                "LevelEditor",
                "PropertyEditor",
                "GameplayTags",
                
                // Landscape modules
                "Landscape",                // Landscape editing
                "LandscapeEditor",          // Landscape editor tools
                
                // Serialization
                "Json",                     // JSON serialization
                "JsonUtilities"             // JSON utilities
            });

        // Private dependencies - internal to this module
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "EditorStyle",              // Editor visual styling
                "ToolMenus",                // Menu system
                "InputCore"                 // Input handling
            });
    }
}
