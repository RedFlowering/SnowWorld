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
	POI         UMETA(DisplayName = "Point of Interest"),
	CaveEntrance UMETA(DisplayName = "Cave Entrance"),
	OreVein     UMETA(DisplayName = "Ore Vein"),
};

/**
 * POI (Point of Interest) Types
 */
UENUM(BlueprintType)
enum class EPOIType : uint8
{
	None         UMETA(DisplayName = "None"),
	Dungeon      UMETA(DisplayName = "Dungeon"),
	Treasure     UMETA(DisplayName = "Treasure"),
	QuestLocation UMETA(DisplayName = "Quest Location"),
	Boss         UMETA(DisplayName = "Boss Arena"),
	Camp         UMETA(DisplayName = "Camp"),
	Ruins        UMETA(DisplayName = "Ruins"),
};

/**
 * Resource Types for Advanced Distribution
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	None         UMETA(DisplayName = "None"),
	IronOre      UMETA(DisplayName = "Iron Ore"),
	GoldOre      UMETA(DisplayName = "Gold Ore"),
	CopperOre    UMETA(DisplayName = "Copper Ore"),
	Coal         UMETA(DisplayName = "Coal"),
	Gems         UMETA(DisplayName = "Gems"),
	CrystalOre   UMETA(DisplayName = "Crystal Ore"),
};

/**
 * Season Types
 */
UENUM(BlueprintType)
enum class ESeasonType : uint8
{
	Spring       UMETA(DisplayName = "Spring"),
	Summer       UMETA(DisplayName = "Summer"),
	Fall         UMETA(DisplayName = "Fall (Autumn)"),
	Winter       UMETA(DisplayName = "Winter"),
};

/**
 * Weather Types
 */
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear        UMETA(DisplayName = "Clear Sky"),
	Cloudy       UMETA(DisplayName = "Cloudy"),
	Rain         UMETA(DisplayName = "Rain"),
	HeavyRain    UMETA(DisplayName = "Heavy Rain"),
	Snow         UMETA(DisplayName = "Snow"),
	HeavySnow    UMETA(DisplayName = "Heavy Snow"),
	Sandstorm    UMETA(DisplayName = "Sandstorm"),
	Fog          UMETA(DisplayName = "Fog"),
	Thunderstorm UMETA(DisplayName = "Thunderstorm"),
	Blizzard     UMETA(DisplayName = "Blizzard"),
};

/**
 * Time of Day
 */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Night        UMETA(DisplayName = "Night"),
	Dawn         UMETA(DisplayName = "Dawn"),
	Morning      UMETA(DisplayName = "Morning"),
	Noon         UMETA(DisplayName = "Noon"),
	Afternoon    UMETA(DisplayName = "Afternoon"),
	Dusk         UMETA(DisplayName = "Dusk"),
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

	// POI-specific data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPOIType POIType = EPOIType::None;

	// POI difficulty level (0-10)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Difficulty = 0;

	// Resource-specific data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EResourceType ResourceType = EResourceType::None;

	// Resource amount/richness (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResourceAmount = 1.0f;

	// Cave depth (for cave systems, in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CaveDepth = 0.0f;
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
 * Cave System Settings
 */
USTRUCT(BlueprintType)
struct FCaveSettings
{
	GENERATED_BODY()

	// Enable cave generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves")
	bool bEnableCaves = true;

	// Cave density (0.0-1.0, how often caves occur)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CaveDensity = 0.3f;

	// Cave threshold (noise value above which caves exist)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CaveThreshold = 0.5f;

	// Minimum depth for cave generation (below surface)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float MinCaveDepth = 200.0f;

	// Maximum depth for cave generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float MaxCaveDepth = 1000.0f;

	// Cave scale (larger = bigger caves)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float CaveScale = 20.0f;

	// 3D noise settings for cave generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves")
	FPerlinNoiseSettings CaveNoiseSettings;

	// Number of cave entrance markers to place
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves")
	int32 CaveEntranceCount = 10;

	// Minimum height for cave entrances (normalized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinEntranceHeight = 0.3f;

	// Maximum height for cave entrances (normalized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caves", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxEntranceHeight = 0.8f;
};

/**
 * POI System Settings
 */
USTRUCT(BlueprintType)
struct FPOISettings
{
	GENERATED_BODY()

	// Enable POI generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bEnablePOI = true;

	// Number of POIs to generate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI", meta = (ClampMin = "0", ClampMax = "100"))
	int32 POICount = 20;

	// Minimum distance between POIs (in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI", meta = (ClampMin = "100.0", ClampMax = "50000.0"))
	float MinPOIDistance = 5000.0f;

	// POI type probabilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	TMap<EPOIType, float> POITypeProbabilities;

	// Difficulty distribution (by distance from center)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	bool bDifficultyByDistance = true;

	// Actor class mapping for POI types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "POI")
	TMap<EPOIType, TSoftClassPtr<AActor>> POIActorClasses;
};

/**
 * Ore Vein Data
 */
USTRUCT(BlueprintType)
struct FOreVeinData
{
	GENERATED_BODY()

	// Center location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Resource type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EResourceType ResourceType = EResourceType::None;

	// Vein radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	// Resource richness (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Richness = 1.0f;

	// Number of resource nodes in this vein
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NodeCount = 10;
};

/**
 * Biome-specific resource multipliers wrapper
 */
USTRUCT(BlueprintType)
struct FBiomeResourceMultipliers
{
	GENERATED_BODY()

	// Resource type multipliers for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EResourceType, float> ResourceMultipliers;
};

/**
 * Resource Distribution Settings
 */
USTRUCT(BlueprintType)
struct FResourceDistributionSettings
{
	GENERATED_BODY()

	// Enable resource distribution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	bool bEnableResources = true;

	// Number of ore veins to generate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (ClampMin = "0", ClampMax = "1000"))
	int32 OreVeinCount = 50;

	// Minimum vein radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float MinVeinRadius = 300.0f;

	// Maximum vein radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float MaxVeinRadius = 1000.0f;

	// Resource type probabilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<EResourceType, float> ResourceTypeProbabilities;

	// Biome-specific resource multipliers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<EBiomeType, FBiomeResourceMultipliers> BiomeResourceMultipliers;

	// Height-based resource distribution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<EResourceType, FVector2D> ResourceHeightRanges; // X = min, Y = max (normalized)

	// Actor class mapping for resource types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	TMap<EResourceType, TSoftClassPtr<AActor>> ResourceActorClasses;

	// Nodes per vein
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MinNodesPerVein = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxNodesPerVein = 20;
};

/**
 * Splatmap Layer Data
 */
USTRUCT(BlueprintType)
struct FSplatmapLayerData
{
	GENERATED_BODY()

	// Layer index (0-7 for Unreal Landscape)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LayerIndex = 0;

	// Layer name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LayerName = NAME_None;

	// Weight data (0-255 per tile)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<uint8> WeightData;
};

/**
 * Splatmap Generation Settings
 */
USTRUCT(BlueprintType)
struct FSplatmapSettings
{
	GENERATED_BODY()

	// Enable splatmap generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap")
	bool bEnableSplatmap = true;

	// Biome to layer mapping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap")
	TMap<EBiomeType, FName> BiomeToLayerMap;

	// Height-based layer assignments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap")
	TArray<FName> HeightLayers; // Index 0 = lowest, increasing

	// Height thresholds for layers (normalized 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap")
	TArray<float> HeightThresholds;

	// Slope-based layer (e.g., cliff texture)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap")
	FName SlopeLayerName = NAME_None;

	// Slope threshold for cliff texture (in degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SlopeThreshold = 45.0f;

	// Blend distance between layers (in tiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splatmap", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float BlendDistance = 3.0f;
};

/**
 * Cave Volume Data (for 3D cave generation)
 */
USTRUCT(BlueprintType)
struct FCaveVolumeData
{
	GENERATED_BODY()

	// 3D grid position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector GridPosition = FIntVector::ZeroValue;

	// Is this location a cave (vs solid)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCave = false;

	// Cave density value (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Density = 0.0f;
};

/**
 * Weather Probability by Season and Biome
 */
USTRUCT(BlueprintType)
struct FWeatherProbability
{
	GENERATED_BODY()

	// Weather type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeatherType WeatherType = EWeatherType::Clear;

	// Probability (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability = 0.0f;

	// Minimum duration (in game hours)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDuration = 1.0f;

	// Maximum duration (in game hours)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 6.0f;
};

/**
 * Season Visual Settings
 */
USTRUCT(BlueprintType)
struct FSeasonVisuals
{
	GENERATED_BODY()

	// Foliage color tint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FoliageTint = FLinearColor::White;

	// Grass color tint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GrassTint = FLinearColor::White;

	// Snow coverage (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SnowCoverage = 0.0f;

	// Ambient temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 20.0f;
};

/**
 * Biome-specific weather probabilities wrapper
 */
USTRUCT(BlueprintType)
struct FBiomeWeatherProbabilities
{
	GENERATED_BODY()

	// Weather probabilities for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeatherProbability> WeatherProbabilities;
};

/**
 * Season Settings
 */
USTRUCT(BlueprintType)
struct FSeasonSettings
{
	GENERATED_BODY()

	// Season type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season")
	ESeasonType Season = ESeasonType::Spring;

	// Duration in game days
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season", meta = (ClampMin = "1", ClampMax = "365"))
	int32 DurationDays = 91;

	// Visual settings for this season
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season|Visuals")
	FSeasonVisuals Visuals;

	// Weather probabilities for this season
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season|Weather")
	TArray<FWeatherProbability> WeatherProbabilities;

	// Biome-specific weather probabilities (overrides default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Season|Weather")
	TMap<EBiomeType, FBiomeWeatherProbabilities> BiomeWeatherProbabilities;
};

/**
 * Day/Night Cycle Settings
 */
USTRUCT(BlueprintType)
struct FDayNightCycleSettings
{
	GENERATED_BODY()

	// Enable day/night cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
	bool bEnableDayNightCycle = true;

	// Real-time minutes per game hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float MinutesPerGameHour = 2.0f;

	// Starting hour (0-24)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float StartingHour = 8.0f;

	// Sunrise hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float SunriseHour = 6.0f;

	// Sunset hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float SunsetHour = 18.0f;

	// Enable dynamic sun movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
	bool bDynamicSunMovement = true;

	// Moon brightness multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MoonBrightness = 0.3f;
};

/**
 * Environment System Settings
 */
USTRUCT(BlueprintType)
struct FEnvironmentSystemSettings
{
	GENERATED_BODY()

	// Enable environment system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	bool bEnableEnvironmentSystem = true;

	// Enable seasons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	bool bEnableSeasons = true;

	// Enable dynamic weather
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	bool bEnableDynamicWeather = true;

	// Starting season
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	ESeasonType StartingSeason = ESeasonType::Spring;

	// Season configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Seasons")
	TArray<FSeasonSettings> SeasonConfigs;

	// Day/Night cycle settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|DayNight")
	FDayNightCycleSettings DayNightSettings;

	// Weather transition duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Weather", meta = (ClampMin = "1.0", ClampMax = "600.0"))
	float WeatherTransitionDuration = 30.0f;

	// Minimum time between weather changes (game hours)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Weather", meta = (ClampMin = "0.5", ClampMax = "48.0"))
	float MinTimeBetweenWeatherChanges = 2.0f;

	// Current biome for weather selection (if needed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment|Weather")
	EBiomeType CurrentBiome = EBiomeType::Grassland;
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

	// ===== Cave System =====

	// Cave generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Caves")
	FCaveSettings CaveSettings;

	// ===== POI System =====

	// POI generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|POI")
	FPOISettings POISettings;

	// ===== Resource Distribution =====

	// Resource distribution settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Resources")
	FResourceDistributionSettings ResourceSettings;

	// ===== Splatmap Generation =====

	// Splatmap generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Splatmap")
	FSplatmapSettings SplatmapSettings;

	// ===== Environment System =====

	// Environment system settings (seasons, weather, day/night)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Environment")
	FEnvironmentSystemSettings EnvironmentSettings;

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

// ========================================
// Runtime Terrain Modification
// ========================================

/**
 * Terrain modification operation type
 */
UENUM(BlueprintType)
enum class ETerrainModificationType : uint8
{
	Raise         UMETA(DisplayName = "Raise Terrain"),
	Lower         UMETA(DisplayName = "Lower Terrain"),
	Flatten       UMETA(DisplayName = "Flatten Terrain"),
	Smooth        UMETA(DisplayName = "Smooth Terrain"),
	Crater        UMETA(DisplayName = "Create Crater"),
	Hill          UMETA(DisplayName = "Create Hill"),
	SetHeight     UMETA(DisplayName = "Set Absolute Height"),
	Paint         UMETA(DisplayName = "Paint Texture"),
};

/**
 * Falloff curve type for terrain modifications
 */
UENUM(BlueprintType)
enum class ETerrainFalloffType : uint8
{
	Linear        UMETA(DisplayName = "Linear"),
	Smooth        UMETA(DisplayName = "Smooth (Cosine)"),
	Spherical     UMETA(DisplayName = "Spherical"),
	Gaussian      UMETA(DisplayName = "Gaussian"),
	Sharp         UMETA(DisplayName = "Sharp (No Falloff)"),
};

/**
 * Terrain modification parameters
 */
USTRUCT(BlueprintType)
struct FTerrainModification
{
	GENERATED_BODY()

	// Center location of modification (world space)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod")
	FVector Location = FVector::ZeroVector;

	// Radius of modification (in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod", meta = (ClampMin = "10.0", ClampMax = "10000.0"))
	float Radius = 500.0f;

	// Strength of modification (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Strength = 1.0f;

	// Type of modification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod")
	ETerrainModificationType ModificationType = ETerrainModificationType::Raise;

	// Falloff curve type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod")
	ETerrainFalloffType FalloffType = ETerrainFalloffType::Smooth;

	// Target height (for SetHeight and Flatten operations, in UE units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod")
	float TargetHeight = 0.0f;

	// Smoothing iterations (for Smooth operation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SmoothIterations = 3;

	// Layer name (for Paint operation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod")
	FName PaintLayerName = NAME_None;

	// Apply modification instantly (false = queue for batch processing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TerrainMod")
	bool bApplyImmediately = true;
};

/**
 * Terrain modification result
 */
USTRUCT(BlueprintType)
struct FTerrainModificationResult
{
	GENERATED_BODY()

	// Was the modification successful?
	UPROPERTY(BlueprintReadOnly, Category = "TerrainMod")
	bool bSuccess = false;

	// Number of landscape components affected
	UPROPERTY(BlueprintReadOnly, Category = "TerrainMod")
	int32 AffectedComponents = 0;

	// Number of vertices modified
	UPROPERTY(BlueprintReadOnly, Category = "TerrainMod")
	int32 ModifiedVertices = 0;

	// Error message (if any)
	UPROPERTY(BlueprintReadOnly, Category = "TerrainMod")
	FString ErrorMessage;
};

// ========================================
// Chunk Caching System
// ========================================

/**
 * Cached chunk data for world generation
 */
USTRUCT(BlueprintType)
struct FWorldChunkData
{
	GENERATED_BODY()

	// Chunk coordinates (X, Y)
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	FIntPoint ChunkCoordinates = FIntPoint::ZeroValue;

	// Chunk size
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	int32 ChunkSize = 64;

	// Heightmap data for this chunk
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	TArray<int32> HeightData;

	// Objects in this chunk
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	TArray<FWorldObjectData> Objects;

	// Biome data for this chunk
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	TArray<FBiomeData> BiomeData;

	// Generation timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	FDateTime GenerationTime;

	// Is this chunk fully generated?
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	bool bIsFullyGenerated = false;

	// Cache hash (for validation)
	UPROPERTY(BlueprintReadOnly, Category = "Chunk")
	int32 CacheHash = 0;
};

/**
 * Chunk cache settings
 */
USTRUCT(BlueprintType)
struct FChunkCacheSettings
{
	GENERATED_BODY()

	// Enable chunk caching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache")
	bool bEnableCaching = true;

	// Maximum number of chunks to cache in memory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache", meta = (ClampMin = "10", ClampMax = "10000"))
	int32 MaxCachedChunks = 100;

	// Enable disk caching (save/load chunks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache")
	bool bEnableDiskCache = true;

	// Cache directory (relative to project Saved folder)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache")
	FString CacheDirectory = TEXT("WorldGeneratorCache");

	// Cache expiration time (in hours, 0 = never expire)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache", meta = (ClampMin = "0.0", ClampMax = "720.0"))
	float CacheExpirationHours = 24.0f;

	// Auto-save cache on generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache")
	bool bAutoSaveCache = true;

	// Compression level (0-9, 0 = no compression)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cache", meta = (ClampMin = "0", ClampMax = "9"))
	int32 CompressionLevel = 5;
};

// ========================================
// Delegates
// ========================================

/**
 * Delegate for async world generation progress
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldGenerationProgress, float, Progress);

/**
 * Delegate for async world generation completion
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWorldGenerationComplete,
	const TArray<int32>&, HeightData,
	const TArray<FWorldObjectData>&, Objects,
	bool, bSuccess);

/**
 * Delegate for season change
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeasonChanged,
	ESeasonType, NewSeason,
	float, SeasonProgress);

/**
 * Delegate for weather change (World Generator)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWorldGenWeatherChanged,
	EWeatherType, NewWeather,
	EWeatherType, PreviousWeather,
	float, TransitionDuration);

/**
 * Delegate for time of day change (World Generator)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldGenTimeOfDayChanged,
	ETimeOfDay, NewTimeOfDay,
	float, CurrentHour);

/**
 * Delegate for day/night cycle tick
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDayNightCycleTick,
	float, CurrentHour,
	int32, CurrentDay,
	float, SunAngle);
