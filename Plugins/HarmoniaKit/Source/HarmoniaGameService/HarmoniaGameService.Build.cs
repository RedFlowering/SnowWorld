// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaGameService: ModuleRules
{
    public HarmoniaGameService(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Json",
            "JsonUtilities",
            "HTTP",
            "Sockets",
            "Networking"
        });

        // Platform-specific online subsystems
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
            PrivateDependencyModuleNames.Add("OnlineSubsystemEOS");
        }
        else if (Target.Platform == UnrealTargetPlatform.XboxOne ||
                 Target.Platform == UnrealTargetPlatform.XSX)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemGDK");
        }
        else if (Target.Platform == UnrealTargetPlatform.PS4 ||
                 Target.Platform == UnrealTargetPlatform.PS5)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemPS4");
        }
    }
}
