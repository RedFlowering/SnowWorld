// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaProgressionDefinitions.generated.h"

class UGameplayEffect;
class ULyraAbilitySet;
class UTexture2D;

/**
 * 스킬 트리 노드 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillNodeType : uint8
{
	// 패시브 스킬 (stat bonuses)
	Passive UMETA(DisplayName = "Passive Skill"),

	// 액티브 스킬 (unlocks abilities)
	Active UMETA(DisplayName = "Active Skill"),

	// 속성 증가 (primary stats: Str, Dex, etc)
	AttributeBoost UMETA(DisplayName = "Attribute Boost"),

	// 특수 노드 (클래스 전환, 각성 등)
	Special UMETA(DisplayName = "Special Node")
};

/**
 * 스킬 트리 카테고리
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillTreeCategory : uint8
{
	// 무기별 전문화
	SwordMastery UMETA(DisplayName = "Sword Mastery"),
	AxeMastery UMETA(DisplayName = "Axe Mastery"),
	SpearMastery UMETA(DisplayName = "Spear Mastery"),
	BowMastery UMETA(DisplayName = "Bow Mastery"),
	MagicMastery UMETA(DisplayName = "Magic Mastery"),

	// 스타일별 전문화
	OffensiveCombat UMETA(DisplayName = "Offensive Combat"),
	DefensiveCombat UMETA(DisplayName = "Defensive Combat"),
	StealthTactics UMETA(DisplayName = "Stealth Tactics"),

	// 기타
	Survival UMETA(DisplayName = "Survival"),
	Crafting UMETA(DisplayName = "Crafting"),
	Social UMETA(DisplayName = "Social")
};

/**
 * 캐릭터 클래스 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaCharacterClass : uint8
{
	// 기본 클래스
	None UMETA(DisplayName = "No Class"),

	// Tier 1 클래스
	Warrior UMETA(DisplayName = "Warrior"),
	Rogue UMETA(DisplayName = "Rogue"),
	Mage UMETA(DisplayName = "Mage"),

	// Tier 2 클래스 (전사 계열)
	Berserker UMETA(DisplayName = "Berserker"),
	Knight UMETA(DisplayName = "Knight"),
	Paladin UMETA(DisplayName = "Paladin"),

	// Tier 2 클래스 (도적 계열)
	Assassin UMETA(DisplayName = "Assassin"),
	Ranger UMETA(DisplayName = "Ranger"),
	Duelist UMETA(DisplayName = "Duelist"),

	// Tier 2 클래스 (마법사 계열)
	Sorcerer UMETA(DisplayName = "Sorcerer"),
	Cleric UMETA(DisplayName = "Cleric"),
	Warlock UMETA(DisplayName = "Warlock")
};

/**
 * 각성 단계
 */
UENUM(BlueprintType)
enum class EHarmoniaAwakeningTier : uint8
{
	Base UMETA(DisplayName = "Base"),
	FirstAwakening UMETA(DisplayName = "First Awakening"),
	SecondAwakening UMETA(DisplayName = "Second Awakening"),
	ThirdAwakening UMETA(DisplayName = "Third Awakening"),
	Transcendent UMETA(DisplayName = "Transcendent")
};

/**
 * 스킬 노드 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaSkillNode
{
	GENERATED_BODY()

	/** 노드 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FName NodeID;

	/** 노드 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeName;

	/** 노드 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeDescription;

	/** 노드 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	TObjectPtr<UTexture2D> NodeIcon;

	/** 노드 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillNodeType NodeType = EHarmoniaSkillNodeType::Passive;

	/** 스킬 트리 카테고리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillTreeCategory Category = EHarmoniaSkillTreeCategory::SwordMastery;

	/** 필요 스킬 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredSkillPoints = 1;

	/** 필요 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 1;

	/** 선행 노드 ID들 (이 노드들이 먼저 해금되어야 함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> PrerequisiteNodeIDs;

	/** 필요 클래스 (None이면 제한 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** 해금 시 부여할 GameplayEffect (stat bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** 해금 시 부여할 AbilitySet (abilities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<ULyraAbilitySet> GrantedAbilitySet;

	/** 최대 투자 가능 포인트 (레벨업 가능한 스킬) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	int32 MaxInvestmentPoints = 1;

	/** UI 위치 (스킬 트리 그래프에서의 좌표) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector2D UIPosition = FVector2D::ZeroVector;

	FHarmoniaSkillNode()
		: NodeID(NAME_None)
		, NodeType(EHarmoniaSkillNodeType::Passive)
		, Category(EHarmoniaSkillTreeCategory::SwordMastery)
		, RequiredSkillPoints(1)
		, RequiredLevel(1)
		, RequiredClass(EHarmoniaCharacterClass::None)
		, MaxInvestmentPoints(1)
		, UIPosition(FVector2D::ZeroVector)
	{
	}
};

/**
 * 클래스 정의 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaClassDefinition
{
	GENERATED_BODY()

	/** 클래스 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass ClassType = EHarmoniaCharacterClass::None;

	/** 클래스 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassName;

	/** 클래스 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassDescription;

	/** 클래스 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TObjectPtr<UTexture2D> ClassIcon;

	/** 시작 스탯 보너스 (Vitality, Strength, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Stats")
	TMap<FGameplayTag, float> StartingStatBonuses;

	/** 레벨업 시 자동 스탯 증가 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TMap<FGameplayTag, float> StatGrowthPerLevel;

	/** 클래스 선택 시 부여되는 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> ClassEffects;

	/** 클래스 선택 시 부여되는 AbilitySet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TObjectPtr<ULyraAbilitySet> ClassAbilitySet;

	/** 이 클래스로 전직 가능한 상위 클래스들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	TArray<EHarmoniaCharacterClass> AdvancementClasses;

	/** 전직 필요 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	int32 RequiredLevelForAdvancement = 20;

	FHarmoniaClassDefinition()
		: ClassType(EHarmoniaCharacterClass::None)
		, RequiredLevelForAdvancement(20)
	{
	}
};

/**
 * 각성 단계 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaAwakeningDefinition
{
	GENERATED_BODY()

	/** 각성 단계 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier Tier = EHarmoniaAwakeningTier::Base;

	/** 각성 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningName;

	/** 각성 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningDescription;

	/** 필요 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 50;

	/** 각성 퀘스트 태그 (완료해야 할 퀘스트) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag RequiredQuestTag;

	/** 각성 시 부여되는 스탯 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	float StatMultiplier = 1.2f;

	/** 각성 시 부여되는 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TArray<TSubclassOf<UGameplayEffect>> AwakeningEffects;

	/** 각성 시 부여되는 AbilitySet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TObjectPtr<ULyraAbilitySet> AwakeningAbilitySet;

	/** 각성 보상 스킬 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	int32 BonusSkillPoints = 5;

	FHarmoniaAwakeningDefinition()
		: Tier(EHarmoniaAwakeningTier::Base)
		, RequiredLevel(50)
		, StatMultiplier(1.2f)
		, BonusSkillPoints(5)
	{
	}
};

/**
 * 뉴게임+ 프레스티지 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaPrestigeDefinition
{
	GENERATED_BODY()

	/** 프레스티지 레벨 (0 = 기본, 1+ = NG+) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** 필요 최소 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPlayerLevel = 100;

	/** 적 난이도 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float EnemyDifficultyMultiplier = 1.5f;

	/** 보상 배율 (경험치, 골드, 드랍) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float RewardMultiplier = 2.0f;

	/** 프레스티지 보너스 스탯 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusStatPoints = 10;

	/** 프레스티지 보너스 스킬 포인트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusSkillPoints = 10;

	/** 프레스티지 전용 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	TArray<TSubclassOf<UGameplayEffect>> PrestigeEffects;

	/** 프레스티지 타이틀/뱃지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	FText PrestigeTitle;

	FHarmoniaPrestigeDefinition()
		: PrestigeLevel(0)
		, RequiredPlayerLevel(100)
		, EnemyDifficultyMultiplier(1.5f)
		, RewardMultiplier(2.0f)
		, BonusStatPoints(10)
		, BonusSkillPoints(10)
	{
	}
};

/**
 * 플레이어 진행 상황 저장 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaProgressionSaveData
{
	GENERATED_BODY()

	/** 현재 경험치 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentExperience = 0;

	/** 현재 레벨 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentLevel = 1;

	/** 사용 가능한 스킬 포인트 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableSkillPoints = 0;

	/** 사용 가능한 스탯 포인트 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableStatPoints = 0;

	/** 현재 클래스 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass CurrentClass = EHarmoniaCharacterClass::None;

	/** 현재 각성 단계 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier CurrentAwakeningTier = EHarmoniaAwakeningTier::Base;

	/** 프레스티지 레벨 (NG+ 횟수) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** 해금된 스킬 노드 ID들과 투자 포인트 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Skills")
	TMap<FName, int32> UnlockedSkillNodes;

	/** 배분된 주요 스탯 (Vitality, Strength, etc) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	TMap<FGameplayTag, int32> AllocatedStats;

	/** 총 획득 경험치 (lifetime) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Statistics")
	int64 TotalExperienceEarned = 0;

	/** 총 레벨업 횟수 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Statistics")
	int32 TotalLevelsGained = 0;

	FHarmoniaProgressionSaveData()
		: CurrentExperience(0)
		, CurrentLevel(1)
		, AvailableSkillPoints(0)
		, AvailableStatPoints(0)
		, CurrentClass(EHarmoniaCharacterClass::None)
		, CurrentAwakeningTier(EHarmoniaAwakeningTier::Base)
		, PrestigeLevel(0)
		, TotalExperienceEarned(0)
		, TotalLevelsGained(0)
	{
	}
};
