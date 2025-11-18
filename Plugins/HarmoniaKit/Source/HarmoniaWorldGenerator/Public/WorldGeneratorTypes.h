// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldGeneratorTypes.generated.h"

UENUM(BlueprintType)
enum class EWorldObjectType : uint8
{
	None        UMETA(DisplayName = "None"),
	Tree        UMETA(DisplayName = "Tree"),
	Rock        UMETA(DisplayName = "Rock"),
	Resource    UMETA(DisplayName = "Resource"),
	Structure   UMETA(DisplayName = "Structure"),
};

/**
 * Biome Types for World Generation
 */
UENUM(BlueprintType)
enum class EBiomeType : uint8
{
	None         UMETA(DisplayName = "None"),
	Ocean        UMETA(DisplayName = "Ocean"),
	Beach        UMETA(DisplayName = "Beach"),
	Desert       UMETA(DisplayName = "Desert"),
	Grassland    UMETA(DisplayName = "Grassland"),
	Forest       UMETA(DisplayName = "Forest"),
	Rainforest   UMETA(DisplayName = "Rainforest"),
	Savanna      UMETA(DisplayName = "Savanna"),
	Taiga        UMETA(DisplayName = "Taiga"),
	Tundra       UMETA(DisplayName = "Tundra"),
	Snow         UMETA(DisplayName = "Snow"),
	Mountain     UMETA(DisplayName = "Mountain"),
	Swamp        UMETA(DisplayName = "Swamp"),
};

USTRUCT(BlueprintType)
struct FWorldObjectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> ActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWorldObjectType ObjectType = EWorldObjectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	// Group ID (for grouped structures like villages)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GroupID = -1;

	// Is this a group center/leader object?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGroupCenter = false;
};

/**
 * Structure Group Settings (Villages, Ruins, etc.)
 */
USTRUCT(BlueprintType)
struct FStructureGroupSettings
{
	GENERATED_BODY()

	// Enable structure group generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup")
	bool bEnableStructureGroups = true;

	// Number of structure groups to generate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup", meta = (ClampMin = "0", ClampMax = "100"))
	int32 GroupCount = 5;

	// Minimum structures per group
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MinStructuresPerGroup = 3;

	// Maximum structures per group
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup", meta = (ClampMin = "1", ClampMax = "50"))
	int32 MaxStructuresPerGroup = 10;

	// Group radius (in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float GroupRadius = 2000.0f;

	// Minimum flatness required (0-1, 1 = perfectly flat)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinFlatness = 0.8f;

	// Structure spacing within group
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StructureGroup", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float StructureSpacing = 500.0f;
};

USTRUCT(BlueprintType)
struct FPerlinNoiseSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Octaves = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Persistence = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Lacunarity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Frequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Amplitude = 1.0f;
};

/**
 * Biome Configuration Settings
 */
USTRUCT(BlueprintType)
struct FBiomeSettings
{
	GENERATED_BODY()

	// Biome type identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	EBiomeType BiomeType = EBiomeType::Grassland;

	// Display color for biome visualization (debug/minimap)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
	FLinearColor BiomeColor = FLinearColor::Green;

	// Ground material for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
	TSoftObjectPtr<UMaterialInterface> GroundMaterial = nullptr;

	// Temperature range (0.0 = cold, 1.0 = hot)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinTemperature = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxTemperature = 0.6f;

	// Moisture range (0.0 = dry, 1.0 = wet)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinMoisture = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxMoisture = 0.6f;

	// Height range for this biome (normalized 0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Terrain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxHeight = 1.0f;

	// Object density multiplier for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Objects")
	float ObjectDensityMultiplier = 1.0f;

	// Object type spawn probabilities specific to this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Objects")
	TMap<EWorldObjectType, float> BiomeObjectProbabilities;
};

/**
 * Water Body Settings (Rivers, Lakes, Ocean)
 */
USTRUCT(BlueprintType)
struct FWaterBodySettings
{
	GENERATED_BODY()

	// Enable water generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
	bool bEnableWater = true;

	// Water surface color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Visual")
	FLinearColor WaterColor = FLinearColor(0.0f, 0.4f, 0.8f, 0.7f);

	// Water material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Visual")
	TSoftObjectPtr<UMaterialInterface> WaterMaterial = nullptr;

	// Deep water color (for ocean/deep lakes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Visual")
	FLinearColor DeepWaterColor = FLinearColor(0.0f, 0.2f, 0.5f, 0.9f);

	// Water depth (in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Terrain")
	float WaterDepth = 100.0f;

	// Maximum depth for deep water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Terrain")
	float MaxWaterDepth = 500.0f;
};

/**
 * River Generation Settings
 */
USTRUCT(BlueprintType)
struct FRiverSettings
{
	GENERATED_BODY()

	// Enable river generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	bool bEnableRivers = true;

	// Number of rivers to generate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	int32 RiverCount = 5;

	// Minimum river length (in tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	int32 MinRiverLength = 50;

	// Maximum river length (in tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
	int32 MaxRiverLength = 200;

	// River width (in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Visual")
	float RiverWidth = 300.0f;

	// River color override (if different from water)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Visual")
	FLinearColor RiverColor = FLinearColor(0.2f, 0.5f, 0.9f, 0.8f);

	// River material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Visual")
	TSoftObjectPtr<UMaterialInterface> RiverMaterial = nullptr;

	// Spline mesh for river (optional, for better visuals)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Visual")
	TSoftObjectPtr<UStaticMesh> RiverSplineMesh = nullptr;

	// Minimum height for river sources
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
	float MinSourceHeight = 0.6f;

	// Rivers flow to sea level or below
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River|Generation")
	bool bFlowToSea = true;
};

/**
 * Lake Generation Settings
 */
USTRUCT(BlueprintType)
struct FLakeSettings
{
	GENERATED_BODY()

	// Enable lake generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
	bool bEnableLakes = true;

	// Number of lakes to generate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
	int32 LakeCount = 10;

	// Minimum lake radius (in tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
	int32 MinLakeRadius = 5;

	// Maximum lake radius (in tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
	int32 MaxLakeRadius = 20;

	// Lake color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Visual")
	FLinearColor LakeColor = FLinearColor(0.1f, 0.4f, 0.7f, 0.8f);

	// Lake material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Visual")
	TSoftObjectPtr<UMaterialInterface> LakeMaterial = nullptr;

	// Preferred height range for lakes (normalized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PreferredMinHeight = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake|Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PreferredMaxHeight = 0.7f;
};

/**
 * Road/Path Generation Settings
 */
USTRUCT(BlueprintType)
struct FRoadSettings
{
	GENERATED_BODY()

	// Enable road generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	bool bEnableRoads = true;

	// Road network density (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RoadDensity = 0.1f;

	// Road mesh for rendering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual")
	TSoftObjectPtr<UStaticMesh> RoadMesh = nullptr;

	// Road spline mesh for curved roads
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual")
	TSoftObjectPtr<UStaticMesh> RoadSplineMesh = nullptr;

	// Road material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual")
	TSoftObjectPtr<UMaterialInterface> RoadMaterial = nullptr;

	// Road width (in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Dimensions")
	float RoadWidth = 400.0f;

	// Maximum road slope (in degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Generation")
	float MaxSlope = 30.0f;

	// Minimum distance between road nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Generation")
	float MinNodeDistance = 500.0f;

	// Points of interest to connect with roads
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Generation")
	TArray<FVector> ConnectionPoints;

	// Bridge mesh for crossing water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road|Visual")
	TSoftObjectPtr<UStaticMesh> BridgeMesh = nullptr;
};

/**
 * Road/Path Data for spawning
 */
USTRUCT(BlueprintType)
struct FRoadSegmentData
{
	GENERATED_BODY()

	// Road segment points (spline points)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> SplinePoints;

	// Mesh to use for this segment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> SegmentMesh = nullptr;

	// Is this segment a bridge?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBridge = false;

	// Segment width
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 400.0f;
};

/**
 * Biome Data with location information
 */
USTRUCT(BlueprintType)
struct FBiomeData
{
	GENERATED_BODY()

	// Grid position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y = 0;

	// Biome type at this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBiomeType BiomeType = EBiomeType::None;

	// Temperature value (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 0.5f;

	// Moisture value (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Moisture = 0.5f;

	// Height value (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height = 0.5f;
};

/**
 * Erosion Simulation Settings
 */
USTRUCT(BlueprintType)
struct FErosionSettings
{
	GENERATED_BODY()

	// Enable erosion simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
	bool bEnableErosion = true;

	// Number of erosion iterations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0", ClampMax = "10000"))
	int32 ErosionIterations = 100000;

	// Erosion radius (how far sediment spreads)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "10"))
	int32 ErosionRadius = 3;

	// Inertia (0-1, higher = straighter paths)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Inertia = 0.05f;

	// Sediment capacity factor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float SedimentCapacityFactor = 4.0f;

	// Minimum sediment capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSedimentCapacity = 0.01f;

	// Erode speed (how fast terrain erodes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ErodeSpeed = 0.3f;

	// Deposit speed (how fast sediment deposits)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DepositSpeed = 0.3f;

	// Evaporate speed (water loss per step)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EvaporateSpeed = 0.01f;

	// Gravity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float Gravity = 4.0f;

	// Max droplet lifetime (steps)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "1", ClampMax = "200"))
	int32 MaxDropletLifetime = 30;

	// Initial water volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float InitialWaterVolume = 1.0f;

	// Initial speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float InitialSpeed = 1.0f;
};

/**
 * World Generation Configuration
 * Seed-based generation ensures same world in multiplayer
 */
USTRUCT(BlueprintType)
struct FWorldGeneratorConfig
{
	GENERATED_BODY()

	// Seed for deterministic world generation (multiplayer sync)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	int32 Seed = 0;

	// World size (recommend power of 2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	int32 SizeX = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	int32 SizeY = 512;

	// Maximum terrain height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	float MaxHeight = 1000.f;

	// Object spawn density (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	float ObjectDensity = 0.008f;

	// Sea level threshold (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	float SeaLevel = 0.42f;

	// Object type spawn probabilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	TMap<EWorldObjectType, float> ObjectTypeProbabilities;

	// Noise generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	FPerlinNoiseSettings NoiseSettings;

	// ===== Biome System =====

	// Enable biome generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Biomes")
	bool bEnableBiomes = true;

	// Biome configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Biomes")
	TArray<FBiomeSettings> BiomeSettings;

	// Temperature noise settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Biomes")
	FPerlinNoiseSettings TemperatureNoiseSettings;

	// Moisture noise settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Biomes")
	FPerlinNoiseSettings MoistureNoiseSettings;

	// Biome blend distance (in tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Biomes")
	float BiomeBlendDistance = 5.0f;

	// ===== Water System =====

	// General water settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Water")
	FWaterBodySettings WaterSettings;

	// River generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Water")
	FRiverSettings RiverSettings;

	// Lake generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Water")
	FLakeSettings LakeSettings;

	// ===== Road System =====

	// Road generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Roads")
	FRoadSettings RoadSettings;

	// ===== Erosion System =====

	// Erosion simulation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Erosion")
	FErosionSettings ErosionSettings;

	// ===== Structure Groups =====

	// Structure group generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Structures")
	FStructureGroupSettings StructureGroupSettings;

	// Maximum slope for structure placement (in degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Structures", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxStructureSlope = 15.0f;

	// ===== Performance Settings =====

	// Chunk size for generation (prevents infinite loop detection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Performance")
	int32 ChunkSize = 64;

	// Max chunks to process per frame (0 = all at once)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Performance")
	int32 MaxChunksPerFrame = 0;

	// ===== Debug Settings =====

	// Enable progress logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Debug")
	bool bEnableProgressLogging = false;

	// Visualize biomes (shows biome colors instead of terrain)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Debug")
	bool bVisualizeBiomes = false;

	// Visualize temperature map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Debug")
	bool bVisualizeTemperature = false;

	// Visualize moisture map
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Debug")
	bool bVisualizeMoisture = false;
};
