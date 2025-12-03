// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaFishingSystemDefinitions.h
 * @brief Fishing system type definitions and data structures
 * 
 * Contains enums, structs, and data assets for the fishing minigame system
 * including fish data, fishing spots, spawn tables, and caught fish records.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFishingSystemDefinitions.generated.h"

/**
 * @enum EFishRarity
 * @brief Rarity classification for fish
 */
UENUM(BlueprintType)
enum class EFishRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * @enum EFishingMinigameType
 * @brief Types of fishing minigames
 */
UENUM(BlueprintType)
enum class EFishingMinigameType : uint8
{
	TimingBased		UMETA(DisplayName = "Timing Based"),		// Timing-based catch
	BarBalance		UMETA(DisplayName = "Bar Balance"),		// Bar balance control
	QuickTimeEvent	UMETA(DisplayName = "Quick Time Event"),	// QTE event
	ReelTension		UMETA(DisplayName = "Reel Tension")		// Line tension management
};

/**
 * @enum EFishingSpotType
 * @brief Types of fishing locations
 */
UENUM(BlueprintType)
enum class EFishingSpotType : uint8
{
	River		UMETA(DisplayName = "River"),
	Lake		UMETA(DisplayName = "Lake"),
	Ocean		UMETA(DisplayName = "Ocean"),
	Pond		UMETA(DisplayName = "Pond"),
	Special		UMETA(DisplayName = "Special")		// Special fishing spot
};

/**
 * @struct FFishData
 * @brief Fish data structure
 */
USTRUCT(BlueprintType)
struct FFishData
{
	GENERATED_BODY()

	/** Fish name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishName;

	/** Fish description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** Rarity tier of the fish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** Minimum size in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinSize = 10.0f;

	/** Maximum size in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxSize = 50.0f;

	/** Minimum weight in kilograms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinWeight = 0.5f;

	/** Maximum weight in kilograms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxWeight = 5.0f;

	/** Base selling price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 BasePrice = 10;

	/** Fish static mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UStaticMesh> FishMesh;

	/** Fish icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Appearance start hour (24h format, -1 = anytime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearStartHour = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearEndHour = -1;

	/** Required weather conditions (empty = all weather) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredWeather;

	/** Required seasons (empty = all seasons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredSeasons;
};

/**
 * @struct FFishingSpotSpawnEntry
 * @brief Fishing spot spawn table entry
 */
USTRUCT(BlueprintType)
struct FFishingSpotSpawnEntry
{
	GENERATED_BODY()

	/** Fish identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName FishID;

	/** Spawn chance percentage (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnChance = 10.0f;

	/** Minimum fishing level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MinFishingLevel = 1;
};

/**
 * @struct FFishingMinigameSettings
 * @brief Fishing minigame configuration
 */
USTRUCT(BlueprintType)
struct FFishingMinigameSettings
{
	GENERATED_BODY()

	/** Minigame type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	EFishingMinigameType MinigameType = EFishingMinigameType::TimingBased;

	/** Difficulty level (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	int32 Difficulty = 5;

	/** Time limit in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float TimeLimit = 30.0f;

	/** Success bonus multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float SuccessBonus = 1.2f;

	/** Perfect catch bonus multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float PerfectBonus = 1.5f;
};

/**
 * @class UFishingSpotData
 * @brief Fishing spot data asset
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UFishingSpotData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Fishing spot display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FText SpotName;

	/** Type of fishing spot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	EFishingSpotType SpotType = EFishingSpotType::River;

	/** Spawn table entries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	TArray<FFishingSpotSpawnEntry> SpawnTable;

	/** Minimum fishing level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	int32 MinimumFishingLevel = 1;

	/** Minigame configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FFishingMinigameSettings MinigameSettings;
};

/**
 * @struct FCaughtFish
 * @brief Caught fish information record
 */
USTRUCT(BlueprintType)
struct FCaughtFish
{
	GENERATED_BODY()

	/** Fish identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FName FishID;

	/** Size in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Size = 0.0f;

	/** Weight in kilograms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Weight = 0.0f;

	/** Timestamp when caught */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FDateTime CaughtTime;

	/** World location where caught */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FVector CaughtLocation;

	/** Quality score (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float QualityScore = 50.0f;
};

/**
 * @struct FFishDataTableRow
 * @brief Fish data table row for editor DataTable management
 * 
 * Used for managing fish data in DataTable format within the editor.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FFishDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Fish unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishID;

	/** Display name of the fish */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText DisplayName;

	/** Description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** Rarity tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** Minimum size in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinSize = 10.0f;

	/** Maximum size in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxSize = 50.0f;

	/** Minimum weight in kilograms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinWeight = 0.5f;

	/** Maximum weight in kilograms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxWeight = 5.0f;

	/** Base selling price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 BasePrice = 10;

	/** Experience gained when caught */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 ExperienceGain = 5;

	/** Fish icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 3D mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Valid fishing spot types for spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<EFishingSpotType> ValidSpotTypes;

	/** Minimum fishing level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 MinFishingLevel = 1;

	/** Appearance start hour (0-23, -1 = anytime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearStartHour = -1;

	/** Appearance end hour (0-23, -1 = anytime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearEndHour = -1;

	/** Required weather conditions (empty = all weather) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredWeather;

	/** Required seasons (empty = all seasons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredSeasons;
};
