// Copyright 2024 HGsofts, All Rights Reserved.

using UnrealBuildTool;

public class CosmeticSystem : ModuleRules
{
	public CosmeticSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {				
			}
			);


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "GameplayAbilities",
                "GameplayTags",
                "ModularGameplay",
                "HairStrandsCore",
                "Niagara",
                "SkeletalMerging"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "DeveloperSettings",
                "AIModule"
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
			);
	}
}
