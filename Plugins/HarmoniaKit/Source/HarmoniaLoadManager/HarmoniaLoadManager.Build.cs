// Copyright 2025 Snow Game Studio.

using System.IO;
using UnrealBuildTool;

public class HarmoniaLoadManager : ModuleRules
{
    public HarmoniaLoadManager(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModuleDirectory, "Private/Macro")
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				// ... add other public dependencies that you statically link with here ...
			}
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Projects",
                "DeveloperSettings",
                "GameplayTags",
				// ... add private dependencies that you statically link with here ...	
			}
            );
    }
}