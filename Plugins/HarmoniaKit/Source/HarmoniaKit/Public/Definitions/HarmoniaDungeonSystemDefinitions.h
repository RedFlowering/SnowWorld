// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonSystemDefinitions.generated.h"

/**
 * ?˜ì „ ?€??
 */
UENUM(BlueprintType)
enum class EDungeonType : uint8
{
	None UMETA(DisplayName = "None"),
	Instance UMETA(DisplayName = "Instance Dungeon"),		// ?¸ìŠ¤?´ìŠ¤ ?˜ì „ - ?Œí‹°??
	Raid UMETA(DisplayName = "Raid"),						// ?ˆì´??- ?€ê·œëª¨ ê³µê²©?€
	Infinite UMETA(DisplayName = "Infinite Dungeon"),		// ë¬´í•œ ?˜ì „ - ë¡œê·¸?¼ì´??
	Challenge UMETA(DisplayName = "Challenge Mode")			// ì±Œë¦°ì§€ ëª¨ë“œ - ê³ ë‚œ?´ë„ ë³€??
};

/**
 * ?˜ì „ ?œì´??
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
 * ?˜ì „ ?íƒœ
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
 * ?ˆì´????• 
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
 * ?˜ì „ ë³´ìƒ ?•ë³´
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonReward
{
	GENERATED_BODY()

	/** ê²½í—˜ì¹?ë³´ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 ExperienceReward = 0;

	/** ê³¨ë“œ ë³´ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	int32 GoldReward = 0;

	/** ?„ì´??ë³´ìƒ (?Œì´ë¸?ì°¸ì¡°) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	TArray<FName> RewardItemIDs;

	/** ë³´ìƒ ë°°ìœ¨ (?œì´?„ë³„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Reward")
	float RewardMultiplier = 1.0f;
};

/**
 * ?˜ì „ ?…ì¥ ì¡°ê±´
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRequirement
{
	GENERATED_BODY()

	/** ìµœì†Œ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinLevel = 1;

	/** ê¶Œì¥ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 RecommendedLevel = 1;

	/** ìµœì†Œ ?Œí‹° ?¸ì› */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MinPartySize = 1;

	/** ìµœë? ?Œí‹° ?¸ì› */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	int32 MaxPartySize = 5;

	/** ?„ìš” ?„ì´??(?…ì¥ê¶??? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredItemIDs;

	/** ?„ìš” ?˜ìŠ¤???„ë£Œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Requirement")
	TArray<FName> RequiredQuestIDs;
};

/**
 * ë¬´í•œ ?˜ì „ ì¸??•ë³´
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FInfiniteDungeonFloor
{
	GENERATED_BODY()

	/** ì¸?ë²ˆí˜¸ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	int32 FloorNumber = 1;

	/** ëª¬ìŠ¤???ˆë²¨ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterLevelMultiplier = 1.0f;

	/** ëª¬ìŠ¤????ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	float MonsterCountMultiplier = 1.0f;

	/** ë³´ìŠ¤ ?±ì¥ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	bool bHasBoss = false;

	/** ?¹ìˆ˜ ?´ë²¤???œê·¸ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Infinite")
	FGameplayTag EventTag;
};

/**
 * ?˜ì „ ??‚¹ ?•ë³´
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonRankingEntry
{
	GENERATED_BODY()

	/** ?Œë ˆ?´ì–´/?Œí‹° ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FString PlayerName;

	/** ?´ë¦¬???œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	float ClearTime = 0.0f;

	/** ?ìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 Score = 0;

	/** ?œì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** ?´ë¦¬??? ì§œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	FDateTime ClearDate;

	/** ?œì¦Œ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Ranking")
	int32 SeasonID = 0;
};

/**
 * ì±Œë¦°ì§€ ëª¨ë“œ ?˜ì •??
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FDungeonChallengeModifier
{
	GENERATED_BODY()

	/** ?˜ì •???´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierName;

	/** ?˜ì •???¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FText ModifierDescription;

	/** ëª¬ìŠ¤??ì²´ë ¥ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterHealthMultiplier = 1.0f;

	/** ëª¬ìŠ¤??ê³µê²©??ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float MonsterDamageMultiplier = 1.0f;

	/** ?Œë ˆ?´ì–´ ì²´ë ¥ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float PlayerHealthMultiplier = 1.0f;

	/** ?ìˆ˜ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	float ScoreMultiplier = 1.0f;

	/** ?¹ìˆ˜ ?¨ê³¼ ?œê·¸ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Challenge")
	FGameplayTagContainer EffectTags;

	/** Equality operator for TArray operations */
	bool operator==(const FDungeonChallengeModifier& Other) const
	{
		return ModifierName.EqualTo(Other.ModifierName);
	}
};

/**
 * ?˜ì „ ?•ë³´ (DataAsset)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UDungeonDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** ?˜ì „ ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FName DungeonID;

	/** ?˜ì „ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonName;

	/** ?˜ì „ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FText DungeonDescription;

	/** ?˜ì „ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	EDungeonType DungeonType = EDungeonType::Instance;

	/** ?…ì¥ ì¡°ê±´ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FDungeonRequirement Requirements;

	/** ë³´ìƒ (?œì´?„ë³„) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TMap<EDungeonDifficulty, FDungeonReward> Rewards;

	/** ?œí•œ ?œê°„ (ì´? 0 = ë¬´ì œ?? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	float TimeLimit = 0.0f;

	/** ?˜ì „ ?ˆë²¨ ê²½ë¡œ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	TSoftObjectPtr<UWorld> DungeonLevel;

	/** ?˜ì „ ?œê·¸ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon")
	FGameplayTagContainer DungeonTags;

	/** ë¬´í•œ ?˜ì „??ì¸??•ë³´ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Infinite", meta = (EditCondition = "DungeonType == EDungeonType::Infinite"))
	TArray<FInfiniteDungeonFloor> InfiniteFloors;

	/** ì±Œë¦°ì§€ ëª¨ë“œ???˜ì •??*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dungeon|Challenge", meta = (EditCondition = "DungeonType == EDungeonType::Challenge"))
	TArray<FDungeonChallengeModifier> ChallengeModifiers;
};

/**
 * ?ˆì´???•ë³´ (DataAsset)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API URaidDataAsset : public UDungeonDataAsset
{
	GENERATED_BODY()

public:
	/** ìµœì†Œ ê³µê²©?€ ?¸ì› */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MinRaidSize = 10;

	/** ìµœë? ê³µê²©?€ ?¸ì› */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 MaxRaidSize = 40;

	/** ê¶Œì¥ ??•  êµ¬ì„± */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	TMap<ERaidRole, int32> RecommendedRoleComposition;

	/** ë¶€???œí•œ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 ReviveLimit = 3;

	/** ë³´ìŠ¤ ?˜ì´ì¦???*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid")
	int32 BossPhaseCount = 1;
};
