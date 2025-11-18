// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaEditor : ModuleRules
{
    public HarmoniaEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "HarmoniaKit",
                "HarmoniaWorldGenerator", // World generator module
                "UnrealEd", // Editor core
                "EditorFramework",
                "EditorSubsystem",
                "Slate",
                "SlateCore",
                "UMG",
                "UMGEditor",
                "Blutility", // Editor utility widgets
                "AssetTools",
                "ContentBrowser",
                "RenderCore",
                "RHI",
                "ImageWrapper",
                "LevelEditor",
                "PropertyEditor",
                "GameplayTags",
                "Landscape", // Landscape editing
                "LandscapeEditor", // Landscape editor tools
                "Json", // JSON serialization for config save/load
                "JsonUtilities" // JSON utilities
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "EditorStyle",
                "ToolMenus",
                "InputCore"
            });
    }
}
