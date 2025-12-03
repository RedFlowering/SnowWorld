// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDifficultySystemDefinitions.generated.h"

/**
 * Difficulty Pact Types
 * Individual modifiers that can be toggled on/off
 */
UENUM(BlueprintType)
enum class EHarmoniaDifficultyPactType : uint8
{
	None = 0				UMETA(DisplayName = "None"),

	// Combat Pacts
	GlassCannon				UMETA(DisplayName = "Glass Cannon"),			// Take +50% damage, Deal +30% damage, +20% souls
	IronWill				UMETA(DisplayName = "Iron Will"),				// Limited checkpoints, +100% rewards
	FragileBones			UMETA(DisplayName = "Fragile Bones"),			// Poise -50%, +25% souls
	BerserkersRage			UMETA(DisplayName = "Berserker's Rage"),		// No blocking/parrying, +40% damage, +30% souls

	// Enemy Pacts
	SwiftFoes				UMETA(DisplayName = "Swift Foes"),				// Enemy speed +30%, +25% XP
	EliteUprising			UMETA(DisplayName = "Elite Uprising"),			// +50% elite spawns, +30% rare drops
	IntelligentEnemies		UMETA(DisplayName = "Intelligent Enemies"),		// Enhanced AI, +35% XP
	RelentlessPursuit		UMETA(DisplayName = "Relentless Pursuit"),		// Reduced aggro reset, +20% souls

	// Resource Pacts
	ScarceRecovery			UMETA(DisplayName = "Scarce Recovery"),			// -50% healing effectiveness, +40% item drops
	FragileEquipment		UMETA(DisplayName = "Fragile Equipment"),		// 2x durability loss, +50% upgrade materials
	HungerOfTheVoid			UMETA(DisplayName = "Hunger of the Void"),		// Stamina drain +30%, +25% stamina regen items

	// Economic Pacts
	PovertysCurse			UMETA(DisplayName = "Poverty's Curse"),			// Vendors 50% more expensive, +60% soul drops
	GamblersFate			UMETA(DisplayName = "Gambler's Fate"),			// Random loot quality variance, +50% legendary chance

	// World Pacts
	PermadealthThreat		UMETA(DisplayName = "Permadeath Threat"),		// Death = save deletion (hardcore mode), +200% all rewards
	NoHUD					UMETA(DisplayName = "No HUD"),					// Minimal UI, +15% all rewards
	FogOfWar				UMETA(DisplayName = "Fog of War"),				// Limited minimap/markers, +20% exploration rewards

	MAX						UMETA(Hidden)
};

/**
 * New Game Plus Tier
 */
UENUM(BlueprintType)
enum class EHarmoniaNewGamePlusTier : uint8
{
	BaseGame = 0			UMETA(DisplayName = "Base Game"),
	NewGamePlus1			UMETA(DisplayName = "New Game+"),
	NewGamePlus2			UMETA(DisplayName = "New Game++"),
	NewGamePlus3			UMETA(DisplayName = "New Game+++"),
	NewGamePlus4			UMETA(DisplayName = "New Game++++"),
	NewGamePlus5			UMETA(DisplayName = "New Game+++++"),
	NewGamePlus6			UMETA(DisplayName = "New Game++++++"),
	NewGamePlus7			UMETA(DisplayName = "New Game+++++++"),

	MAX						UMETA(Hidden)
};

/**
 * Dynamic Difficulty Adjustment Mode
 */
UENUM(BlueprintType)
enum class EHarmoniaDynamicDifficultyMode : uint8
{
	Disabled = 0			UMETA(DisplayName = "Disabled"),
	Subtle					UMETA(DisplayName = "Subtle"),					// Minor adjustments (±5%)
	Moderate				UMETA(DisplayName = "Moderate"),				// Medium adjustments (±10%)
	Adaptive				UMETA(DisplayName = "Adaptive"),				// Significant adjustments (±20%)

	MAX						UMETA(Hidden)
};

/**
 * Difficulty Pact Configuration
 * Defines the effects and rewards of a specific pact
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDifficultyPactConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pact")
	EHarmoniaDifficultyPactType PactType = EHarmoniaDifficultyPactType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pact")
	FText PactName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pact")
	FText PactDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pact")
	bool bIsActive = false;

	// Combat Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Combat")
	float PlayerDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Combat")
	float PlayerDamageTakenMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Combat")
	float PlayerPoiseMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Combat")
	bool bDisableBlocking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Combat")
	bool bDisableParrying = false;

	// Enemy Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	float EnemyHealthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	float EnemyDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	float EnemyMovementSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	float EnemyAttackSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	float EliteSpawnChanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	bool bEnhancedAI = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Enemy")
	float AggroResetTimeMultiplier = 1.0f;

	// Resource Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Resources")
	float HealingEffectivenessMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Resources")
	float StaminaDrainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Resources")
	float DurabilityLossMultiplier = 1.0f;

	// Economic Modifiers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float VendorPriceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float SoulGainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float ExperienceGainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float ItemDropRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float RareDropRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float LegendaryDropRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Economy")
	float UpgradeMaterialDropMultiplier = 1.0f;

	// Special Features
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Special")
	int32 MaxCheckpointUses = -1; // -1 = unlimited

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Special")
	bool bPermadeathEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Special")
	bool bHideHUD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Special")
	bool bLimitedMinimap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifiers|Special")
	float LootQualityVariance = 0.0f; // Random variance for Gambler's Fate

	// Difficulty Rating (for calculating total difficulty bonus)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pact")
	float DifficultyRating = 1.0f;

	FHarmoniaDifficultyPactConfig()
		: PactType(EHarmoniaDifficultyPactType::None)
		, bIsActive(false)
		, PlayerDamageMultiplier(1.0f)
		, PlayerDamageTakenMultiplier(1.0f)
		, PlayerPoiseMultiplier(1.0f)
		, bDisableBlocking(false)
		, bDisableParrying(false)
		, EnemyHealthMultiplier(1.0f)
		, EnemyDamageMultiplier(1.0f)
		, EnemyMovementSpeedMultiplier(1.0f)
		, EnemyAttackSpeedMultiplier(1.0f)
		, EliteSpawnChanceMultiplier(1.0f)
		, bEnhancedAI(false)
		, AggroResetTimeMultiplier(1.0f)
		, HealingEffectivenessMultiplier(1.0f)
		, StaminaDrainMultiplier(1.0f)
		, DurabilityLossMultiplier(1.0f)
		, VendorPriceMultiplier(1.0f)
		, SoulGainMultiplier(1.0f)
		, ExperienceGainMultiplier(1.0f)
		, ItemDropRateMultiplier(1.0f)
		, RareDropRateMultiplier(1.0f)
		, LegendaryDropRateMultiplier(1.0f)
		, UpgradeMaterialDropMultiplier(1.0f)
		, MaxCheckpointUses(-1)
		, bPermadeathEnabled(false)
		, bHideHUD(false)
		, bLimitedMinimap(false)
		, LootQualityVariance(0.0f)
		, DifficultyRating(1.0f)
	{
	}
};

/**
 * New Game Plus Legacy Bonus
 * Permanent bonuses earned from completing the game
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaNewGamePlusLegacy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy")
	int32 CompletionCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy")
	EHarmoniaNewGamePlusTier CurrentTier = EHarmoniaNewGamePlusTier::BaseGame;

	// Permanent Bonuses (cumulative)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Bonuses")
	float PermanentHealthBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Bonuses")
	float PermanentStaminaBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Bonuses")
	float PermanentDamageBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Bonuses")
	float PermanentSoulGainBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Bonuses")
	int32 BonusSkillPoints = 0;

	// Unlocked Features
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Unlocks")
	TArray<FName> UnlockedCosmetics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Unlocks")
	TArray<FName> UnlockedWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Unlocks")
	TArray<FName> UnlockedAbilities;

	// Statistics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Stats")
	float FastestCompletionTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Stats")
	int32 TotalDeaths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Legacy|Stats")
	int32 TotalBossesDefeated = 0;

	FHarmoniaNewGamePlusLegacy()
		: CompletionCount(0)
		, CurrentTier(EHarmoniaNewGamePlusTier::BaseGame)
		, PermanentHealthBonus(0.0f)
		, PermanentStaminaBonus(0.0f)
		, PermanentDamageBonus(0.0f)
		, PermanentSoulGainBonus(0.0f)
		, BonusSkillPoints(0)
		, FastestCompletionTime(0.0f)
		, TotalDeaths(0)
		, TotalBossesDefeated(0)
	{
	}
};

/**
 * Dynamic Difficulty State
 * Tracks player performance for adaptive difficulty
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDynamicDifficultyState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Difficulty")
	EHarmoniaDynamicDifficultyMode Mode = EHarmoniaDynamicDifficultyMode::Disabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Difficulty")
	float CurrentAdjustment = 1.0f; // Multiplier applied to difficulty (0.8 to 1.2)

	// Performance Tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 RecentDeaths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 RecentVictories = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageHealthRemaining = 1.0f; // 0.0 to 1.0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageCombatDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 PerfectParries = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 PerfectDodges = 0;

	FHarmoniaDynamicDifficultyState()
		: Mode(EHarmoniaDynamicDifficultyMode::Disabled)
		, CurrentAdjustment(1.0f)
		, RecentDeaths(0)
		, RecentVictories(0)
		, AverageHealthRemaining(1.0f)
		, AverageCombatDuration(0.0f)
		, PerfectParries(0)
		, PerfectDodges(0)
	{
	}
};

/**
 * Complete Difficulty Configuration
 * Contains all difficulty settings for the current playthrough
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDifficultyConfiguration
{
	GENERATED_BODY()

	// Active Pacts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TArray<FHarmoniaDifficultyPactConfig> ActivePacts;

	// New Game Plus
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FHarmoniaNewGamePlusLegacy NewGamePlusLegacy;

	// Dynamic Difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FHarmoniaDynamicDifficultyState DynamicDifficulty;

	// Base Difficulty Level (from original system)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 BaseDifficultyLevel = 1; // 0=Easy, 1=Normal, 2=Hard, 3=Very Hard

	FHarmoniaDifficultyConfiguration()
		: BaseDifficultyLevel(1)
	{
	}
};
