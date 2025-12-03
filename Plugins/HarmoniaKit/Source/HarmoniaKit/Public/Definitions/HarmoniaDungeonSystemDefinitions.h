// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaDungeonSystemDefinitions.h
 * @brief Dungeon system type definitions
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonSystemDefinitions.generated.h"

/**
 * @enum EDungeonType
 * @brief Dungeon type
 */
UENUM(BlueprintType)
enum class EDungeonType : uint8
{
	None UMETA(DisplayName = "None"),
	Instance UMETA(DisplayName = "Instance Dungeon"),		// Instance dungeon - Party
	Raid UMETA(DisplayName = "Raid"),						// Raid - Large scale
	Infinite UMETA(DisplayName = "Infinite Dungeon"),		// Infinite dungeon - Roguelike
	Challenge UMETA(DisplayName = "Challenge Mode")			// Challenge mode - High difficulty variant
};

/**
 * @enum EDungeonDifficulty
 * @brief Dungeon difficulty
 */
UENUM(BlueprintType)
enum class EDungeonDifficulty : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Hard UMETA(DisplayName = "Hard"),
	Expert UMETA(DisplayName = "Expert"),
	Master UMETA(DisplayName = "Master"),
	Nightmare UMETA(DisplayName = "Nightmare")
};

/**
 * @enum EDungeonState
 * @brief Dungeon state
 */
UENUM(BlueprintType)
enum class EDungeonState : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	InProgress UMETA(DisplayName = "In Progress"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * @enum ERaidRole
 * @brief Raid role
 */
UENUM(BlueprintType)
enum class ERaidRole : uint8
{
	None UMETA(DisplayName = "None"),
	Tank UMETA(DisplayName = "Tank"),
	Healer UMETA(DisplayName = "Healer"),
	DPS UMETA(DisplayName = "DPS"),
	Support UMETA(DisplayName = "Support")
};

/**
 * @struct FDungeonReward
 * @brief Dungeon reward info
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonReward
{
	GENERATED_BODY()

	/** Experience reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 ExperienceReward = 0;

	/** Gold reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 GoldReward = 0;

	/** Item rewards (item references) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	TArray<FName> RewardItemIDs;

	/** Reward multiplier (per difficulty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	float RewardMultiplier = 1.0f;
};

/**
 * @struct FDungeonRequirement
 * @brief Dungeon entry requirements
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRequirement
{
	GENERATED_BODY()

	/** Minimum level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinLevel = 1;

	/** Recommended level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 RecommendedLevel = 1;

	/** Minimum party size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinPartySize = 1;

	/** Maximum party size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MaxPartySize = 5;

	/** Required items (entry keys) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredItemIDs;

	/** Required quest completions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredQuestIDs;
};

/**
 * @struct FInfiniteDungeonFloor
 * @brief Infinite dungeon floor info
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FInfiniteDungeonFloor
{
	GENERATED_BODY()

	/** Floor number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	int32 FloorNumber = 1;

	/** Monster level multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterLevelMultiplier = 1.0f;

	/** Monster count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterCountMultiplier = 1.0f;

	/** Has boss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	bool bHasBoss = false;

	/** Special event tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	FGameplayTag EventTag;
};

/**
 * @struct FDungeonRankingEntry
 * @brief Dungeon ranking info
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRankingEntry
{
	GENERATED_BODY()

	/** Player/Party name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FString PlayerName;

	/** Clear time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	float ClearTime = 0.0f;

	/** Score */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 Score = 0;

	/** Difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** Clear date */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FDateTime ClearDate;

	/** Season ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 SeasonID = 0;
};

/**
 * @struct FDungeonChallengeModifier
 * @brief Challenge mode modifier
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonChallengeModifier
{
	GENERATED_BODY()

	/** Modifier name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierName;

	/** Modifier description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierDescription;

	/** Monster health multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterHealthMultiplier = 1.0f;

	/** Monster damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterDamageMultiplier = 1.0f;

	/** Player health multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float PlayerHealthMultiplier = 1.0f;

	/** Score multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float ScoreMultiplier = 1.0f;

	/** Special effect tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FGameplayTagContainer EffectTags;

	/** Equality operator for TArray operations */
	bool operator==(const FDungeonChallengeModifier& Other) const
	{
		return ModifierName.EqualTo(Other.ModifierName);
	}
};

/**
 * @class UDungeonDataAsset
 * @brief DataAsset containing dungeon configuration data
 * 
 * Defines dungeon settings including requirements, rewards, time limits,
 * and type-specific configurations for instance, infinite, and challenge modes.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UDungeonDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique dungeon identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FName DungeonID;

	/** Display name of the dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonName;

	/** Dungeon description text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonDescription;

	/** Type of dungeon (Instance, Infinite, Raid, Challenge) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	EDungeonType DungeonType = EDungeonType::Instance;

	/** Entry requirements for this dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FDungeonRequirement Requirements;

	/** Rewards mapped by difficulty level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TMap<EDungeonDifficulty, FDungeonReward> Rewards;

	/** Time limit in seconds (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	float TimeLimit = 0.0f;

	/** Dungeon level world asset reference */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TSoftObjectPtr<UWorld> DungeonLevel;

	/** Gameplay tags associated with this dungeon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FGameplayTagContainer DungeonTags;

	/** Floor configurations for infinite dungeon mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Infinite", meta = (EditCondition = "DungeonType == EDungeonType::Infinite"))
	TArray<FInfiniteDungeonFloor> InfiniteFloors;

	/** Modifier settings for challenge mode */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Challenge", meta = (EditCondition = "DungeonType == EDungeonType::Challenge"))
	TArray<FDungeonChallengeModifier> ChallengeModifiers;
};

/**
 * @class URaidDataAsset
 * @brief DataAsset extending dungeon data for raid configurations
 * 
 * Extends UDungeonDataAsset with raid-specific settings such as
 * party size limits, role compositions, and boss phase configurations.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API URaidDataAsset : public UDungeonDataAsset
{
	GENERATED_BODY()

public:
	/** Minimum raid party size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MinRaidSize = 10;

	/** Maximum raid party size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MaxRaidSize = 40;

	/** Recommended role composition for the raid */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	TMap<ERaidRole, int32> RecommendedRoleComposition;

	/** Number of revives allowed during the raid */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 ReviveLimit = 3;

	/** Number of boss phases in the encounter */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 BossPhaseCount = 1;
};
