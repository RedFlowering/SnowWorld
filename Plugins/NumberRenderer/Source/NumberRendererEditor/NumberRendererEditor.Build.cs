// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

using UnrealBuildTool;

public class NumberRendererEditor : ModuleRules
{
    public NumberRendererEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
			}
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "NumberRenderer/Public",
                "NumberRendererEditor/Private",
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
                "NumberRenderer"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Json",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "PropertyEditor",
                "EditorStyle"
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
