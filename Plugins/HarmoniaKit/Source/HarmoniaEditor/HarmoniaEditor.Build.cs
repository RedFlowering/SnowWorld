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
                "GameplayTags"
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
