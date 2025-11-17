// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "HarmoniaMonsterSystemDefinitions.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UBehaviorTree;

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
 * Monster Attack Pattern
 * Defines a single attack that a monster can perform
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMonsterAttackPattern
{
	GENERATED_BODY()

	// Attack identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FName AttackID = NAME_None;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FText AttackName;

	// Attack animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	// Gameplay ability to activate for this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TSubclassOf<UGameplayAbility> AttackAbility;

	// Minimum range for this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float MinRange = 0.0f;

	// Maximum range for this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float MaxRange = 200.0f;

	// Cooldown between uses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Cooldown = 2.0f;

	// Weight for random attack selection (higher = more likely)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	int32 SelectionWeight = 1;

	// Required monster state to use this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	EHarmoniaMonsterState RequiredState = EHarmoniaMonsterState::Combat;

	// Gameplay tags required to use this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTagContainer RequiredTags;

	// Gameplay tags that block this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FGameplayTagContainer BlockedTags;
};

/**
 * Boss Phase Data
 * Defines a phase for boss monsters
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaBossPhase
{
	GENERATED_BODY()

	// Phase identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FName PhaseID = NAME_None;

	// Phase name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	FText PhaseName;

	// Health percentage to trigger this phase (1.0 = 100%, 0.5 = 50%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthThreshold = 1.0f;

	// Phase transition montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> TransitionMontage = nullptr;

	// Attacks available in this phase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TArray<FHarmoniaMonsterAttackPattern> PhaseAttacks;

	// Gameplay effects to apply when entering this phase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<TSubclassOf<UGameplayEffect>> PhaseEffects;

	// Behavior tree to use in this phase (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TObjectPtr<UBehaviorTree> PhaseBehaviorTree = nullptr;

	// Movement speed multiplier for this phase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	float MovementSpeedMultiplier = 1.0f;

	// Attack speed multiplier for this phase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	float AttackSpeedMultiplier = 1.0f;

	// Whether boss is invulnerable during transition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
	bool bInvulnerableDuringTransition = true;
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
	// Stats & Combat
	// ============================================================================

	// Base stats
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FHarmoniaMonsterStats BaseStats;

	// Aggro type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	EHarmoniaMonsterAggroType AggroType = EHarmoniaMonsterAggroType::Neutral;

	// Aggro range (how far the monster can detect players)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float AggroRange = 1000.0f;

	// Attack range (how close monster needs to be to attack)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float AttackRange = 200.0f;

	// Retreat health percentage (when to run away, 0 = never)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RetreatHealthThreshold = 0.0f;

	// ============================================================================
	// Attacks
	// ============================================================================

	// Available attack patterns
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FHarmoniaMonsterAttackPattern> AttackPatterns;

	// ============================================================================
	// AI Behavior
	// ============================================================================

	// Behavior tree to use
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree = nullptr;

	// Patrol radius (0 = no patrol)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float PatrolRadius = 500.0f;

	// Wait time between patrol points
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float PatrolWaitTime = 3.0f;

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
};
