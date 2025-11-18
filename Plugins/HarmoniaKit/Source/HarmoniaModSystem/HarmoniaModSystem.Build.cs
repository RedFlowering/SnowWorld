// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaModSystem: ModuleRules
{
    public HarmoniaModSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "Json",
            "JsonUtilities"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "HarmoniaLoadManager",
            "DeveloperSettings"
        });
    }
}
