// Copyright 2017 Lee Ju Sik

using UnrealBuildTool;
using System.IO;

public class RootMotionGuidePluginEditor : ModuleRules
{
	public RootMotionGuidePluginEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				//"RootMotionGuidePluginEditor/Public"
                //Path.Combine(ModuleDirectory, "Public")
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				//"RootMotionGuidePluginEditor/Private",
                //Path.Combine(ModuleDirectory, "Private")
				// ... add other private include paths required here ...
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
				"Projects",
				"InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                // ... add private dependencies that you statically link with here ...	
                "EditorStyle",
                "Persona",
                "AnimationEditor",
                "AnimationCore",
				"AnimationDataController",
				"RootMotionGuidePlugin"
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
