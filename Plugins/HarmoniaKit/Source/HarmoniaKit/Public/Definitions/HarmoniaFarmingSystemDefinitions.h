// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaFarmingSystemDefinitions.h
 * @brief Farming system type definitions and data structures
 * 
 * Contains enums, structs, and data assets for the farming system
 * including crops, soil, fertilizers, and harvest mechanics.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFarmingSystemDefinitions.generated.h"

/**
 * @enum ECropType
 * @brief Types of crops that can be grown
 */
UENUM(BlueprintType)
enum class ECropType : uint8
{
	Vegetable	UMETA(DisplayName = "Vegetable"),	// Vegetables
	Fruit		UMETA(DisplayName = "Fruit"),		// Fruits
	Grain		UMETA(DisplayName = "Grain"),		// Grains
	Herb		UMETA(DisplayName = "Herb"),		// Herbs
	Flower		UMETA(DisplayName = "Flower"),		// Flowers
	Tree		UMETA(DisplayName = "Tree")			// Trees
};

/**
 * @enum ECropGrowthStage
 * @brief Crop growth stages
 */
UENUM(BlueprintType)
enum class ECropGrowthStage : uint8
{
	Seed		UMETA(DisplayName = "Seed"),		// Seed
	Sprout		UMETA(DisplayName = "Sprout"),		// Sprouting
	Growing		UMETA(DisplayName = "Growing"),		// Growing
	Mature		UMETA(DisplayName = "Mature"),		// Mature
	Harvest		UMETA(DisplayName = "Harvest"),		// Ready to harvest
	Withered	UMETA(DisplayName = "Withered")		// Withered
};

/**
 * @enum ESoilQuality
 * @brief Soil quality levels
 */
UENUM(BlueprintType)
enum class ESoilQuality : uint8
{
	Poor		UMETA(DisplayName = "Poor"),
	Normal		UMETA(DisplayName = "Normal"),
	Good		UMETA(DisplayName = "Good"),
	Excellent	UMETA(DisplayName = "Excellent"),
	Perfect		UMETA(DisplayName = "Perfect")
};

/**
 * @enum ESeason
 * @brief Season types
 */
UENUM(BlueprintType)
enum class ESeason : uint8
{
	Spring		UMETA(DisplayName = "Spring"),
	Summer		UMETA(DisplayName = "Summer"),
	Autumn		UMETA(DisplayName = "Autumn"),
	Winter		UMETA(DisplayName = "Winter"),
	AllSeasons	UMETA(DisplayName = "All Seasons")
};

/**
 * @struct FCropData
 * @brief Crop data structure
 */
USTRUCT(BlueprintType)
struct FCropData
{
	GENERATED_BODY()

	/** Crop name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropName;

	/** Crop description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** Type of crop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** Seasons when crop can grow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TArray<ESeason> GrowingSeasons;

	/** Sprout stage growth time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float SproutTime = 300.0f;

	/** Growing stage growth time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowingTime = 600.0f;

	/** Maturity stage growth time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float MaturityTime = 900.0f;

	/** Harvest window duration (seconds, withers after) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float HarvestWindow = 600.0f;

	/** Minimum harvest yield */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MinYield = 1;

	/** Maximum harvest yield */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MaxYield = 3;

	/** Can be reharvested */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	bool bReharvestablle = false;

	/** Reharvest time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float ReharvestTime = 300.0f;

	/** Seed purchase price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 SeedPrice = 10;

	/** Crop selling price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 CropPrice = 30;

	/** Experience reward on harvest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 ExperienceReward = 15;

	/** Crop meshes per growth stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TMap<ECropGrowthStage, TSoftObjectPtr<UStaticMesh>> StageMeshes;

	/** Crop icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * @struct FSoilData
 * @brief Soil data structure
 */
USTRUCT(BlueprintType)
struct FSoilData
{
	GENERATED_BODY()

	/** Soil quality level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	ESoilQuality Quality = ESoilQuality::Normal;

	/** Moisture level (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float MoistureLevel = 50.0f;

	/** Fertility level (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilityLevel = 50.0f;

	/** Fertilizer applied flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	bool bFertilized = false;

	/** Fertilizer effect remaining duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilizerDuration = 0.0f;

	/** Growth speed bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float GrowthSpeedBonus = 0.0f;

	/** Yield bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float YieldBonus = 0.0f;
};

/**
 * @struct FCropInstance
 * @brief Crop instance runtime data
 */
USTRUCT(BlueprintType)
struct FCropInstance
{
	GENERATED_BODY()

	/** Crop identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** Current growth stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropGrowthStage CurrentStage = ECropGrowthStage::Seed;

	/** Planting timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime PlantedTime;

	/** Current stage start timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime StageStartTime;

	/** Last watering timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime LastWateredTime;

	/** Soil data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FSoilData Soil;

	/** Total harvest count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 HarvestCount = 0;

	/** Quality score (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float QualityScore = 50.0f;
};

/**
 * @struct FFertilizerData
 * @brief Fertilizer data structure
 */
USTRUCT(BlueprintType)
struct FFertilizerData
{
	GENERATED_BODY()

	/** Fertilizer name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FName FertilizerName;

	/** Fertilizer description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FText Description;

	/** Growth speed bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float GrowthSpeedBonus = 20.0f;

	/** Yield bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float YieldBonus = 10.0f;

	/** Quality improvement bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float QualityBonus = 15.0f;

	/** Effect duration (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float Duration = 1800.0f;

	/** Fertilizer purchase price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	int32 Price = 50;
};

/**
 * @struct FHarvestResult
 * @brief Harvest result data
 */
USTRUCT(BlueprintType)
struct FHarvestResult
{
	GENERATED_BODY()

	/** Harvested crop identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName CropID;

	/** Harvest quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** Quality score */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float Quality = 50.0f;

	/** Experience gained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** Bonus item IDs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;

	/** Received seeds flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bGotSeeds = false;

	/** Number of seeds obtained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 SeedCount = 0;
};

/**
 * @struct FCropDataTableRow
 * @brief Crop data table row for editor DataTable management
 * 
 * Used for managing crops in DataTable format within the editor.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FCropDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Crop unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** Display name of the crop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText DisplayName;

	/** Description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** Type of crop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** Required growing season */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	ESeason RequiredSeason = ESeason::AllSeasons;

	/** Growth time in game hours */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowthTimeHours = 24.0f;

	/** Water consumption rate per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float WaterConsumptionRate = 0.05f;

	/** Minimum farming level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Requirement")
	int32 MinFarmingLevel = 1;

	/** Minimum harvest yield */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MinYield = 1;

	/** Maximum harvest yield */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxYield = 3;

	/** Can be reharvested (perennial crop) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	bool bReharvestable = false;

	/** Regrowth time in hours (if reharvestable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float RegrowthTimeHours = 12.0f;

	/** Maximum harvest count (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxHarvests = 0;

	/** Seed drop chance on harvest (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float SeedDropChance = 0.3f;

	/** Experience gained on harvest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 ExperienceGain = 5;

	/** Seed item ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName SeedItemID;

	/** Harvested item ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName HarvestedItemID;

	/** Base selling price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 BasePrice = 10;

	/** Crop icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TSoftObjectPtr<UTexture2D> Icon;
};
