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
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            // UMG와 Slate는 향후 UI 통합 시 필요할 수 있음
        });
    }
}
