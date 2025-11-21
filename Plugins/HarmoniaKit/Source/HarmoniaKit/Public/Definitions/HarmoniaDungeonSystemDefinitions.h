// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonSystemDefinitions.generated.h"

/**
 * 던전 타입
 */
UENUM(BlueprintType)
enum class EDungeonType : uint8
{
	None UMETA(DisplayName = "None"),
	Instance UMETA(DisplayName = "Instance Dungeon"),		// 인스턴스 던전 - 파티용
	Raid UMETA(DisplayName = "Raid"),						// 레이드 - 대규모 공격대
	Infinite UMETA(DisplayName = "Infinite Dungeon"),		// 무한 던전 - 로그라이크
	Challenge UMETA(DisplayName = "Challenge Mode")			// 챌린지 모드 - 고난이도 변형
};

/**
 * 던전 난이도
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
 * 던전 상태
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
 * 레이드 역할
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
 * 던전 보상 정보
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonReward
{
	GENERATED_BODY()

	/** 경험치 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 ExperienceReward = 0;

	/** 골드 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 GoldReward = 0;

	/** 아이템 보상 (테이블 참조) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	TArray<FName> RewardItemIDs;

	/** 보상 배율 (난이도별) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	float RewardMultiplier = 1.0f;
};

/**
 * 던전 입장 조건
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRequirement
{
	GENERATED_BODY()

	/** 최소 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinLevel = 1;

	/** 권장 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 RecommendedLevel = 1;

	/** 최소 파티 인원 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinPartySize = 1;

	/** 최대 파티 인원 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MaxPartySize = 5;

	/** 필요 아이템 (입장권 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredItemIDs;

	/** 필요 퀘스트 완료 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredQuestIDs;
};

/**
 * 무한 던전 층 정보
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FInfiniteDungeonFloor
{
	GENERATED_BODY()

	/** 층 번호 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	int32 FloorNumber = 1;

	/** 몬스터 레벨 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterLevelMultiplier = 1.0f;

	/** 몬스터 수 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterCountMultiplier = 1.0f;

	/** 보스 등장 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	bool bHasBoss = false;

	/** 특수 이벤트 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	FGameplayTag EventTag;
};

/**
 * 던전 랭킹 정보
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRankingEntry
{
	GENERATED_BODY()

	/** 플레이어/파티 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FString PlayerName;

	/** 클리어 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	float ClearTime = 0.0f;

	/** 점수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 Score = 0;

	/** 난이도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** 클리어 날짜 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FDateTime ClearDate;

	/** 시즌 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 SeasonID = 0;
};

/**
 * 챌린지 모드 수정자
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonChallengeModifier
{
	GENERATED_BODY()

	/** 수정자 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierName;

	/** 수정자 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierDescription;

	/** 몬스터 체력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterHealthMultiplier = 1.0f;

	/** 몬스터 공격력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterDamageMultiplier = 1.0f;

	/** 플레이어 체력 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float PlayerHealthMultiplier = 1.0f;

	/** 점수 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float ScoreMultiplier = 1.0f;

	/** 특수 효과 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FGameplayTagContainer EffectTags;

	/** Equality operator for TArray operations */
	bool operator==(const FDungeonChallengeModifier& Other) const
	{
		return ModifierName.EqualTo(Other.ModifierName);
	}
};

/**
 * 던전 정보 (DataAsset)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UDungeonDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 던전 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FName DungeonID;

	/** 던전 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonName;

	/** 던전 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonDescription;

	/** 던전 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	EDungeonType DungeonType = EDungeonType::Instance;

	/** 입장 조건 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FDungeonRequirement Requirements;

	/** 보상 (난이도별) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TMap<EDungeonDifficulty, FDungeonReward> Rewards;

	/** 제한 시간 (초, 0 = 무제한) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	float TimeLimit = 0.0f;

	/** 던전 레벨 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TSoftObjectPtr<UWorld> DungeonLevel;

	/** 던전 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FGameplayTagContainer DungeonTags;

	/** 무한 던전용 층 정보 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Infinite", meta = (EditCondition = "DungeonType == EDungeonType::Infinite"))
	TArray<FInfiniteDungeonFloor> InfiniteFloors;

	/** 챌린지 모드용 수정자 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Challenge", meta = (EditCondition = "DungeonType == EDungeonType::Challenge"))
	TArray<FDungeonChallengeModifier> ChallengeModifiers;
};

/**
 * 레이드 정보 (DataAsset)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API URaidDataAsset : public UDungeonDataAsset
{
	GENERATED_BODY()

public:
	/** 최소 공격대 인원 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MinRaidSize = 10;

	/** 최대 공격대 인원 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MaxRaidSize = 40;

	/** 권장 역할 구성 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	TMap<ERaidRole, int32> RecommendedRoleComposition;

	/** 부활 제한 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 ReviveLimit = 3;

	/** 보스 페이즈 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 BossPhaseCount = 1;
};
