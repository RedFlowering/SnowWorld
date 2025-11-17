// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaQuestSystemDefinitions.generated.h"

/**
 * Quest state enum
 */
UENUM(BlueprintType)
enum class EQuestState : uint8
{
	Locked			UMETA(DisplayName = "Locked"),			// 잠김 (조건 미충족)
	Available		UMETA(DisplayName = "Available"),		// 수락 가능
	InProgress		UMETA(DisplayName = "In Progress"),		// 진행 중
	ReadyToComplete	UMETA(DisplayName = "Ready to Complete"),// 완료 가능 (목표 달성)
	Completed		UMETA(DisplayName = "Completed"),		// 완료됨
	Failed			UMETA(DisplayName = "Failed"),			// 실패
	MAX				UMETA(Hidden)
};

/**
 * Quest type enum
 */
UENUM(BlueprintType)
enum class EQuestType : uint8
{
	Main			UMETA(DisplayName = "Main Quest"),		// 메인 퀘스트
	Side			UMETA(DisplayName = "Side Quest"),		// 사이드 퀘스트
	Daily			UMETA(DisplayName = "Daily Quest"),		// 일일 퀘스트
	Weekly			UMETA(DisplayName = "Weekly Quest"),	// 주간 퀘스트
	Repeatable		UMETA(DisplayName = "Repeatable"),		// 반복 가능
	Story			UMETA(DisplayName = "Story Quest"),		// 스토리 퀘스트
	Tutorial		UMETA(DisplayName = "Tutorial"),		// 튜토리얼
	Achievement		UMETA(DisplayName = "Achievement"),		// 업적
	MAX				UMETA(Hidden)
};

/**
 * Quest objective type
 */
UENUM(BlueprintType)
enum class EQuestObjectiveType : uint8
{
	Kill			UMETA(DisplayName = "Kill Enemy"),		// 적 처치
	Collect			UMETA(DisplayName = "Collect Item"),	// 아이템 수집
	Talk			UMETA(DisplayName = "Talk to NPC"),		// NPC 대화
	Reach			UMETA(DisplayName = "Reach Location"),	// 위치 도달
	Craft			UMETA(DisplayName = "Craft Item"),		// 아이템 제작
	Build			UMETA(DisplayName = "Build Structure"),	// 구조물 건설
	Equip			UMETA(DisplayName = "Equip Item"),		// 아이템 장착
	Use				UMETA(DisplayName = "Use Item"),		// 아이템 사용
	Gather			UMETA(DisplayName = "Gather Resource"),	// 자원 채집
	Deliver			UMETA(DisplayName = "Deliver Item"),	// 아이템 전달
	Escort			UMETA(DisplayName = "Escort NPC"),		// NPC 호위
	Defend			UMETA(DisplayName = "Defend Location"),	// 위치 방어
	Discover		UMETA(DisplayName = "Discover Area"),	// 지역 탐험
	Custom			UMETA(DisplayName = "Custom"),			// 커스텀 (태그 기반)
	MAX				UMETA(Hidden)
};

/**
 * Quest condition type
 * Used for quest unlock conditions and prerequisites
 */
UENUM(BlueprintType)
enum class EQuestConditionType : uint8
{
	None				UMETA(DisplayName = "None"),				// 조건 없음
	QuestCompleted		UMETA(DisplayName = "Quest Completed"),		// 선행 퀘스트 완료
	QuestNotCompleted	UMETA(DisplayName = "Quest Not Completed"),	// 특정 퀘스트 미완료
	Level				UMETA(DisplayName = "Level Requirement"),	// 레벨 요구사항
	HasItem				UMETA(DisplayName = "Has Item"),			// 아이템 소지
	HasTag				UMETA(DisplayName = "Has Tag"),				// 게임플레이 태그 소지
	RecipeKnown			UMETA(DisplayName = "Recipe Known"),		// 레시피 습득
	Custom				UMETA(DisplayName = "Custom"),				// 커스텀 조건
	MAX					UMETA(Hidden)
};

/**
 * Quest reward type
 */
UENUM(BlueprintType)
enum class EQuestRewardType : uint8
{
	Experience		UMETA(DisplayName = "Experience"),		// 경험치
	Gold			UMETA(DisplayName = "Gold"),			// 골드
	Item			UMETA(DisplayName = "Item"),			// 아이템
	Recipe			UMETA(DisplayName = "Recipe"),			// 레시피
	Tag				UMETA(DisplayName = "Gameplay Tag"),	// 게임플레이 태그
	UnlockQuest		UMETA(DisplayName = "Unlock Quest"),	// 퀘스트 해금
	Custom			UMETA(DisplayName = "Custom"),			// 커스텀 보상
	MAX				UMETA(Hidden)
};

/**
 * Quest objective data
 * Represents a single objective within a quest
 */
USTRUCT(BlueprintType)
struct FQuestObjective
{
	GENERATED_BODY()

	// Objective type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestObjectiveType ObjectiveType = EQuestObjectiveType::Kill;

	// Objective description (e.g., "Kill 10 wolves")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description = FText();

	// Target ID (enemy type, item ID, NPC ID, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID TargetId = FHarmoniaID();

	// Target gameplay tags (for custom objectives or additional filtering)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer TargetTags;

	// Required count (e.g., kill 10 enemies, collect 5 items)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	// Current progress
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	int32 CurrentCount = 0;

	// Whether this objective is optional
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bOptional = false;

	// Whether this objective is hidden from UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bHidden = false;

	// Location (for Reach, Defend, Discover objectives)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector TargetLocation = FVector::ZeroVector;

	// Radius for location-based objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0.0"))
	float LocationRadius = 500.0f;

	FQuestObjective()
		: ObjectiveType(EQuestObjectiveType::Kill)
		, Description()
		, TargetId()
		, TargetTags()
		, RequiredCount(1)
		, CurrentCount(0)
		, bOptional(false)
		, bHidden(false)
		, TargetLocation(FVector::ZeroVector)
		, LocationRadius(500.0f)
	{}

	// Check if objective is completed
	FORCEINLINE bool IsCompleted() const
	{
		return CurrentCount >= RequiredCount;
	}

	// Get progress as percentage (0.0 - 1.0)
	FORCEINLINE float GetProgress() const
	{
		if (RequiredCount <= 0) return 0.0f;
		return FMath::Clamp((float)CurrentCount / (float)RequiredCount, 0.0f, 1.0f);
	}
};

/**
 * Quest condition data
 * Used for quest unlock requirements and prerequisites
 */
USTRUCT(BlueprintType)
struct FQuestCondition
{
	GENERATED_BODY()

	// Condition type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestConditionType ConditionType = EQuestConditionType::None;

	// Target quest ID (for QuestCompleted, QuestNotCompleted conditions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID TargetQuestId = FHarmoniaID();

	// Required level (for Level condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	// Required item ID (for HasItem condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID RequiredItemId = FHarmoniaID();

	// Required item amount (for HasItem condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 RequiredItemAmount = 1;

	// Required gameplay tags (for HasTag condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer RequiredTags;

	// Required recipe ID (for RecipeKnown condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID RequiredRecipeId = FHarmoniaID();

	FQuestCondition()
		: ConditionType(EQuestConditionType::None)
		, TargetQuestId()
		, RequiredLevel(1)
		, RequiredItemId()
		, RequiredItemAmount(1)
		, RequiredTags()
		, RequiredRecipeId()
	{}
};

/**
 * Quest reward data
 */
USTRUCT(BlueprintType)
struct FQuestReward
{
	GENERATED_BODY()

	// Reward type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestRewardType RewardType = EQuestRewardType::Experience;

	// Experience amount (for Experience reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0"))
	int32 ExperienceAmount = 0;

	// Gold amount (for Gold reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0"))
	int32 GoldAmount = 0;

	// Item ID (for Item reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID ItemId = FHarmoniaID();

	// Item amount (for Item reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 ItemAmount = 1;

	// Recipe ID (for Recipe reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID RecipeId = FHarmoniaID();

	// Gameplay tags to grant (for Tag reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer TagsToGrant;

	// Quest ID to unlock (for UnlockQuest reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID QuestToUnlock = FHarmoniaID();

	FQuestReward()
		: RewardType(EQuestRewardType::Experience)
		, ExperienceAmount(0)
		, GoldAmount(0)
		, ItemId()
		, ItemAmount(1)
		, RecipeId()
		, TagsToGrant()
		, QuestToUnlock()
	{}
};

/**
 * Quest data (DataTable row)
 * Complete quest definition
 */
USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	// Quest ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FHarmoniaID QuestId = FHarmoniaID();

	// Quest type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	EQuestType QuestType = EQuestType::Side;

	// Quest name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestName = FText();

	// Quest description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestDescription = FText();

	// Quest icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TSoftObjectPtr<UTexture2D> QuestIcon = nullptr;

	// Quest category tags (e.g., "Quest.Category.Combat", "Quest.Category.Crafting")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FGameplayTagContainer CategoryTags;

	// Quest objectives
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	TArray<FQuestObjective> Objectives;

	// Quest unlock conditions (prerequisites)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Conditions")
	TArray<FQuestCondition> UnlockConditions;

	// Quest rewards
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards")
	TArray<FQuestReward> Rewards;

	// Optional rewards (player can choose one)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards")
	TArray<FQuestReward> OptionalRewards;

	// Maximum number of optional rewards player can choose
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards", meta = (ClampMin = "0"))
	int32 MaxOptionalRewardChoices = 1;

	// NPC that gives this quest
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|NPCs")
	FHarmoniaID QuestGiverId = FHarmoniaID();

	// NPC that completes this quest (can be same as giver)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|NPCs")
	FHarmoniaID QuestCompleterId = FHarmoniaID();

	// Whether this quest can be abandoned
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Settings")
	bool bCanAbandon = true;

	// Whether this quest auto-completes when objectives are met
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Settings")
	bool bAutoComplete = false;

	// Whether this quest is hidden from quest log
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Settings")
	bool bHidden = false;

	// Time limit in seconds (0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Settings", meta = (ClampMin = "0.0"))
	float TimeLimit = 0.0f;

	// Sort order (for quest log display)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Settings")
	int32 SortOrder = 0;

	// Recommended player level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Settings", meta = (ClampMin = "1"))
	int32 RecommendedLevel = 1;

	// Quests to auto-start after completing this quest
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Chain")
	TArray<FHarmoniaID> NextQuests;

	FQuestData()
		: QuestId()
		, QuestType(EQuestType::Side)
		, QuestName()
		, QuestDescription()
		, QuestIcon(nullptr)
		, CategoryTags()
		, Objectives()
		, UnlockConditions()
		, Rewards()
		, OptionalRewards()
		, MaxOptionalRewardChoices(1)
		, QuestGiverId()
		, QuestCompleterId()
		, bCanAbandon(true)
		, bAutoComplete(false)
		, bHidden(false)
		, TimeLimit(0.0f)
		, SortOrder(0)
		, RecommendedLevel(1)
		, NextQuests()
	{}
};

/**
 * Active quest progress data
 * Tracks player's progress on an active quest
 */
USTRUCT(BlueprintType)
struct FActiveQuestProgress
{
	GENERATED_BODY()

	// Quest ID
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	FHarmoniaID QuestId = FHarmoniaID();

	// Quest state
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	EQuestState State = EQuestState::InProgress;

	// Objective progress
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TArray<FQuestObjective> ObjectiveProgress;

	// Quest start timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	float StartTime = 0.0f;

	// Elapsed time since quest started
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	float ElapsedTime = 0.0f;

	// Whether this quest is tracked in UI
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	bool bTracked = false;

	FActiveQuestProgress()
		: QuestId()
		, State(EQuestState::InProgress)
		, ObjectiveProgress()
		, StartTime(0.0f)
		, ElapsedTime(0.0f)
		, bTracked(false)
	{}

	// Check if all required objectives are completed
	bool AreObjectivesCompleted() const
	{
		for (const FQuestObjective& Objective : ObjectiveProgress)
		{
			if (!Objective.bOptional && !Objective.IsCompleted())
			{
				return false;
			}
		}
		return true;
	}

	// Get overall quest progress (0.0 - 1.0)
	float GetOverallProgress() const
	{
		if (ObjectiveProgress.Num() == 0) return 0.0f;

		int32 TotalObjectives = 0;
		float TotalProgress = 0.0f;

		for (const FQuestObjective& Objective : ObjectiveProgress)
		{
			if (!Objective.bOptional)
			{
				TotalObjectives++;
				TotalProgress += Objective.GetProgress();
			}
		}

		if (TotalObjectives == 0) return 0.0f;
		return TotalProgress / TotalObjectives;
	}

	// Check if quest has time limit
	bool HasTimeLimit(const FQuestData& QuestData) const
	{
		return QuestData.TimeLimit > 0.0f;
	}

	// Get remaining time
	float GetRemainingTime(const FQuestData& QuestData) const
	{
		if (!HasTimeLimit(QuestData)) return 0.0f;
		return FMath::Max(0.0f, QuestData.TimeLimit - ElapsedTime);
	}

	// Check if time is up
	bool IsTimeUp(const FQuestData& QuestData) const
	{
		return HasTimeLimit(QuestData) && ElapsedTime >= QuestData.TimeLimit;
	}
};

/**
 * Quest save data
 * Used for save/load system
 */
USTRUCT(BlueprintType)
struct FQuestSaveData
{
	GENERATED_BODY()

	// Active quests
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FActiveQuestProgress> ActiveQuests;

	// Completed quest IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FHarmoniaID> CompletedQuests;

	// Failed quest IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FHarmoniaID> FailedQuests;

	FQuestSaveData()
		: ActiveQuests()
		, CompletedQuests()
		, FailedQuests()
	{}
};
