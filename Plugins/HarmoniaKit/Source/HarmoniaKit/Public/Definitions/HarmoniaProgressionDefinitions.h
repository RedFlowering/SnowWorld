// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaProgressionDefinitions.generated.h"

class UGameplayEffect;
class ULyraAbilitySet;
class UTexture2D;

/**
 * ?¤í‚¬ ?¸ë¦¬ ?¸ë“œ ?€??
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillNodeType : uint8
{
	// ?¨ì‹œë¸??¤í‚¬ (stat bonuses)
	Passive UMETA(DisplayName = "Passive Skill"),

	// ?¡í‹°ë¸??¤í‚¬ (unlocks abilities)
	Active UMETA(DisplayName = "Active Skill"),

	// ?ì„± ì¦ê? (primary stats: Str, Dex, etc)
	AttributeBoost UMETA(DisplayName = "Attribute Boost"),

	// ?¹ìˆ˜ ?¸ë“œ (?´ë˜???„í™˜, ê°ì„± ??
	Special UMETA(DisplayName = "Special Node")
};

/**
 * ?¤í‚¬ ?¸ë¦¬ ì¹´í…Œê³ ë¦¬
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillTreeCategory : uint8
{
	// ë¬´ê¸°ë³??„ë¬¸??
	SwordMastery UMETA(DisplayName = "Sword Mastery"),
	AxeMastery UMETA(DisplayName = "Axe Mastery"),
	SpearMastery UMETA(DisplayName = "Spear Mastery"),
	BowMastery UMETA(DisplayName = "Bow Mastery"),
	MagicMastery UMETA(DisplayName = "Magic Mastery"),

	// ?¤í??¼ë³„ ?„ë¬¸??
	OffensiveCombat UMETA(DisplayName = "Offensive Combat"),
	DefensiveCombat UMETA(DisplayName = "Defensive Combat"),
	StealthTactics UMETA(DisplayName = "Stealth Tactics"),

	// ê¸°í?
	Survival UMETA(DisplayName = "Survival"),
	Crafting UMETA(DisplayName = "Crafting"),
	Social UMETA(DisplayName = "Social")
};

/**
 * ìºë¦­???´ë˜???€??
 */
UENUM(BlueprintType)
enum class EHarmoniaCharacterClass : uint8
{
	// ê¸°ë³¸ ?´ë˜??
	None UMETA(DisplayName = "No Class"),

	// Tier 1 ?´ë˜??
	Warrior UMETA(DisplayName = "Warrior"),
	Rogue UMETA(DisplayName = "Rogue"),
	Mage UMETA(DisplayName = "Mage"),

	// Tier 2 ?´ë˜??(?„ì‚¬ ê³„ì—´)
	Berserker UMETA(DisplayName = "Berserker"),
	Knight UMETA(DisplayName = "Knight"),
	Paladin UMETA(DisplayName = "Paladin"),

	// Tier 2 ?´ë˜??(?„ì  ê³„ì—´)
	Assassin UMETA(DisplayName = "Assassin"),
	Ranger UMETA(DisplayName = "Ranger"),
	Duelist UMETA(DisplayName = "Duelist"),

	// Tier 2 ?´ë˜??(ë§ˆë²•??ê³„ì—´)
	Sorcerer UMETA(DisplayName = "Sorcerer"),
	Cleric UMETA(DisplayName = "Cleric"),
	Warlock UMETA(DisplayName = "Warlock")
};

/**
 * ê°ì„± ?¨ê³„
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
 * ?¤í‚¬ ?¸ë“œ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FHarmoniaSkillNode
{
	GENERATED_BODY()

	/** ?¸ë“œ ê³ ìœ  ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FName NodeID;

	/** ?¸ë“œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeName;

	/** ?¸ë“œ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	FText NodeDescription;

	/** ?¸ë“œ ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	TObjectPtr<UTexture2D> NodeIcon;

	/** ?¸ë“œ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillNodeType NodeType = EHarmoniaSkillNodeType::Passive;

	/** ?¤í‚¬ ?¸ë¦¬ ì¹´í…Œê³ ë¦¬ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	EHarmoniaSkillTreeCategory Category = EHarmoniaSkillTreeCategory::SwordMastery;

	/** ?„ìš” ?¤í‚¬ ?¬ì¸??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredSkillPoints = 1;

	/** ?„ìš” ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 1;

	/** ? í–‰ ?¸ë“œ ID??(???¸ë“œ?¤ì´ ë¨¼ì? ?´ê¸ˆ?˜ì–´???? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> PrerequisiteNodeIDs;

	/** ?„ìš” ?´ë˜??(None?´ë©´ ?œí•œ ?†ìŒ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** ?´ê¸ˆ ??ë¶€?¬í•  GameplayEffect (stat bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** ?´ê¸ˆ ??ë¶€?¬í•  AbilitySet (abilities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<ULyraAbilitySet> GrantedAbilitySet;

	/** ìµœë? ?¬ì ê°€???¬ì¸??(?ˆë²¨??ê°€?¥í•œ ?¤í‚¬) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
	int32 MaxInvestmentPoints = 1;

	/** UI ?„ì¹˜ (?¤í‚¬ ?¸ë¦¬ ê·¸ë˜?„ì—?œì˜ ì¢Œí‘œ) */
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
 * ?´ë˜???•ì˜ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FHarmoniaClassDefinition
{
	GENERATED_BODY()

	/** ?´ë˜???€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass ClassType = EHarmoniaCharacterClass::None;

	/** ?´ë˜???´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassName;

	/** ?´ë˜???¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText ClassDescription;

	/** ?´ë˜???„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TObjectPtr<UTexture2D> ClassIcon;

	/** ?œì‘ ?¤íƒ¯ ë³´ë„ˆ??(Vitality, Strength, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Starting Stats")
	TMap<FGameplayTag, float> StartingStatBonuses;

	/** ?ˆë²¨?????ë™ ?¤íƒ¯ ì¦ê? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	TMap<FGameplayTag, float> StatGrowthPerLevel;

	/** ?´ë˜??? íƒ ??ë¶€?¬ë˜??GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TSubclassOf<UGameplayEffect>> ClassEffects;

	/** ?´ë˜??? íƒ ??ë¶€?¬ë˜??AbilitySet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TObjectPtr<ULyraAbilitySet> ClassAbilitySet;

	/** ???´ë˜?¤ë¡œ ?„ì§ ê°€?¥í•œ ?ìœ„ ?´ë˜?¤ë“¤ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	TArray<EHarmoniaCharacterClass> AdvancementClasses;

	/** ?„ì§ ?„ìš” ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advancement")
	int32 RequiredLevelForAdvancement = 20;

	FHarmoniaClassDefinition()
		: ClassType(EHarmoniaCharacterClass::None)
		, RequiredLevelForAdvancement(20)
	{
	}
};

/**
 * ê°ì„± ?¨ê³„ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FHarmoniaAwakeningDefinition
{
	GENERATED_BODY()

	/** ê°ì„± ?¨ê³„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier Tier = EHarmoniaAwakeningTier::Base;

	/** ê°ì„± ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningName;

	/** ê°ì„± ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awakening")
	FText AwakeningDescription;

	/** ?„ìš” ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredLevel = 50;

	/** ê°ì„± ?˜ìŠ¤???œê·¸ (?„ë£Œ?´ì•¼ ???˜ìŠ¤?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FGameplayTag RequiredQuestTag;

	/** ê°ì„± ??ë¶€?¬ë˜???¤íƒ¯ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	float StatMultiplier = 1.2f;

	/** ê°ì„± ??ë¶€?¬ë˜??GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TArray<TSubclassOf<UGameplayEffect>> AwakeningEffects;

	/** ê°ì„± ??ë¶€?¬ë˜??AbilitySet */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonuses")
	TObjectPtr<ULyraAbilitySet> AwakeningAbilitySet;

	/** ê°ì„± ë³´ìƒ ?¤í‚¬ ?¬ì¸??*/
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
 * ?´ê²Œ?? ?„ë ˆ?¤í‹°ì§€ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FHarmoniaPrestigeDefinition
{
	GENERATED_BODY()

	/** ?„ë ˆ?¤í‹°ì§€ ?ˆë²¨ (0 = ê¸°ë³¸, 1+ = NG+) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** ?„ìš” ìµœì†Œ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredPlayerLevel = 100;

	/** ???œì´??ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float EnemyDifficultyMultiplier = 1.5f;

	/** ë³´ìƒ ë°°ìœ¨ (ê²½í—˜ì¹? ê³¨ë“œ, ?œë) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	float RewardMultiplier = 2.0f;

	/** ?„ë ˆ?¤í‹°ì§€ ë³´ë„ˆ???¤íƒ¯ ?¬ì¸??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusStatPoints = 10;

	/** ?„ë ˆ?¤í‹°ì§€ ë³´ë„ˆ???¤í‚¬ ?¬ì¸??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 BonusSkillPoints = 10;

	/** ?„ë ˆ?¤í‹°ì§€ ?„ìš© GameplayEffect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	TArray<TSubclassOf<UGameplayEffect>> PrestigeEffects;

	/** ?„ë ˆ?¤í‹°ì§€ ?€?´í?/ë±ƒì? */
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
 * ?¤í‚¬ ?¸ë“œ ?¬ì ?•ë³´ (?¤íŠ¸?Œí¬ ë³µì œ??
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
 * ?¤íƒ¯ ë°°ë¶„ ?•ë³´ (?¤íŠ¸?Œí¬ ë³µì œ??
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
 * ?Œë ˆ?´ì–´ ì§„í–‰ ?í™© ?€???°ì´??
 */
USTRUCT(BlueprintType)
struct FHarmoniaProgressionSaveData
{
	GENERATED_BODY()

	/** ?„ì¬ ê²½í—˜ì¹?*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentExperience = 0;

	/** ?„ì¬ ?ˆë²¨ */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 CurrentLevel = 1;

	/** ?¬ìš© ê°€?¥í•œ ?¤í‚¬ ?¬ì¸??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableSkillPoints = 0;

	/** ?¬ìš© ê°€?¥í•œ ?¤íƒ¯ ?¬ì¸??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 AvailableStatPoints = 0;

	/** ?„ì¬ ?´ë˜??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Class")
	EHarmoniaCharacterClass CurrentClass = EHarmoniaCharacterClass::None;

	/** ?„ì¬ ê°ì„± ?¨ê³„ */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Awakening")
	EHarmoniaAwakeningTier CurrentAwakeningTier = EHarmoniaAwakeningTier::Base;

	/** ?„ë ˆ?¤í‹°ì§€ ?ˆë²¨ (NG+ ?Ÿìˆ˜) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Prestige")
	int32 PrestigeLevel = 0;

	/** ?´ê¸ˆ???¤í‚¬ ?¸ë“œ ID?¤ê³¼ ?¬ì ?¬ì¸??*/
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Skills")
	TArray<FSkillNodeInvestment> UnlockedSkillNodes;

	/** ë°°ë¶„??ì£¼ìš” ?¤íƒ¯ (Vitality, Strength, etc) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	TArray<FStatAllocation> AllocatedStats;

	/** ì´??ë“ ê²½í—˜ì¹?(lifetime) */
	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Statistics")
	int64 TotalExperienceEarned = 0;

	/** ì´??ˆë²¨???Ÿìˆ˜ */
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
