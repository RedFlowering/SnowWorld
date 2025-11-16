// Copyright 2025 Snow Game Studio.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "WorldGeneratorTypes.h"
#include "HarmoniaWorldGeneratorSubsystem.generated.h"

/**
 * Harmonia World Generation Subsystem
 * - Generates terrain heightmap and world objects
 * - Seed-based deterministic generation for multiplayer sync
 * - Chunk-based processing to prevent editor freezing
 */
UCLASS()
class HARMONIAWORLDGENERATOR_API UHarmoniaWorldGeneratorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /**
     * Generate complete world (heightmap + objects)
     * @param Config         - World generation parameters
     * @param OutHeightData  - Generated heightmap data (for Landscape)
     * @param OutObjects     - Generated world objects
     * @param ActorClassMap  - Actor class mapping for object types
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator")
    void GenerateWorld(
        const FWorldGeneratorConfig& Config,
        TArray<int32>& OutHeightData,
        TArray<FWorldObjectData>& OutObjects,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap
    );

    /**
     * Generate heightmap only (no objects)
     * More efficient for large terrains
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator")
    void GenerateHeightmapOnly(
        const FWorldGeneratorConfig& Config,
        TArray<int32>& OutHeightData
    );

    /**
     * Generate objects for a specific region
     * Can be called on-demand for streaming worlds
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator")
    void GenerateObjectsInRegion(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap,
        int32 RegionMinX,
        int32 RegionMinY,
        int32 RegionMaxX,
        int32 RegionMaxY,
        TArray<FWorldObjectData>& OutObjects
    );

private:
    /**
     * Generate heightmap data with chunk-based processing
     */
    void GenerateHeightmap(
        const FWorldGeneratorConfig& Config,
        TArray<int32>& OutHeightData
    );

    /**
     * Generate world objects with chunk-based processing
     */
    void GenerateObjects(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap,
        TArray<FWorldObjectData>& OutObjects
    );

    /**
     * Pick random object type based on probability map
     */
    EWorldObjectType PickObjectType(
        const TMap<EWorldObjectType, float>& ProbMap,
        FRandomStream& Random
    );

    /**
     * Calculate height at specific location
     */
    float CalculateHeightAtLocation(
        float X,
        float Y,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Check if location is valid for object placement
     */
    bool IsValidObjectLocation(
        int32 X,
        int32 Y,
        const TArray<int32>& HeightData,
        const FWorldGeneratorConfig& Config
    );
};
