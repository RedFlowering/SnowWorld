// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonSystemDefinitions.generated.h"

/**
 * ?�전 ?�??
 */
UENUM(BlueprintType)
enum class EDungeonType : uint8
{
	None UMETA(DisplayName = "None"),
	Instance UMETA(DisplayName = "Instance Dungeon"),		// ?�스?�스 ?�전 - ?�티??
	Raid UMETA(DisplayName = "Raid"),						// ?�이??- ?�규모 공격?�
	Infinite UMETA(DisplayName = "Infinite Dungeon"),		// 무한 ?�전 - 로그?�이??
	Challenge UMETA(DisplayName = "Challenge Mode")			// 챌린지 모드 - 고난?�도 변??
};

/**
 * ?�전 ?�이??
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
 * ?�전 ?�태
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
 * ?�이????��
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
 * ?�전 보상 ?�보
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonReward
{
	GENERATED_BODY()

	/** 경험�?보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 ExperienceReward = 0;

	/** 골드 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 GoldReward = 0;

	/** ?�이??보상 (?�이�?참조) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	TArray<FName> RewardItemIDs;

	/** 보상 배율 (?�이?�별) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	float RewardMultiplier = 1.0f;
};

/**
 * ?�전 ?�장 조건
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRequirement
{
	GENERATED_BODY()

	/** 최소 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinLevel = 1;

	/** 권장 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 RecommendedLevel = 1;

	/** 최소 ?�티 ?�원 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinPartySize = 1;

	/** 최�? ?�티 ?�원 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MaxPartySize = 5;

	/** ?�요 ?�이??(?�장�??? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredItemIDs;

	/** ?�요 ?�스???�료 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredQuestIDs;
};

/**
 * 무한 ?�전 �??�보
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FInfiniteDungeonFloor
{
	GENERATED_BODY()

	/** �?번호 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	int32 FloorNumber = 1;

	/** 몬스???�벨 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterLevelMultiplier = 1.0f;

	/** 몬스????배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterCountMultiplier = 1.0f;

	/** 보스 ?�장 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	bool bHasBoss = false;

	/** ?�수 ?�벤???�그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	FGameplayTag EventTag;
};

/**
 * ?�전 ??�� ?�보
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRankingEntry
{
	GENERATED_BODY()

	/** ?�레?�어/?�티 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FString PlayerName;

	/** ?�리???�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	float ClearTime = 0.0f;

	/** ?�수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 Score = 0;

	/** ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** ?�리???�짜 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FDateTime ClearDate;

	/** ?�즌 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 SeasonID = 0;
};

/**
 * 챌린지 모드 ?�정??
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonChallengeModifier
{
	GENERATED_BODY()

	/** ?�정???�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierName;

	/** ?�정???�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierDescription;

	/** 몬스??체력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterHealthMultiplier = 1.0f;

	/** 몬스??공격??배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterDamageMultiplier = 1.0f;

	/** ?�레?�어 체력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float PlayerHealthMultiplier = 1.0f;

	/** ?�수 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float ScoreMultiplier = 1.0f;

	/** ?�수 ?�과 ?�그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FGameplayTagContainer EffectTags;

	/** Equality operator for TArray operations */
	bool operator==(const FDungeonChallengeModifier& Other) const
	{
		return ModifierName.EqualTo(Other.ModifierName);
	}
};

/**
 * ?�전 ?�보 (DataAsset)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UDungeonDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** ?�전 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FName DungeonID;

	/** ?�전 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonName;

	/** ?�전 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonDescription;

	/** ?�전 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	EDungeonType DungeonType = EDungeonType::Instance;

	/** ?�장 조건 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FDungeonRequirement Requirements;

	/** 보상 (?�이?�별) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TMap<EDungeonDifficulty, FDungeonReward> Rewards;

	/** ?�한 ?�간 (�? 0 = 무제?? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	float TimeLimit = 0.0f;

	/** ?�전 ?�벨 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TSoftObjectPtr<UWorld> DungeonLevel;

	/** ?�전 ?�그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FGameplayTagContainer DungeonTags;

	/** 무한 ?�전??�??�보 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Infinite", meta = (EditCondition = "DungeonType == EDungeonType::Infinite"))
	TArray<FInfiniteDungeonFloor> InfiniteFloors;

	/** 챌린지 모드???�정??*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Challenge", meta = (EditCondition = "DungeonType == EDungeonType::Challenge"))
	TArray<FDungeonChallengeModifier> ChallengeModifiers;
};

/**
 * ?�이???�보 (DataAsset)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API URaidDataAsset : public UDungeonDataAsset
{
	GENERATED_BODY()

public:
	/** 최소 공격?� ?�원 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MinRaidSize = 10;

	/** 최�? 공격?� ?�원 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MaxRaidSize = 40;

	/** 권장 ??�� 구성 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	TMap<ERaidRole, int32> RecommendedRoleComposition;

	/** 부???�한 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 ReviveLimit = 3;

	/** 보스 ?�이�???*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 BossPhaseCount = 1;
};
