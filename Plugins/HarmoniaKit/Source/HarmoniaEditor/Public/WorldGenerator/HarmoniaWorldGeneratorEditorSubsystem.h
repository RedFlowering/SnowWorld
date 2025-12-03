// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "WorldGeneratorTypes.h"
#include "HarmoniaWorldGeneratorEditorSubsystem.generated.h"

class UHarmoniaWorldGeneratorSubsystem;
class ALandscape;

/**
 * Editor Subsystem for World Generator Visualization and Utilities
 * Provides editor-only tools for debugging and previewing procedural world generation
 */
UCLASS()
class HARMONIAEDITOR_API UHarmoniaWorldGeneratorEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	// ========================================
	// Editor Visualization & Debug Tools
	// ========================================

	/**
	 * Draw debug visualization for biome map
	 * Shows biome boundaries and types in editor
	 * @param World - World context
	 * @param BiomeData - Biome data to visualize
	 * @param Config - World generation config
	 * @param Duration - How long to display (0 = single frame)
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawBiomeDebugVisualization(
		UWorld* World,
		const TArray<FBiomeData>& BiomeData,
		const FWorldGeneratorConfig& Config,
		float Duration = 10.0f
	);

	/**
	 * Draw debug visualization for roads
	 * @param World - World context
	 * @param RoadSegments - Road segments to visualize
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawRoadDebugVisualization(
		UWorld* World,
		const TArray<FRoadSegmentData>& RoadSegments,
		float Duration = 10.0f
	);

	/**
	 * Draw debug visualization for rivers
	 * @param World - World context
	 * @param RiverSegments - River segments to visualize
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawRiverDebugVisualization(
		UWorld* World,
		const TArray<FRoadSegmentData>& RiverSegments,
		float Duration = 10.0f
	);

	/**
	 * Draw debug visualization for structure placements
	 * @param World - World context
	 * @param Objects - World objects to visualize
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawStructureDebugVisualization(
		UWorld* World,
		const TArray<FWorldObjectData>& Objects,
		float Duration = 10.0f
	);

	/**
	 * Draw debug visualization for cave entrances
	 * @param World - World context
	 * @param CaveEntrances - Cave entrance locations
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawCaveDebugVisualization(
		UWorld* World,
		const TArray<FWorldObjectData>& CaveEntrances,
		float Duration = 10.0f
	);

	/**
	 * Draw debug visualization for resource distribution
	 * @param World - World context
	 * @param OreVeins - Ore vein data
	 * @param ResourceNodes - Resource node locations
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawResourceDebugVisualization(
		UWorld* World,
		const TArray<FOreVeinData>& OreVeins,
		const TArray<FWorldObjectData>& ResourceNodes,
		float Duration = 10.0f
	);

	/**
	 * Draw debug visualization for POIs
	 * @param World - World context
	 * @param POIs - POI locations
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawPOIDebugVisualization(
		UWorld* World,
		const TArray<FWorldObjectData>& POIs,
		float Duration = 10.0f
	);

	/**
	 * Draw heightmap as debug mesh in editor
	 * @param World - World context
	 * @param HeightData - Heightmap data
	 * @param Config - World generation config
	 * @param Duration - How long to display
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void DrawHeightmapDebugVisualization(
		UWorld* World,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config,
		float Duration = 10.0f
	);

	/**
	 * Clear all debug visualizations
	 * @param World - World context
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor|Debug")
	void ClearDebugVisualizations(UWorld* World);

	// ========================================
	// Editor Utility Functions
	// ========================================

	/**
	 * Quick generate and preview world in editor
	 * Automatically creates debug visualizations
	 * @param World - World context
	 * @param Config - World generation config
	 * @param bShowBiomes - Draw biome visualization
	 * @param bShowRoads - Draw road visualization
	 * @param bShowRivers - Draw river visualization
	 * @param bShowStructures - Draw structure visualization
	 * @param bShowResources - Draw resource visualization
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor")
	void EditorQuickPreview(
		UWorld* World,
		const FWorldGeneratorConfig& Config,
		bool bShowBiomes = true,
		bool bShowRoads = true,
		bool bShowRivers = true,
		bool bShowStructures = true,
		bool bShowResources = false
	);

	/**
	 * Export heightmap to PNG file
	 * @param HeightData - Heightmap data
	 * @param Config - World generation config
	 * @param FilePath - Output file path
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor")
	bool ExportHeightmapToPNG(
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config,
		const FString& FilePath
	);

	/**
	 * Export biome map to PNG file
	 * @param BiomeData - Biome data
	 * @param Config - World generation config
	 * @param FilePath - Output file path
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Editor")
	bool ExportBiomeMapToPNG(
		const TArray<FBiomeData>& BiomeData,
		const FWorldGeneratorConfig& Config,
		const FString& FilePath
	);

	/**
	 * Get biome color for visualization
	 * @param BiomeType - Biome type
	 * @return Color for debug drawing
	 */
	UFUNCTION(BlueprintPure, Category = "WorldGenerator|Editor|Colors")
	static FColor GetBiomeDebugColor(EBiomeType BiomeType);

	/**
	 * Get resource type color for visualization
	 * @param ResourceType - Resource type
	 * @return Color for debug drawing
	 */
	UFUNCTION(BlueprintPure, Category = "WorldGenerator|Editor|Colors")
	static FColor GetResourceDebugColor(EResourceType ResourceType);

	/**
	 * Get POI type color for visualization
	 * @param POIType - POI type
	 * @return Color for debug drawing
	 */
	UFUNCTION(BlueprintPure, Category = "WorldGenerator|Editor|Colors")
	static FColor GetPOIDebugColor(EPOIType POIType);

private:
	/**
	 * Get world generator subsystem from game instance
	 */
	UHarmoniaWorldGeneratorSubsystem* GetWorldGeneratorSubsystem(UWorld* World) const;
};
