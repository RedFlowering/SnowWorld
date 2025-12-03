// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaLoadManagerEditor Module Build Rules
 * 
 * Editor-only module that extends HarmoniaLoadManager with editor functionality.
 * Provides automatic macro regeneration when Registry assets are saved.
 * 
 * Dependencies:
 * - Core: Unreal Engine core functionality
 * - CoreUObject: UObject system for asset handling
 * - Engine: Engine runtime
 * - HarmoniaLoadManager: Runtime load manager module
 */

using UnrealBuildTool;

public class HarmoniaLoadManagerEditor : ModuleRules
{
    public HarmoniaLoadManagerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        // Use explicit or shared PCH for faster compilation
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",              // Core functionality
                "CoreUObject",       // UObject and asset system
                "Engine",            // Engine runtime
                "HarmoniaLoadManager", // Runtime load manager
            });
    }
}
