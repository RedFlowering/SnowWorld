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
 * Quest marker type
 * Visual indicators for quest objectives
 */
UENUM(BlueprintType)
enum class EQuestMarkerType : uint8
{
	Objective		UMETA(DisplayName = "Objective"),		// 주요 목표
	Optional		UMETA(DisplayName = "Optional"),		// 선택 목표
	Discover		UMETA(DisplayName = "Discover"),		// 탐험 지점
	Deliver			UMETA(DisplayName = "Deliver"),			// 전달 지점
	Talk			UMETA(DisplayName = "Talk"),			// 대화 NPC
	Gather			UMETA(DisplayName = "Gather"),			// 채집 지점
	Custom			UMETA(DisplayName = "Custom"),			// 커스텀
	MAX				UMETA(Hidden)
};

/**
 * Quest fail condition type
 */
UENUM(BlueprintType)
enum class EQuestFailConditionType : uint8
{
	None			UMETA(DisplayName = "None"),			// 없음
	TimeLimit		UMETA(DisplayName = "Time Limit"),		// 시간 제한
	NPCDied			UMETA(DisplayName = "NPC Died"),		// NPC 사망
	ItemLost		UMETA(DisplayName = "Item Lost"),		// 아이템 손실
	LocationLeft	UMETA(DisplayName = "Location Left"),	// 지역 이탈
	PlayerDied		UMETA(DisplayName = "Player Died"),		// 플레이어 사망
	Custom			UMETA(DisplayName = "Custom"),			// 커스텀 조건
	MAX				UMETA(Hidden)
};

/**
 * Quest event trigger type
 */
UENUM(BlueprintType)
enum class EQuestEventTrigger : uint8
{
	OnStart			UMETA(DisplayName = "On Start"),		// 퀘스트 시작 시
	OnComplete		UMETA(DisplayName = "On Complete"),		// 퀘스트 완료 시
	OnFail			UMETA(DisplayName = "On Fail"),			// 퀘스트 실패 시
	OnAbandon		UMETA(DisplayName = "On Abandon"),		// 퀘스트 포기 시
	OnPhaseChange	UMETA(DisplayName = "On Phase Change"),	// 단계 변경 시
	OnObjectiveComplete UMETA(DisplayName = "On Objective Complete"), // 목표 완료 시
	MAX				UMETA(Hidden)
};

/**
 * Quest notification type
 */
UENUM(BlueprintType)
enum class EQuestNotificationType : uint8
{
	QuestAdded		UMETA(DisplayName = "Quest Added"),			// 퀘스트 추가
	QuestStarted	UMETA(DisplayName = "Quest Started"),		// 퀘스트 시작
	QuestCompleted	UMETA(DisplayName = "Quest Completed"),		// 퀘스트 완료
	QuestFailed		UMETA(DisplayName = "Quest Failed"),		// 퀘스트 실패
	ObjectiveComplete UMETA(DisplayName = "Objective Complete"),// 목표 완료
	PhaseComplete	UMETA(DisplayName = "Phase Complete"),		// 단계 완료
	HintShown		UMETA(DisplayName = "Hint Shown"),			// 힌트 표시
	MAX				UMETA(Hidden)
};

/**
 * Quest event type
 */
UENUM(BlueprintType)
enum class EQuestEventType : uint8
{
	None			UMETA(DisplayName = "None"),			// 없음
	SpawnActor		UMETA(DisplayName = "Spawn Actor"),		// 액터 스폰
	PlayCutscene	UMETA(DisplayName = "Play Cutscene"),	// 컷씬 재생
	GrantReward		UMETA(DisplayName = "Grant Reward"),	// 보상 지급
	ModifyWorld		UMETA(DisplayName = "Modify World"),	// 월드 변경
	StartQuest		UMETA(DisplayName = "Start Quest"),		// 퀘스트 시작
	Custom			UMETA(DisplayName = "Custom"),			// 커스텀
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

	// Whether this is a bonus objective (optional but gives extra rewards)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bBonus = false;

	// Location (for Reach, Defend, Discover objectives)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector TargetLocation = FVector::ZeroVector;

	// Radius for location-based objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0.0"))
	float LocationRadius = 500.0f;

	// Target actor reference (for dynamic targets like NPCs)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	FQuestObjective()
		: ObjectiveType(EQuestObjectiveType::Kill)
		, Description()
		, TargetId()
		, TargetTags()
		, RequiredCount(1)
		, CurrentCount(0)
		, bOptional(false)
		, bHidden(false)
		, bBonus(false)
		, TargetLocation(FVector::ZeroVector)
		, LocationRadius(500.0f)
		, TargetActor(nullptr)
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
 * Bonus objective data
 * Optional objectives that grant bonus rewards when completed
 */
USTRUCT(BlueprintType)
struct FBonusObjective
{
	GENERATED_BODY()

	// Objective description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	// Objective type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestObjectiveType ObjectiveType = EQuestObjectiveType::Kill;

	// Target ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID TargetId;

	// Required count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	// Bonus reward for completing this objective
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FQuestReward BonusReward;

	FBonusObjective()
		: Description()
		, ObjectiveType(EQuestObjectiveType::Kill)
		, TargetId()
		, RequiredCount(1)
		, BonusReward()
	{}
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
 * Quest marker data
 * Visual indicator for quest objectives on map/compass
 */
USTRUCT(BlueprintType)
struct FQuestMarker
{
	GENERATED_BODY()

	// Marker type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestMarkerType MarkerType = EQuestMarkerType::Objective;

	// World location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector WorldLocation = FVector::ZeroVector;

	// Target actor (dynamic marker that follows actor)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	// Whether to show distance to marker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bShowDistance = true;

	// Marker icon (optional, can be set by marker type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TSoftObjectPtr<UTexture2D> MarkerIcon = nullptr;

	// Marker color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FLinearColor MarkerColor = FLinearColor::Yellow;

	FQuestMarker()
		: MarkerType(EQuestMarkerType::Objective)
		, WorldLocation(FVector::ZeroVector)
		, TargetActor(nullptr)
		, bShowDistance(true)
		, MarkerIcon(nullptr)
		, MarkerColor(FLinearColor::Yellow)
	{}
};

/**
 * Quest dialogue data
 * Dialogue lines for quest NPCs
 */
USTRUCT(BlueprintType)
struct FQuestDialogue
{
	GENERATED_BODY()

	// Dialogue when accepting quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FText> StartDialogues;

	// Dialogue when quest is in progress (talking to quest giver again)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FText> InProgressDialogues;

	// Dialogue when ready to complete quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FText> ReadyToCompleteDialogues;

	// Dialogue when completing quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FText> CompletionDialogues;

	// Dialogue when quest failed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FText> FailureDialogues;

	FQuestDialogue()
		: StartDialogues()
		, InProgressDialogues()
		, ReadyToCompleteDialogues()
		, CompletionDialogues()
		, FailureDialogues()
	{}
};

/**
 * Quest hint data
 * Hints shown to player when stuck
 */
USTRUCT(BlueprintType)
struct FQuestHint
{
	GENERATED_BODY()

	// Hint text
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText HintText = FText();

	// Delay before showing hint (seconds since quest start)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0.0"))
	float ShowAfterSeconds = 30.0f;

	// Show hint if player is stuck (no progress)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bShowIfStuck = true;

	// Optional hint location (to show marker)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector HintLocation = FVector::ZeroVector;

	// Whether hint has a location marker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bHasLocation = false;

	// Whether hint was shown
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	bool bShown = false;

	FQuestHint()
		: HintText()
		, ShowAfterSeconds(30.0f)
		, bShowIfStuck(true)
		, HintLocation(FVector::ZeroVector)
		, bHasLocation(false)
		, bShown(false)
	{}
};

/**
 * Dynamic objective count
 * Allows objective count to scale with player level or party size
 */
USTRUCT(BlueprintType)
struct FDynamicObjectiveCount
{
	GENERATED_BODY()

	// Base count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 BaseCount = 10;

	// Additional count per player level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	float CountPerLevel = 0.0f;

	// Additional count per party member
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	float CountPerPartyMember = 0.0f;

	// Maximum count (0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0"))
	int32 MaxCount = 0;

	FDynamicObjectiveCount()
		: BaseCount(10)
		, CountPerLevel(0.0f)
		, CountPerPartyMember(0.0f)
		, MaxCount(0)
	{}

	// Calculate actual count
	int32 CalculateCount(int32 PlayerLevel, int32 PartySize) const
	{
		float Total = BaseCount;
		Total += CountPerLevel * PlayerLevel;
		Total += CountPerPartyMember * FMath::Max(0, PartySize - 1); // Don't count self

		int32 Result = FMath::RoundToInt(Total);

		if (MaxCount > 0)
		{
			Result = FMath::Min(Result, MaxCount);
		}

		return FMath::Max(1, Result);
	}
};

/**
 * Quest fail condition
 * Conditions that cause quest to fail
 */
USTRUCT(BlueprintType)
struct FQuestFailCondition
{
	GENERATED_BODY()

	// Fail condition type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestFailConditionType ConditionType = EQuestFailConditionType::None;

	// Target ID (NPC ID for NPCDied, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID TargetId = FHarmoniaID();

	// Required item ID (for ItemLost condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID RequiredItemId = FHarmoniaID();

	// Failure message shown to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText FailureMessage = FText();

	// Required location (for LocationLeft condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector RequiredLocation = FVector::ZeroVector;

	// Maximum distance from location (for LocationLeft condition)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "0.0"))
	float MaxDistance = 1000.0f;

	FQuestFailCondition()
		: ConditionType(EQuestFailConditionType::None)
		, TargetId()
		, RequiredItemId()
		, FailureMessage()
		, RequiredLocation(FVector::ZeroVector)
		, MaxDistance(1000.0f)
	{}
};

/**
 * Quest event
 * Events triggered by quest state changes
 */
USTRUCT(BlueprintType)
struct FQuestEvent
{
	GENERATED_BODY()

	// When to trigger event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestEventTrigger TriggerType = EQuestEventTrigger::OnStart;

	// Event type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EQuestEventType EventType = EQuestEventType::None;

	// Actor class to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TSubclassOf<AActor> ActorToSpawn = nullptr;

	// Spawn location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FVector SpawnLocation = FVector::ZeroVector;

	// Event tags to broadcast
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer EventTags;

	// Whether to broadcast to all players (multiplayer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bBroadcastToWorld = false;

	// Custom event name (for blueprint events)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName CustomEventName = NAME_None;

	// Bonus reward (for GrantReward event type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FQuestReward BonusReward;

	// Target quest ID (for StartQuest event type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FHarmoniaID TargetQuestId;

	FQuestEvent()
		: TriggerType(EQuestEventTrigger::OnStart)
		, EventType(EQuestEventType::None)
		, ActorToSpawn(nullptr)
		, SpawnLocation(FVector::ZeroVector)
		, EventTags()
		, bBroadcastToWorld(false)
		, CustomEventName(NAME_None)
		, BonusReward()
		, TargetQuestId()
	{}
};

/**
 * Quest notification
 * UI notification for quest events
 */
USTRUCT(BlueprintType)
struct FQuestNotification
{
	GENERATED_BODY()

	// Notification type
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	EQuestNotificationType NotificationType = EQuestNotificationType::QuestStarted;

	// Quest ID
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FHarmoniaID QuestId = FHarmoniaID();

	// Quest name
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FText QuestName = FText();

	// Notification message
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FText Message = FText();

	// Notification icon
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TSoftObjectPtr<UTexture2D> Icon = nullptr;

	// Display duration (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float DisplayDuration = 5.0f;

	// Timestamp when notification was created
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FDateTime Timestamp;

	// Whether notification has been shown
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	bool bShown = false;

	FQuestNotification()
		: NotificationType(EQuestNotificationType::QuestStarted)
		, QuestId()
		, QuestName()
		, Message()
		, Icon(nullptr)
		, DisplayDuration(5.0f)
		, Timestamp(FDateTime::Now())
		, bShown(false)
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
 * Quest phase data
 * Multi-phase quest support
 */
USTRUCT(BlueprintType)
struct FQuestPhase
{
	GENERATED_BODY()

	// Phase number (0-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 PhaseNumber = 0;

	// Phase name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText PhaseName = FText();

	// Phase description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText PhaseDescription = FText();

	// Phase objectives
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestObjective> PhaseObjectives;

	// Phase markers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestMarker> PhaseMarkers;

	// Phase events (triggered when phase starts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestEvent> PhaseEvents;

	// Auto-advance to next phase when objectives complete
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bAutoAdvance = true;

	FQuestPhase()
		: PhaseNumber(0)
		, PhaseName()
		, PhaseDescription()
		, PhaseObjectives()
		, PhaseMarkers()
		, PhaseEvents()
		, bAutoAdvance(true)
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

	// Quest objectives (used if not using phases)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	TArray<FQuestObjective> Objectives;

	// Quest phases (multi-phase quest system)
	// If phases are defined, objectives from phases will be used instead
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Phases")
	TArray<FQuestPhase> Phases;

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

	// Quest dialogue
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Dialogue")
	FQuestDialogue Dialogues;

	// Quest markers
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Markers")
	TArray<FQuestMarker> Markers;

	// Quest hints
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Hints")
	TArray<FQuestHint> Hints;

	// Quest events
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Events")
	TArray<FQuestEvent> Events;

	// Quest fail conditions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|FailConditions")
	TArray<FQuestFailCondition> FailConditions;

	// Bonus objectives (optional objectives for extra rewards)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Objectives")
	TArray<FBonusObjective> BonusObjectives;

	// Bonus rewards (for completing bonus objectives)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Rewards")
	TArray<FQuestReward> BonusRewards;

	// Quest priority (higher = shown first in UI)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|UI")
	int32 Priority = 0;

	// Whether this quest can be shared with party members
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Party")
	bool bCanShare = true;

	// Whether quest progress is shared among party members
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Party")
	bool bSharedProgress = false;

	// Minimum party size required
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Party", meta = (ClampMin = "1"))
	int32 MinPartySize = 1;

	// Maximum party size allowed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Party", meta = (ClampMin = "1"))
	int32 MaxPartySize = 4;

	FQuestData()
		: QuestId()
		, QuestType(EQuestType::Side)
		, QuestName()
		, QuestDescription()
		, QuestIcon(nullptr)
		, CategoryTags()
		, Objectives()
		, Phases()
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
		, Dialogues()
		, Markers()
		, Hints()
		, Events()
		, FailConditions()
		, BonusObjectives()
		, BonusRewards()
		, Priority(0)
		, bCanShare(true)
		, bSharedProgress(false)
		, MinPartySize(1)
		, MaxPartySize(4)
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

	// Current phase number (for phased quests, 0-based)
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	int32 CurrentPhase = 0;

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
		, CurrentPhase(0)
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
 * Quest statistics
 * Player quest completion statistics
 */
USTRUCT(BlueprintType)
struct FQuestStatistics
{
	GENERATED_BODY()

	// Total quests completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 TotalQuestsCompleted = 0;

	// Main quests completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 MainQuestsCompleted = 0;

	// Side quests completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 SideQuestsCompleted = 0;

	// Daily quests completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 DailyQuestsCompleted = 0;

	// Weekly quests completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 WeeklyQuestsCompleted = 0;

	// Total quests failed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 TotalQuestsFailed = 0;

	// Total quests abandoned
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 TotalQuestsAbandoned = 0;

	// Average completion time (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float AverageCompletionTime = 0.0f;

	// Total objectives completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 TotalObjectivesCompleted = 0;

	// Completion count by quest type
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TMap<EQuestType, int32> CompletedByType;

	// Completion count by category
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TMap<FGameplayTag, int32> CompletedByCategory;

	// Longest quest completed (time)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FHarmoniaID LongestQuestCompleted;

	// Longest quest time (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float LongestQuestTime = 0.0f;

	// Fastest completion times per quest
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	TMap<FHarmoniaID, float> FastestCompletionTimes;

	// Current quest streak (consecutive days with quest completion)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 CurrentStreak = 0;

	// Best quest streak
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 BestStreak = 0;

	// Last completion date
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FDateTime LastCompletionDate;

	FQuestStatistics()
		: TotalQuestsCompleted(0)
		, MainQuestsCompleted(0)
		, SideQuestsCompleted(0)
		, DailyQuestsCompleted(0)
		, WeeklyQuestsCompleted(0)
		, TotalQuestsFailed(0)
		, TotalQuestsAbandoned(0)
		, AverageCompletionTime(0.0f)
		, TotalObjectivesCompleted(0)
		, CompletedByType()
		, CompletedByCategory()
		, LongestQuestCompleted()
		, LongestQuestTime(0.0f)
		, CurrentStreak(0)
		, BestStreak(0)
		, LastCompletionDate(FDateTime::MinValue())
	{}
};

/**
 * Quest log entry
 * Journal/log entry for a quest
 */
USTRUCT(BlueprintType)
struct FQuestLogEntry
{
	GENERATED_BODY()

	// Quest ID
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FHarmoniaID QuestId;

	// Quest name
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FText QuestName;

	// Quest priority
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 Priority = 0;

	// Whether quest is completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	bool bCompleted = false;

	// When quest was started
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FDateTime StartTime;

	// When quest was completed
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FDateTime CompletionTime;

	// Last updated time
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FDateTime LastUpdated;

	// Total time taken (seconds)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float TotalTime = 0.0f;

	// Player notes
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	FString PlayerNotes;

	// Completion count (for repeatable quests)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	int32 CompletionCount = 0;

	// Best completion time (for repeatable quests)
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	float BestCompletionTime = 0.0f;

	// Whether this entry is favorited
	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	bool bFavorited = false;

	FQuestLogEntry()
		: QuestId()
		, StartTime(FDateTime::MinValue())
		, CompletionTime(FDateTime::MinValue())
		, TotalTime(0.0f)
		, PlayerNotes()
		, CompletionCount(0)
		, BestCompletionTime(0.0f)
		, bFavorited(false)
	{}
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

	// Quest statistics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FQuestStatistics Statistics;

	// Quest log entries
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestLogEntry> LogEntries;

	FQuestSaveData()
		: ActiveQuests()
		, CompletedQuests()
		, FailedQuests()
		, Statistics()
		, LogEntries()
	{}
};
