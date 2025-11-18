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

    // Environment system delegates
    UPROPERTY(BlueprintAssignable, Category = "WorldGenerator|Environment")
    FOnSeasonChanged OnSeasonChanged;

    UPROPERTY(BlueprintAssignable, Category = "WorldGenerator|Environment")
    FOnWeatherChanged OnWeatherChanged;

    UPROPERTY(BlueprintAssignable, Category = "WorldGenerator|Environment")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "WorldGenerator|Environment")
    FOnDayNightCycleTick OnDayNightCycleTick;
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

    // ========================================
    // Environment System (Seasons, Weather, Day/Night)
    // ========================================

    /**
     * Initialize environment system with settings
     * Call this before starting the environment system
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void InitializeEnvironmentSystem(const FEnvironmentSystemSettings& Settings);

    /**
     * Start environment system (seasons, weather, day/night cycle)
     * Requires InitializeEnvironmentSystem() to be called first
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void StartEnvironmentSystem();

    /**
     * Stop environment system
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void StopEnvironmentSystem();

    /**
     * Check if environment system is running
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    bool IsEnvironmentSystemRunning() const { return bEnvironmentSystemActive; }

    /**
     * Manually set current season
     * @param NewSeason - Season to set
     * @param bBroadcast - Whether to broadcast OnSeasonChanged delegate
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void SetCurrentSeason(ESeasonType NewSeason, bool bBroadcast = true);

    /**
     * Get current season
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    ESeasonType GetCurrentSeason() const { return CurrentSeason; }

    /**
     * Get current season progress (0-1)
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    float GetSeasonProgress() const { return SeasonProgress; }

    /**
     * Manually change weather
     * @param NewWeather - Weather type to change to
     * @param TransitionDuration - Duration of weather transition in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void ChangeWeather(EWeatherType NewWeather, float TransitionDuration = 30.0f);

    /**
     * Get current weather
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    EWeatherType GetCurrentWeather() const { return CurrentWeather; }

    /**
     * Get current time of day
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    ETimeOfDay GetCurrentTimeOfDay() const;

    /**
     * Get current game time in hours (0-24)
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    float GetCurrentGameTime() const { return CurrentGameTime; }

    /**
     * Set current game time in hours (0-24)
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void SetCurrentGameTime(float Hours);

    /**
     * Get sun angle in degrees (0-360)
     * 0 = midnight, 90 = sunrise, 180 = noon, 270 = sunset
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    float GetSunAngle() const;

    /**
     * Get current season visual settings
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    FSeasonVisuals GetCurrentSeasonVisuals() const;

    /**
     * Set time progression speed multiplier
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Environment")
    void SetTimeSpeed(float SpeedMultiplier);

    // ========================================
    // Runtime Terrain Modification
    // ========================================

    /**
     * Apply terrain modification at runtime
     * @param Landscape - Target landscape actor
     * @param Modification - Modification parameters
     * @return Result of the modification
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult ApplyTerrainModification(
        ALandscape* Landscape,
        const FTerrainModification& Modification
    );

    /**
     * Create a crater at specified location
     * Useful for explosions, meteor impacts, etc.
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult CreateCrater(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        float Depth,
        ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth
    );

    /**
     * Flatten terrain in a circular area
     * Useful for building foundations
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult FlattenTerrain(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        float TargetHeight,
        ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth
    );

    /**
     * Create a hill/mound at specified location
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult CreateHill(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        float Height,
        ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth
    );

    /**
     * Smooth terrain in a circular area
     * Reduces sharp edges and roughness
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult SmoothTerrain(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        int32 Iterations = 3
    );

    /**
     * Raise terrain in a circular area
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult RaiseTerrain(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        float Amount,
        ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth
    );

    /**
     * Lower terrain in a circular area
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult LowerTerrain(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        float Amount,
        ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth
    );

    /**
     * Paint landscape layer at location
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|RuntimeModification")
    FTerrainModificationResult PaintLandscapeLayer(
        ALandscape* Landscape,
        FVector Location,
        float Radius,
        FName LayerName,
        float Strength = 1.0f,
        ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth
    );

    // ========================================
    // Chunk Caching System
    // ========================================

    /**
     * Initialize chunk cache with settings
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    void InitializeChunkCache(const FChunkCacheSettings& Settings);

    /**
     * Get cached chunk data
     * @param ChunkCoordinates - Chunk coordinates to retrieve
     * @param OutChunkData - Retrieved chunk data
     * @return True if chunk was found in cache
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    bool GetCachedChunk(FIntPoint ChunkCoordinates, FWorldChunkData& OutChunkData);

    /**
     * Cache a chunk for future reuse
     * @param ChunkData - Chunk data to cache
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    void CacheChunk(const FWorldChunkData& ChunkData);

    /**
     * Clear all cached chunks from memory
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    void ClearChunkCache();

    /**
     * Save chunk cache to disk
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    bool SaveChunkCacheToDisk();

    /**
     * Load chunk cache from disk
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    bool LoadChunkCacheFromDisk();

    /**
     * Get cache statistics
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    void GetCacheStatistics(int32& OutCachedChunks, int32& OutMemoryUsageKB, int32& OutDiskCacheSize);

    /**
     * Remove expired chunks from cache
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    void CleanExpiredChunks();

    /**
     * Check if chunk exists in cache
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    bool IsChunkCached(FIntPoint ChunkCoordinates) const;

    /**
     * Preload chunks in a region
     * Useful for streaming worlds
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator|Cache")
    void PreloadChunksInRegion(FIntPoint MinChunk, FIntPoint MaxChunk);

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

    // ========================================
    // Environment System Private Methods
    // ========================================

    /**
     * Tick environment system (called by USubsystem::Tick)
     */
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    /**
     * Update day/night cycle
     */
    void UpdateDayNightCycle(float DeltaTime);

    /**
     * Update season progression
     */
    void UpdateSeasonProgression(float DeltaTime);

    /**
     * Update weather system
     */
    void UpdateWeather(float DeltaTime);

    /**
     * Select random weather based on season and biome
     */
    EWeatherType SelectRandomWeather(ESeasonType Season, EBiomeType Biome);

    /**
     * Calculate sun angle from game time
     */
    float CalculateSunAngleFromTime(float GameTime) const;

    /**
     * Get season settings for a specific season
     */
    const FSeasonSettings* GetSeasonSettings(ESeasonType Season) const;

    /**
     * Interpolate season visuals between two seasons
     */
    FSeasonVisuals InterpolateSeasonVisuals(
        const FSeasonVisuals& From,
        const FSeasonVisuals& To,
        float Alpha
    ) const;

    // ========================================
    // Runtime Terrain Modification Helpers
    // ========================================

    /**
     * Calculate falloff weight based on distance and falloff type
     */
    float CalculateFalloff(
        float Distance,
        float Radius,
        ETerrainFalloffType FalloffType
    ) const;

    /**
     * Get landscape heightmap data in region
     */
    bool GetLandscapeHeightData(
        ALandscape* Landscape,
        FVector Center,
        float Radius,
        TArray<uint16>& OutHeightData,
        int32& OutMinX,
        int32& OutMinY,
        int32& OutMaxX,
        int32& OutMaxY
    );

    /**
     * Set landscape heightmap data in region
     */
    bool SetLandscapeHeightData(
        ALandscape* Landscape,
        const TArray<uint16>& HeightData,
        int32 MinX,
        int32 MinY,
        int32 MaxX,
        int32 MaxY
    );

    /**
     * Convert world location to landscape coordinates
     */
    FVector2D WorldToLandscapeCoordinates(
        ALandscape* Landscape,
        FVector WorldLocation
    ) const;

    // ========================================
    // Chunk Cache Helpers
    // ========================================

    /**
     * Calculate hash for chunk validation
     */
    int32 CalculateChunkHash(const FWorldChunkData& ChunkData) const;

    /**
     * Get chunk file path for disk cache
     */
    FString GetChunkCacheFilePath(FIntPoint ChunkCoordinates) const;

    /**
     * Evict least recently used chunks if cache is full
     */
    void EvictLRUChunks();

    /**
     * Calculate memory usage of cached chunks
     */
    int32 CalculateCacheMemoryUsage() const;

    // Async generation state
    std::atomic<bool> bIsGenerating{false};
    std::atomic<bool> bCancelRequested{false};
    std::atomic<float> CurrentProgress{0.0f};

    // Environment system state
    bool bEnvironmentSystemActive = false;
    FEnvironmentSystemSettings EnvironmentSettings;

    ESeasonType CurrentSeason = ESeasonType::Spring;
    float SeasonProgress = 0.0f; // 0-1 within current season
    float TotalSeasonTime = 0.0f; // Total time elapsed in current season

    EWeatherType CurrentWeather = EWeatherType::Clear;
    EWeatherType PreviousWeather = EWeatherType::Clear;
    float WeatherTransitionProgress = 1.0f; // 1.0 = fully transitioned
    float TimeSinceLastWeatherChange = 0.0f;

    float CurrentGameTime = 12.0f; // 0-24 hours
    float TimeSpeedMultiplier = 1.0f;

    FRandomStream WeatherRandom;

    // Chunk cache state
    FChunkCacheSettings CacheSettings;
    TMap<FIntPoint, FWorldChunkData> ChunkCache;
    TArray<FIntPoint> ChunkAccessOrder; // For LRU eviction
};
