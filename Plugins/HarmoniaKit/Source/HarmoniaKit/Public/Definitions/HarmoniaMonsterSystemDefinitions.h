// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "HarmoniaTeamSystemDefinitions.h"
#include "HarmoniaMonsterSystemDefinitions.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UBehaviorTree;

// ============================================================================
// Monster State & Behavior Enums
// ============================================================================

/**
 * Monster State Enum
 * Defines the current behavior state of the monster
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Alert UMETA(DisplayName = "Alert"),
	Combat UMETA(DisplayName = "Combat"),
	Retreating UMETA(DisplayName = "Retreating"),
	Stunned UMETA(DisplayName = "Stunned"),
	Dead UMETA(DisplayName = "Dead")
};

/**
 * Monster Aggro Type
 * Defines how the monster reacts to players
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterAggroType : uint8
{
	Passive UMETA(DisplayName = "Passive"),		// Never attacks unless provoked
	Neutral UMETA(DisplayName = "Neutral"),		// Attacks when attacked
	Aggressive UMETA(DisplayName = "Aggressive"),	// Attacks on sight
	Territorial UMETA(DisplayName = "Territorial")	// Attacks when in territory
};

// ============================================================================
// Faction System
// ============================================================================

/**
 * Monster Faction/Team
 * Determines which actors are allies, enemies, or neutral
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterFaction : uint8
{
	/**
	 * Neutral - Does not attack anyone unless attacked first
	 * Examples: Wildlife, passive creatures
	 */
	Neutral UMETA(DisplayName = "Neutral"),

	/**
	 * Player Hostile - Attacks players on sight
	 * Examples: Standard enemy monsters
	 */
	PlayerHostile UMETA(DisplayName = "Player Hostile"),

	/**
	 * Player Friendly - Helps players, attacks enemies
	 * Examples: Allied NPCs, summoned creatures
	 */
	PlayerFriendly UMETA(DisplayName = "Player Friendly"),

	/**
	 * Monster - Other monsters of same faction (typically don't attack each other)
	 * Examples: Goblin tribe, Undead legion
	 */
	Monster1 UMETA(DisplayName = "Monster Faction 1"),
	Monster2 UMETA(DisplayName = "Monster Faction 2"),
	Monster3 UMETA(DisplayName = "Monster Faction 3"),
	Monster4 UMETA(DisplayName = "Monster Faction 4"),

	/**
	 * Custom factions for specific use cases
	 */
	Custom1 UMETA(DisplayName = "Custom Faction 1"),
	Custom2 UMETA(DisplayName = "Custom Faction 2"),
	Custom3 UMETA(DisplayName = "Custom Faction 3")
};

/**
 * Faction Relationship
 * Defines how a faction reacts to another faction
 */
UENUM(BlueprintType)
enum class EFactionRelationship : uint8
{
	/** Treat as friend, won't attack */
	Friendly UMETA(DisplayName = "Friendly"),

	/** Ignore unless provoked */
	Neutral UMETA(DisplayName = "Neutral"),

	/** Attack on sight */
	Hostile UMETA(DisplayName = "Hostile")
};

/**
 * Faction Settings
 * Configures faction behavior and relationships
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaFactionSettings
{
	GENERATED_BODY()

	/** This monster's faction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	EHarmoniaMonsterFaction Faction = EHarmoniaMonsterFaction::PlayerHostile;

	/**
	 * Can attack members of same faction
	 * Useful for free-for-all scenarios or boss minions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	bool bCanAttackSameFaction = false;

	/**
	 * Retaliate when attacked, even by normally friendly factions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	bool bRetaliateWhenAttacked = true;

	/**
	 * Help allies of same faction when they're attacked
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	bool bDefendAllies = true;

	/**
	 * Custom faction relationships (overrides default behavior)
	 * Key: Faction, Value: Relationship
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	TMap<EHarmoniaMonsterFaction, EFactionRelationship> CustomRelationships;

	/**
	 * Get relationship with another faction
	 */
	EFactionRelationship GetRelationshipWith(EHarmoniaMonsterFaction OtherFaction) const;

	/**
	 * Can this faction attack the other faction?
	 */
	bool CanAttack(EHarmoniaMonsterFaction OtherFaction) const;

	/**
	 * Should help the other faction?
	 */
	bool ShouldHelp(EHarmoniaMonsterFaction OtherFaction) const;
};

// ============================================================================
// Loot System
// ============================================================================

/**
 * Loot Item Type
 */
UENUM(BlueprintType)
enum class EHarmoniaLootItemType : uint8
{
	Item UMETA(DisplayName = "Item"),				// Regular item from inventory system
	Currency UMETA(DisplayName = "Currency"),		// Gold, gems, etc.
	Experience UMETA(DisplayName = "Experience"),	// XP reward
	Custom UMETA(DisplayName = "Custom")			// Custom reward type
};

/**
 * Loot Rarity
 */
UENUM(BlueprintType)
enum class EHarmoniaLootRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Loot Table Row
 * Defines a single loot entry with drop chance
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

	// Type of loot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	EHarmoniaLootItemType LootType = EHarmoniaLootItemType::Item;

	// Item ID (row name from item data table)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName ItemID = NAME_None;

	// Quantity range (min-max)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 MaxQuantity = 1;

	// Drop chance (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;

	// Rarity (for filtering/display)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	EHarmoniaLootRarity Rarity = EHarmoniaLootRarity::Common;

	// Level requirement for this loot to drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	int32 MinMonsterLevel = 1;

	// Whether this is a guaranteed drop
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	bool bGuaranteedDrop = false;

	// Custom data (for script extensions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FString CustomData;
};

/**
 * Loot Table Data Asset
 * Collection of loot entries for a monster or group of monsters
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaLootTableData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Display name for this loot table
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	FText TableName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table", meta = (MultiLine = true))
	FText Description;

	// Loot entries
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	TArray<FHarmoniaLootTableRow> LootEntries;

	// Maximum number of items to drop
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	int32 MaxDrops = 3;

	// Guaranteed gold range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	int32 MinGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	int32 MaxGold = 0;

	// Guaranteed experience
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	int32 BaseExperience = 0;

	// Experience multiplier per monster level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Table")
	float ExperiencePerLevel = 1.0f;

	/**
	 * Generate loot from this table
	 * @param MonsterLevel Level of the monster
	 * @param LuckModifier Luck modifier from player (increases drop chances)
	 * @return Array of loot items to drop
	 */
	UFUNCTION(BlueprintCallable, Category = "Loot Table")
	TArray<FHarmoniaLootTableRow> GenerateLoot(int32 MonsterLevel, float LuckModifier = 0.0f) const;
};

// ============================================================================
// Monster Stats & Configuration
// ============================================================================

/**
 * Monster Stats
 * Base stats for a monster (before level scaling)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMonsterStats
{
	GENERATED_BODY()

	// Base Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseHealth = 100.0f;

	// Base Max Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseMaxHealth = 100.0f;

	// Base Attack Power
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseAttackPower = 10.0f;

	// Base Defense
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseDefense = 5.0f;

	// Base Movement Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseMovementSpeed = 400.0f;

	// Base Attack Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseAttackSpeed = 1.0f;

	// Critical Chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalChance = 0.05f;

	// Critical Damage Multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CriticalDamage = 1.5f;

	// Health scaling per level (percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float HealthPerLevel = 0.1f;

	// Attack power scaling per level (percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float AttackPowerPerLevel = 0.08f;

	// Defense scaling per level (percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float DefensePerLevel = 0.05f;

	/**
	 * Calculate scaled stat value
	 */
	static float CalculateScaledStat(float BaseStat, float PerLevelMultiplier, int32 Level)
	{
		return BaseStat * (1.0f + PerLevelMultiplier * FMath::Max(0, Level - 1));
	}

	/**
	 * Get scaled health for given level
	 */
	float GetScaledHealth(int32 Level) const
	{
		return CalculateScaledStat(BaseMaxHealth, HealthPerLevel, Level);
	}

	/**
	 * Get scaled attack power for given level
	 */
	float GetScaledAttackPower(int32 Level) const
	{
		return CalculateScaledStat(BaseAttackPower, AttackPowerPerLevel, Level);
	}

	/**
	 * Get scaled defense for given level
	 */
	float GetScaledDefense(int32 Level) const
	{
		return CalculateScaledStat(BaseDefense, DefensePerLevel, Level);
	}
};

/**
 * Boss Phase Data
 * Defines a phase for boss monsters
 * 
 * NOTE: Phase는 HP 임계값 기반 상태 전이만 정의합니다.
 * 공격 패턴, 어빌리티, 전환 연출 등은 UHarmoniaBossPatternComponent에서 관리합니다.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaBossPhase
{
	GENERATED_BODY()

	/** Phase identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FName PhaseID = NAME_None;

	/** Phase name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FText PhaseName;

	/** Health percentage to trigger this phase (1.0 = 100%, 0.5 = 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthThreshold = 1.0f;
};

// ============================================================================
// Group AI Definitions (Forward Declarations)
// ============================================================================

/**
 * Squad Formation Type
 * Defines how squad members position themselves
 */
UENUM(BlueprintType)
enum class EHarmoniaSquadFormationType : uint8
{
	Loose UMETA(DisplayName = "Loose"),			// Spread out, maintain distance
	Tight UMETA(DisplayName = "Tight"),			// Stay close together
	Circle UMETA(DisplayName = "Circle"),		// Surround target
	Line UMETA(DisplayName = "Line"),			// Form a line
	Wedge UMETA(DisplayName = "Wedge"),			// V-formation
	Flanking UMETA(DisplayName = "Flanking")	// Flank target from sides
};

/**
 * Squad Role
 * Defines the role of a monster within a squad
 */
UENUM(BlueprintType)
enum class EHarmoniaSquadRole : uint8
{
	Leader UMETA(DisplayName = "Leader"),		// Leads the squad
	Tank UMETA(DisplayName = "Tank"),			// Frontline fighter
	DPS UMETA(DisplayName = "DPS"),				// Damage dealer
	Support UMETA(DisplayName = "Support"),		// Healer/buffer
	Scout UMETA(DisplayName = "Scout")			// Reconnaissance
};

/**
 * Monster Data Asset
 * Primary data asset for defining a monster type
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Basic Information
	// ============================================================================

	// Monster ID (unique identifier)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Info")
	FName MonsterID = NAME_None;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Info")
	FText MonsterName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Info", meta = (MultiLine = true))
	FText Description;

	// Monster icon/portrait
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster Info")
	TObjectPtr<UTexture2D> MonsterIcon = nullptr;

	// ============================================================================
	// Stats
	// ============================================================================

	/** Base stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FHarmoniaMonsterStats BaseStats;

	// ============================================================================
	// Team
	// ============================================================================

	/** Team identification */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Team")
	FHarmoniaTeamIdentification TeamID;

	// ============================================================================
	// AI Behavior
	// ============================================================================

	/** Behavior tree to use */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;

	// ============================================================================
	// Loot & Rewards
	// ============================================================================

	// Loot table reference
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UHarmoniaLootTableData> LootTable = nullptr;

	// ============================================================================
	// Gameplay Tags
	// ============================================================================

	// Gameplay tags for this monster type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	FGameplayTagContainer MonsterTags;

	// ============================================================================
	// Visual & Audio
	// ============================================================================

	// Death montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;

	// Spawn montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> SpawnMontage = nullptr;

	// Ragdoll on death
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death")
	bool bRagdollOnDeath = true;

	// Time before destroying corpse (0 = never)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death")
	float CorpseLifetime = 30.0f;

	// ============================================================================
	// Boss Specific (optional)
	// ============================================================================

	// Whether this is a boss monster
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	bool bIsBoss = false;

	// Boss phases (only used if bIsBoss is true)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (EditCondition = "bIsBoss"))
	TArray<FHarmoniaBossPhase> BossPhases;

	// Boss music/theme
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (EditCondition = "bIsBoss"))
	TObjectPtr<USoundBase> BossMusic = nullptr;

	// Show boss health bar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (EditCondition = "bIsBoss"))
	bool bShowBossHealthBar = true;

	// ============================================================================
	// Group AI Settings (optional)
	// ============================================================================

	// Whether this monster can participate in squad behavior
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group AI")
	bool bCanFormSquad = true;

	// Preferred squad size
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group AI", meta = (EditCondition = "bCanFormSquad", ClampMin = "2", ClampMax = "10"))
	int32 PreferredSquadSize = 3;

	// Squad formation type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group AI", meta = (EditCondition = "bCanFormSquad"))
	EHarmoniaSquadFormationType FormationType = EHarmoniaSquadFormationType::Loose;

	// Whether this monster can be a squad leader
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group AI", meta = (EditCondition = "bCanFormSquad"))
	bool bCanBeSquadLeader = true;

	// Squad coordination range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Group AI", meta = (EditCondition = "bCanFormSquad", ClampMin = "500", ClampMax = "5000"))
	float SquadCoordinationRange = 2000.0f;
};

/**
 * Squad Member Info
 * Information about a single squad member
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSquadMemberInfo
{
	GENERATED_BODY()

	// Monster actor
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Monster = nullptr;

	// Assigned role
	UPROPERTY(BlueprintReadWrite)
	EHarmoniaSquadRole Role = EHarmoniaSquadRole::DPS;

	// Formation position (relative to leader)
	UPROPERTY(BlueprintReadWrite)
	FVector FormationOffset = FVector::ZeroVector;

	// Whether this member is alive
	UPROPERTY(BlueprintReadWrite)
	bool bIsAlive = true;

	// Last known location (for regrouping)
	UPROPERTY(BlueprintReadWrite)
	FVector LastKnownLocation = FVector::ZeroVector;

	FHarmoniaSquadMemberInfo()
	{
	}

	bool IsValid() const
	{
		return Monster != nullptr && bIsAlive;
	}
};

// ============================================================================
// Emotion & Advanced AI State Definitions
// ============================================================================

/**
 * Monster Emotion State
 * Defines the emotional state that affects behavior
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterEmotion : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),		// Normal state
	Enraged UMETA(DisplayName = "Enraged"),		// High damage, low defense
	Fearful UMETA(DisplayName = "Fearful"),		// Defensive, may flee
	Cautious UMETA(DisplayName = "Cautious"),	// Defensive, tactical
	Confident UMETA(DisplayName = "Confident"),	// Aggressive, risky
	Exhausted UMETA(DisplayName = "Exhausted")	// Low performance
};

/**
 * Tactical Position Type
 * Defines types of tactical positions monsters can seek
 */
UENUM(BlueprintType)
enum class EHarmoniaTacticalPosition : uint8
{
	None UMETA(DisplayName = "None"),
	Cover UMETA(DisplayName = "Cover"),				// Behind obstacle
	HighGround UMETA(DisplayName = "High Ground"),	// Elevated position
	Flanking UMETA(DisplayName = "Flanking"),		// Side attack position
	Retreat UMETA(DisplayName = "Retreat"),			// Safe distance
	Ambush UMETA(DisplayName = "Ambush")			// Hidden attack position
};

/**
 * Emotion State Data
 * Stores information about current emotional state
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaEmotionStateData
{
	GENERATED_BODY()

	// Current emotion
	UPROPERTY(BlueprintReadWrite)
	EHarmoniaMonsterEmotion CurrentEmotion = EHarmoniaMonsterEmotion::Neutral;

	// Intensity of the emotion (0.0 - 1.0)
	UPROPERTY(BlueprintReadWrite)
	float Intensity = 0.0f;

	// Time remaining in this emotional state
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0.0f;

	// Attack power modifier
	UPROPERTY(BlueprintReadWrite)
	float AttackModifier = 1.0f;

	// Defense modifier
	UPROPERTY(BlueprintReadWrite)
	float DefenseModifier = 1.0f;

	// Movement speed modifier
	UPROPERTY(BlueprintReadWrite)
	float SpeedModifier = 1.0f;

	// Attack speed modifier
	UPROPERTY(BlueprintReadWrite)
	float AttackSpeedModifier = 1.0f;

	FHarmoniaEmotionStateData()
	{
	}

	// Check if in active emotional state
	bool IsActive() const
	{
		return CurrentEmotion != EHarmoniaMonsterEmotion::Neutral && Duration > 0.0f;
	}
};

/**
 * Combo Attack State
 * Tracks current combo chain progress
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaComboState
{
	GENERATED_BODY()

	// Current attack in the combo
	UPROPERTY(BlueprintReadWrite)
	FName CurrentAttackID = NAME_None;

	// Available follow-up attacks
	UPROPERTY(BlueprintReadWrite)
	TArray<FName> AvailableFollowUps;

	// Time remaining to execute next combo attack
	UPROPERTY(BlueprintReadWrite)
	float ComboWindowRemaining = 0.0f;

	// Number of attacks in current combo chain
	UPROPERTY(BlueprintReadWrite)
	int32 ComboCount = 0;

	FHarmoniaComboState()
	{
	}

	// Check if combo is active
	bool IsActive() const
	{
		return ComboWindowRemaining > 0.0f && AvailableFollowUps.Num() > 0;
	}

	// Reset combo state
	void Reset()
	{
		CurrentAttackID = NAME_None;
		AvailableFollowUps.Empty();
		ComboWindowRemaining = 0.0f;
		ComboCount = 0;
	}
};

/**
 * Tactical State Data
 * Stores information about tactical positioning
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaTacticalStateData
{
	GENERATED_BODY()

	// Current tactical position type
	UPROPERTY(BlueprintReadWrite)
	EHarmoniaTacticalPosition CurrentPosition = EHarmoniaTacticalPosition::None;

	// Target tactical position location
	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	// Whether currently in optimal position
	UPROPERTY(BlueprintReadWrite)
	bool bInOptimalPosition = false;

	// Cover actor (if using cover)
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> CoverActor = nullptr;

	// Height advantage over target (in cm)
	UPROPERTY(BlueprintReadWrite)
	float HeightAdvantage = 0.0f;

	FHarmoniaTacticalStateData()
	{
	}

	// Check if has valid tactical position
	bool HasValidPosition() const
	{
		return CurrentPosition != EHarmoniaTacticalPosition::None;
	}
};
