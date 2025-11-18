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

    /**
     * Generate biome map for the world
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param OutBiomeData   - Generated biome data for each tile
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Biomes")
    void GenerateBiomeMap(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        TArray<FBiomeData>& OutBiomeData
    );

    /**
     * Generate rivers
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param BiomeData      - Biome data
     * @param OutRiverSegments - Generated river segments
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Water")
    void GenerateRivers(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        const TArray<FBiomeData>& BiomeData,
        TArray<FRoadSegmentData>& OutRiverSegments
    );

    /**
     * Generate lakes
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data (will be modified for lake basins)
     * @param BiomeData      - Biome data
     * @param OutLakeLocations - Generated lake center locations and radii
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Water")
    void GenerateLakes(
        const FWorldGeneratorConfig& Config,
        TArray<int32>& HeightData,
        const TArray<FBiomeData>& BiomeData,
        TArray<FVector>& OutLakeLocations
    );

    /**
     * Generate roads between points of interest
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param OutRoadSegments - Generated road segments
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Roads")
    void GenerateRoads(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        TArray<FRoadSegmentData>& OutRoadSegments
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

    /**
     * Calculate temperature at location using noise
     */
    float CalculateTemperature(
        float X,
        float Y,
        float Height,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Calculate moisture at location using noise
     */
    float CalculateMoisture(
        float X,
        float Y,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Determine biome type from temperature, moisture, and height
     */
    EBiomeType DetermineBiomeType(
        float Temperature,
        float Moisture,
        float Height,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Find best matching biome settings
     */
    const FBiomeSettings* FindBiomeSettings(
        EBiomeType BiomeType,
        const FWorldGeneratorConfig& Config
    ) const;

    /**
     * Trace river from source to sea
     */
    void TraceRiver(
        int32 StartX,
        int32 StartY,
        const TArray<int32>& HeightData,
        const FWorldGeneratorConfig& Config,
        TArray<FVector>& OutRiverPath
    );

    /**
     * Find path between two points (A* pathfinding for roads)
     */
    void FindPath(
        FVector Start,
        FVector End,
        const TArray<int32>& HeightData,
        const FWorldGeneratorConfig& Config,
        TArray<FVector>& OutPath
    );
};
