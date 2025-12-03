// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaProgressionDefinitions.generated.h"

class UGameplayEffect;
class ULyraAbilitySet;
class UTexture2D;

/**
 * ?�킬 ?�리 ?�드 ?�??
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillNodeType : uint8
{
	// ?�시�??�킬 (stat bonuses)
	Passive UMETA(DisplayName = "Passive Skill"),

	// ?�티�??�킬 (unlocks abilities)
	Active UMETA(DisplayName = "Active Skill"),

	// ?�성 증�? (primary stats: Str, Dex, etc)
	AttributeBoost UMETA(DisplayName = "Attribute Boost"),

	// ?�수 ?�드 (?�래???�환, 각성 ??
	Special UMETA(DisplayName = "Special Node")
};

/**
 * ?�킬 ?�리 카테고리
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillTreeCategory : uint8
{
	// 무기�??�문??
	SwordMastery UMETA(DisplayName = "Sword Mastery"),
	AxeMastery UMETA(DisplayName = "Axe Mastery"),
	SpearMastery UMETA(DisplayName = "Spear Mastery"),
	BowMastery UMETA(DisplayName = "Bow Mastery"),
	MagicMastery UMETA(DisplayName = "Magic Mastery"),

	// ?��??�별 ?�문??
	OffensiveCombat UMETA(DisplayName = "Offensive Combat"),
	DefensiveCombat UMETA(DisplayName = "Defensive Combat"),
	StealthTactics UMETA(DisplayName = "Stealth Tactics"),

	// 기�?
	Survival UMETA(DisplayName = "Survival"),
	Crafting UMETA(DisplayName = "Crafting"),
	Social UMETA(DisplayName = "Social")
};

/**
 * 캐릭???�래???�??
 */
UENUM(BlueprintType)
enum class EHarmoniaCharacterClass : uint8
{
	// 기본 ?�래??
	None UMETA(DisplayName = "No Class"),

	// Tier 1 ?�래??
	Warrior UMETA(DisplayName = "Warrior"),
	Rogue UMETA(DisplayName = "Rogue"),
	Mage UMETA(DisplayName = "Mage"),

	// Tier 2 ?�래??(?�사 계열)
	Berserker UMETA(DisplayName = "Berserker"),
	Knight UMETA(DisplayName = "Knight"),
	Paladin UMETA(DisplayName = "Paladin"),

	// Tier 2 ?�래??(?�적 계열)
	Assassin UMETA(DisplayName = "Assassin"),
	Ranger UMETA(DisplayName = "Ranger"),
	Duelist UMETA(DisplayName = "Duelist"),

	// Tier 2 ?�래??(마법??계열)
	Sorcerer UMETA(DisplayName = "Sorcerer"),
	Cleric UMETA(DisplayName = "Cleric"),
	Warlock UMETA(DisplayName = "Warlock")
};

/**
 * 각성 ?�계
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
 * ?�킬 ?�드 ?�이??
 */
USTRUCT(BlueprintType)
struct FHarmoniaSkillNode
{
	GENERATED_BODY()

	/** ?�드 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FName NodeID;

	/** ?�드 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeName;

	/** ?�드 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeDescription;

	/** ?�드 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	TObjectPtr<UTexture2D> NodeIcon;

	/** ?�드 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillNodeType NodeType = EHarmoniaSkillNodeType::Passive;

	/** ?�킬 ?�리 카테고리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillTreeCategory Category = EHarmoniaSkillTreeCategory::SwordMastery;

	/** ?�요 ?�킬 ?�인??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredSkillPoints = 1;

	/** ?�요 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 1;

	/** ?�행 ?�드 ID??(???�드?�이 먼�? ?�금?�어???? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> PrerequisiteNodeIDs;

	/** ?�요 ?�래??(None?�면 ?�한 ?�음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** ?�금 ??부?�할 GameplayEffect (stat bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** ?�금 ??부?�할 AbilitySet (abilities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<ULyraAbilitySet> GrantedAbilitySet;

	/** 최�? ?�자 가???�인??(?�벨??가?�한 ?�킬) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	int32 MaxInvestmentPoints = 1;

	/** UI ?�치 (?�킬 ?�리 그래?�에?�의 좌표) */
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
 * ?�래???�의 ?�이??
 */
USTRUCT(BlueprintType)
struct FHarmoniaClassDefinition
{
	GENERATED_BODY()

	/** ?�래???�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass ClassType = EHarmoniaCharacterClass::None;

	/** ?�래???�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassName;

	/** ?�래???�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassDescription;

	/** ?�래???�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TObjectPtr<UTexture2D> ClassIcon;

	/** ?�작 ?�탯 보너??(Vitality, Strength, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Stats")
	TMap<FGameplayTag, float> StartingStatBonuses;

	/** ?�벨?????�동 ?�탯 증�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TMap<FGameplayTag, float> StatGrowthPerLevel;

	/** ?�래???�택 ??부?�되??GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> ClassEffects;

	/** ?�래???�택 ??부?�되??AbilitySet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TObjectPtr<ULyraAbilitySet> ClassAbilitySet;

	/** ???�래?�로 ?�직 가?�한 ?�위 ?�래?�들 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	TArray<EHarmoniaCharacterClass> AdvancementClasses;

	/** ?�직 ?�요 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	int32 RequiredLevelForAdvancement = 20;

	FHarmoniaClassDefinition()
		: ClassType(EHarmoniaCharacterClass::None)
		, RequiredLevelForAdvancement(20)
	{
	}
};

/**
 * 각성 ?�계 ?�이??
 */
USTRUCT(BlueprintType)
struct FHarmoniaAwakeningDefinition
{
	GENERATED_BODY()

	/** 각성 ?�계 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier Tier = EHarmoniaAwakeningTier::Base;

	/** 각성 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningName;

	/** 각성 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningDescription;

	/** ?�요 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 50;

	/** 각성 ?�스???�그 (?�료?�야 ???�스?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag RequiredQuestTag;

	/** 각성 ??부?�되???�탯 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	float StatMultiplier = 1.2f;

	/** 각성 ??부?�되??GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TArray<TSubclassOf<UGameplayEffect>> AwakeningEffects;

	/** 각성 ??부?�되??AbilitySet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TObjectPtr<ULyraAbilitySet> AwakeningAbilitySet;

	/** 각성 보상 ?�킬 ?�인??*/
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
 * ?�게?? ?�레?�티지 ?�이??
 */
USTRUCT(BlueprintType)
struct FHarmoniaPrestigeDefinition
{
	GENERATED_BODY()

	/** ?�레?�티지 ?�벨 (0 = 기본, 1+ = NG+) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** ?�요 최소 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPlayerLevel = 100;

	/** ???�이??배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float EnemyDifficultyMultiplier = 1.5f;

	/** 보상 배율 (경험�? 골드, ?�랍) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float RewardMultiplier = 2.0f;

	/** ?�레?�티지 보너???�탯 ?�인??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusStatPoints = 10;

	/** ?�레?�티지 보너???�킬 ?�인??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusSkillPoints = 10;

	/** ?�레?�티지 ?�용 GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	TArray<TSubclassOf<UGameplayEffect>> PrestigeEffects;

	/** ?�레?�티지 ?�?��?/뱃�? */
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
 * ?�킬 ?�드 ?�자 ?�보 (?�트?�크 복제??
 */
USTRUCT(BlueprintType)
struct FSkillNodeInvestment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	FName NodeID;

	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	int32 InvestedPoints = 0;

	FSkillNodeInvestment()
		: NodeID(NAME_None)
		, InvestedPoints(0)
	{
	}

	FSkillNodeInvestment(FName InNodeID, int32 InPoints)
		: NodeID(InNodeID)
		, InvestedPoints(InPoints)
	{
	}
};

/**
 * ?�탯 배분 ?�보 (?�트?�크 복제??
 */
USTRUCT(BlueprintType)
struct FStatAllocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Stat")
	FGameplayTag StatTag;

	UPROPERTY(BlueprintReadWrite, Category = "Stat")
	int32 AllocatedPoints = 0;

	FStatAllocation()
		: AllocatedPoints(0)
	{
	}

	FStatAllocation(FGameplayTag InStatTag, int32 InPoints)
		: StatTag(InStatTag)
		, AllocatedPoints(InPoints)
	{
	}
};

/**
 * ?�레?�어 진행 ?�황 ?�???�이??
 */
USTRUCT(BlueprintType)
struct FHarmoniaProgressionSaveData
{
	GENERATED_BODY()

	/** ?�재 경험�?*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentExperience = 0;

	/** ?�재 ?�벨 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentLevel = 1;

	/** ?�용 가?�한 ?�킬 ?�인??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableSkillPoints = 0;

	/** ?�용 가?�한 ?�탯 ?�인??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableStatPoints = 0;

	/** ?�재 ?�래??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass CurrentClass = EHarmoniaCharacterClass::None;

	/** ?�재 각성 ?�계 */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier CurrentAwakeningTier = EHarmoniaAwakeningTier::Base;

	/** ?�레?�티지 ?�벨 (NG+ ?�수) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** ?�금???�킬 ?�드 ID?�과 ?�자 ?�인??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Skills")
	TArray<FSkillNodeInvestment> UnlockedSkillNodes;

	/** 배분??주요 ?�탯 (Vitality, Strength, etc) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	TArray<FStatAllocation> AllocatedStats;

	/** �??�득 경험�?(lifetime) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Statistics")
	int64 TotalExperienceEarned = 0;

	/** �??�벨???�수 */
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
