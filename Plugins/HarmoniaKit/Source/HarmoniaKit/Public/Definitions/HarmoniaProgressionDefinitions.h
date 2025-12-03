// Copyright 2025 Snow Game Studio.

#pragma once

/**
 * @file HarmoniaProgressionDefinitions.h
 * @brief Character progression system definitions
 * @author Harmonia Team
 * 
 * Defines skill trees, character classes, awakening tiers,
 * and prestige systems for character advancement.
 */

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaProgressionDefinitions.generated.h"

class UGameplayEffect;
class ULyraAbilitySet;
class UTexture2D;

/**
 * @enum EHarmoniaSkillNodeType
 * @brief Skill tree node types
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillNodeType : uint8
{
	/** Passive skill (stat bonuses) */
	Passive UMETA(DisplayName = "Passive Skill"),

	/** Active skill (unlocks abilities) */
	Active UMETA(DisplayName = "Active Skill"),

	/** Attribute boost (primary stats: Str, Dex, etc) */
	AttributeBoost UMETA(DisplayName = "Attribute Boost"),

	/** Special node (class change, awakening, etc.) */
	Special UMETA(DisplayName = "Special Node")
};

/**
 * @enum EHarmoniaSkillTreeCategory
 * @brief Skill tree categories
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillTreeCategory : uint8
{
	/** Weapon specializations */
	SwordMastery UMETA(DisplayName = "Sword Mastery"),
	AxeMastery UMETA(DisplayName = "Axe Mastery"),
	SpearMastery UMETA(DisplayName = "Spear Mastery"),
	BowMastery UMETA(DisplayName = "Bow Mastery"),
	MagicMastery UMETA(DisplayName = "Magic Mastery"),

	/** Combat style specializations */
	OffensiveCombat UMETA(DisplayName = "Offensive Combat"),
	DefensiveCombat UMETA(DisplayName = "Defensive Combat"),
	StealthTactics UMETA(DisplayName = "Stealth Tactics"),

	/** Utility */
	Survival UMETA(DisplayName = "Survival"),
	Crafting UMETA(DisplayName = "Crafting"),
	Social UMETA(DisplayName = "Social")
};

/**
 * @enum EHarmoniaCharacterClass
 * @brief Character class types
 */
UENUM(BlueprintType)
enum class EHarmoniaCharacterClass : uint8
{
	/** Base class */
	None UMETA(DisplayName = "No Class"),

	/** Tier 1 classes */
	Warrior UMETA(DisplayName = "Warrior"),
	Rogue UMETA(DisplayName = "Rogue"),
	Mage UMETA(DisplayName = "Mage"),

	/** Tier 2 classes (Warrior branch) */
	Berserker UMETA(DisplayName = "Berserker"),
	Knight UMETA(DisplayName = "Knight"),
	Paladin UMETA(DisplayName = "Paladin"),

	/** Tier 2 classes (Rogue branch) */
	Assassin UMETA(DisplayName = "Assassin"),
	Ranger UMETA(DisplayName = "Ranger"),
	Duelist UMETA(DisplayName = "Duelist"),

	/** Tier 2 classes (Mage branch) */
	Sorcerer UMETA(DisplayName = "Sorcerer"),
	Cleric UMETA(DisplayName = "Cleric"),
	Warlock UMETA(DisplayName = "Warlock")
};

/**
 * @enum EHarmoniaAwakeningTier
 * @brief Awakening tiers
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
 * @struct FHarmoniaSkillNode
 * @brief Skill node data structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaSkillNode
{
	GENERATED_BODY()

	/** Node unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FName NodeID;

	/** Node name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeName;

	/** Node description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeDescription;

	/** Node icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	TObjectPtr<UTexture2D> NodeIcon;

	/** Node type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillNodeType NodeType = EHarmoniaSkillNodeType::Passive;

	/** Skill tree category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillTreeCategory Category = EHarmoniaSkillTreeCategory::SwordMastery;

	/** Required skill points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredSkillPoints = 1;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 1;

	/** Prerequisite node IDs (these nodes must be unlocked first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> PrerequisiteNodeIDs;

	/** Required class (None means no restriction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** Granted gameplay effects on unlock (stat bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** Granted ability set on unlock (abilities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<ULyraAbilitySet> GrantedAbilitySet;

	/** Maximum investment points (for upgradeable skills) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	int32 MaxInvestmentPoints = 1;

	/** UI position (coordinates in skill tree graph) */
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
 * @struct FHarmoniaClassDefinition
 * @brief Class definition data structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaClassDefinition
{
	GENERATED_BODY()

	/** Class type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass ClassType = EHarmoniaCharacterClass::None;

	/** Class name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassName;

	/** Class description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassDescription;

	/** Class icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TObjectPtr<UTexture2D> ClassIcon;

	/** Starting stat bonuses (Vitality, Strength, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Stats")
	TMap<FGameplayTag, float> StartingStatBonuses;

	/** Auto stat growth per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TMap<FGameplayTag, float> StatGrowthPerLevel;

	/** Granted gameplay effects on class selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> ClassEffects;

	/** Granted ability set on class selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TObjectPtr<ULyraAbilitySet> ClassAbilitySet;

	/** Available advancement classes from this class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	TArray<EHarmoniaCharacterClass> AdvancementClasses;

	/** Required level for advancement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	int32 RequiredLevelForAdvancement = 20;

	FHarmoniaClassDefinition()
		: ClassType(EHarmoniaCharacterClass::None)
		, RequiredLevelForAdvancement(20)
	{
	}
};

/**
 * @struct FHarmoniaAwakeningDefinition
 * @brief Awakening tier definition data structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaAwakeningDefinition
{
	GENERATED_BODY()

	/** Awakening tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier Tier = EHarmoniaAwakeningTier::Base;

	/** Awakening name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningName;

	/** Awakening description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningDescription;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 50;

	/** Required quest tag (quest that must be completed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag RequiredQuestTag;

	/** Stat multiplier on awakening */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	float StatMultiplier = 1.2f;

	/** Granted gameplay effects on awakening */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TArray<TSubclassOf<UGameplayEffect>> AwakeningEffects;

	/** Granted ability set on awakening */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TObjectPtr<ULyraAbilitySet> AwakeningAbilitySet;

	/** Bonus skill points on awakening */
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
 * @struct FHarmoniaPrestigeDefinition
 * @brief New Game+ prestige data structure
 */
USTRUCT(BlueprintType)
struct FHarmoniaPrestigeDefinition
{
	GENERATED_BODY()

	/** Prestige level (0 = base, 1+ = NG+) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** Required minimum player level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPlayerLevel = 100;

	/** Enemy difficulty multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float EnemyDifficultyMultiplier = 1.5f;

	/** Reward multiplier (experience, gold, drops) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float RewardMultiplier = 2.0f;

	/** Prestige bonus stat points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusStatPoints = 10;

	/** Prestige bonus skill points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusSkillPoints = 10;

/** Prestige gameplay effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	TArray<TSubclassOf<UGameplayEffect>> PrestigeEffects;

	/** Prestige title/badge */
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
 * @struct FSkillNodeInvestment
 * @brief Skill node investment info (for network replication)
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
 * @struct FStatAllocation
 * @brief Stat allocation info (for network replication)
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
 * @struct FHarmoniaProgressionSaveData
 * @brief Player progression save data
 */
USTRUCT(BlueprintType)
struct FHarmoniaProgressionSaveData
{
	GENERATED_BODY()

	/** Current experience */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentExperience = 0;

	/** Current level */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentLevel = 1;

	/** Available skill points */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableSkillPoints = 0;

	/** Available stat points */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableStatPoints = 0;

	/** Current class */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass CurrentClass = EHarmoniaCharacterClass::None;

	/** Current awakening tier */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier CurrentAwakeningTier = EHarmoniaAwakeningTier::Base;

	/** Prestige level (NG+ count) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** Unlocked skill node IDs with invested points */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Skills")
	TArray<FSkillNodeInvestment> UnlockedSkillNodes;

	/** Allocated main stats (Vitality, Strength, etc) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	TArray<FStatAllocation> AllocatedStats;

	/** Total experience earned (lifetime) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Statistics")
	int64 TotalExperienceEarned = 0;

	/** Total levels gained */
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
