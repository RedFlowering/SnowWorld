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
 * - Async generation support for large worlds
 */
UCLASS()
class HARMONIAWORLDGENERATOR_API UHarmoniaWorldGeneratorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Async generation delegates
    UPROPERTY(BlueprintAssignable, Category = "WorldGenerator|Async")
    FOnWorldGenerationProgress OnGenerationProgress;

    UPROPERTY(BlueprintAssignable, Category = "WorldGenerator|Async")
    FOnWorldGenerationComplete OnGenerationComplete;
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

    /**
     * Apply erosion simulation to heightmap
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data (will be modified)
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Erosion")
    void ApplyErosion(
        const FWorldGeneratorConfig& Config,
        TArray<int32>& HeightData
    );

    /**
     * Generate structure groups (villages, ruins, etc.)
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param ActorClassMap  - Actor class mapping for object types
     * @param OutObjects     - Generated grouped structures
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Structures")
    void GenerateStructureGroups(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap,
        TArray<FWorldObjectData>& OutObjects
    );

    /**
     * Generate world asynchronously (non-blocking)
     * Results will be delivered through OnGenerationComplete delegate
     * Progress updates through OnGenerationProgress delegate
     * @param Config         - World generation parameters
     * @param ActorClassMap  - Actor class mapping for object types
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Async")
    void GenerateWorldAsync(
        const FWorldGeneratorConfig& Config,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap
    );

    /**
     * Check if async generation is currently running
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Async")
    bool IsGeneratingAsync() const { return bIsGenerating; }

    /**
     * Cancel ongoing async generation
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Async")
    void CancelAsyncGeneration();

    /**
     * Generate cave system using 3D Perlin noise
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data (for entrance placement)
     * @param OutCaveVolume  - 3D cave volume data
     * @param OutCaveEntrances - Cave entrance locations
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Caves")
    void GenerateCaveSystem(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        TArray<FCaveVolumeData>& OutCaveVolume,
        TArray<FWorldObjectData>& OutCaveEntrances
    );

    /**
     * Generate POIs (Points of Interest)
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param BiomeData      - Biome data
     * @param OutPOIs        - Generated POI locations
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|POI")
    void GeneratePOIs(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        const TArray<FBiomeData>& BiomeData,
        TArray<FWorldObjectData>& OutPOIs
    );

    /**
     * Generate resource distribution (ore veins)
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param BiomeData      - Biome data
     * @param OutOreVeins    - Generated ore vein data
     * @param OutResourceNodes - Individual resource node objects
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Resources")
    void GenerateResourceDistribution(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        const TArray<FBiomeData>& BiomeData,
        TArray<FOreVeinData>& OutOreVeins,
        TArray<FWorldObjectData>& OutResourceNodes
    );

    /**
     * Generate splatmap for Landscape texture painting
     * @param Config         - World generation parameters
     * @param HeightData     - Heightmap data
     * @param BiomeData      - Biome data
     * @param OutSplatmapLayers - Generated splatmap layer data
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Splatmap")
    void GenerateSplatmap(
        const FWorldGeneratorConfig& Config,
        const TArray<int32>& HeightData,
        const TArray<FBiomeData>& BiomeData,
        TArray<FSplatmapLayerData>& OutSplatmapLayers
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

    /**
     * Simulate single water droplet for erosion
     */
    void SimulateDroplet(
        TArray<float>& HeightMap,
        const FWorldGeneratorConfig& Config,
        FRandomStream& Random
    );

    /**
     * Calculate height and gradient at position
     */
    float CalculateHeightAndGradient(
        const TArray<float>& HeightMap,
        float PosX,
        float PosY,
        int32 MapSize,
        FVector2D& OutGradient
    );

    /**
     * Calculate terrain flatness at location
     * @return Flatness value (0-1, 1 = perfectly flat)
     */
    float CalculateFlatness(
        int32 X,
        int32 Y,
        int32 Radius,
        const TArray<int32>& HeightData,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Check if location is flat enough for structures
     */
    bool IsFlatEnoughForStructure(
        int32 X,
        int32 Y,
        const TArray<int32>& HeightData,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Async generation worker function
     */
    void AsyncGenerationWorker(
        FWorldGeneratorConfig Config,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap
    );

    /**
     * Update progress on game thread
     */
    void UpdateProgress(float Progress);

    /**
     * Complete async generation on game thread
     */
    void CompleteAsyncGeneration(
        TArray<int32> HeightData,
        TArray<FWorldObjectData> Objects,
        bool bSuccess
    );

    /**
     * Calculate 3D cave noise value at position
     */
    float Calculate3DCaveNoise(
        float X,
        float Y,
        float Z,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Pick random POI type based on probability map
     */
    EPOIType PickPOIType(
        const TMap<EPOIType, float>& ProbMap,
        FRandomStream& Random
    );

    /**
     * Pick random resource type based on probability and biome
     */
    EResourceType PickResourceType(
        const TMap<EResourceType, float>& ProbMap,
        EBiomeType BiomeType,
        float Height,
        const FWorldGeneratorConfig& Config,
        FRandomStream& Random
    );

    /**
     * Calculate slope at location (in degrees)
     */
    float CalculateSlope(
        int32 X,
        int32 Y,
        const TArray<int32>& HeightData,
        const FWorldGeneratorConfig& Config
    );

    /**
     * Get splatmap layer weight for location
     */
    uint8 CalculateLayerWeight(
        int32 X,
        int32 Y,
        FName LayerName,
        const TArray<int32>& HeightData,
        const TArray<FBiomeData>& BiomeData,
        const FWorldGeneratorConfig& Config
    );

    // Async generation state
    std::atomic<bool> bIsGenerating{false};
    std::atomic<bool> bCancelRequested{false};
    std::atomic<float> CurrentProgress{0.0f};
};
