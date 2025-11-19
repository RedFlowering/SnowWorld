// Copyright 2025 Snow Game Studio.

using UnrealBuildTool;

public class HarmoniaOnlineSubsystem: ModuleRules
{
    public HarmoniaOnlineSubsystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Sockets",
            "Json",
            "JsonUtilities",
            "HTTP",
            "EOSVoiceChat"
        });

        // Platform-specific Online Subsystems
        if (Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Linux ||
            Target.Platform == UnrealTargetPlatform.Mac)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
        }

        // EOS is cross-platform
        PrivateDependencyModuleNames.Add("OnlineSubsystemEOS");
    }
}
