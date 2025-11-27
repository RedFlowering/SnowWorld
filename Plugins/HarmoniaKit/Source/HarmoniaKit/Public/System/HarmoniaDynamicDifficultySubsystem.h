// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDynamicDifficultySubsystem.generated.h"

/**
 * Player performance metrics for DDA
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPerformanceMetrics
{
	GENERATED_BODY()

	// Combat Performance
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 TotalDeaths = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 DeathsInLastHour = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 ConsecutiveDeaths = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	int32 KillsWithoutDeath = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float AverageHealthOnVictory = 0.5f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	float AverageCombatDuration = 0.0f;

	// Skill Metrics
	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	float ParrySuccessRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	float DodgeSuccessRate = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	float HitAccuracy = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	int32 PerfectParries = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Skill")
	int32 PerfectDodges = 0;

	// Session Data
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	float PlayTimeHours = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 BossesDefeated = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Session")
	int32 ElitesDefeated = 0;

	// Damage Metrics
	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float TotalDamageDealt = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float TotalDamageTaken = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	float DamageRatio = 1.0f;  // Dealt / Taken

	// Calculated Skill Rating (0-100)
	UPROPERTY(BlueprintReadWrite, Category = "Rating")
	float OverallSkillRating = 50.0f;

	// Time tracking
	UPROPERTY(BlueprintReadWrite, Category = "Internal")
	TArray<float> RecentDeathTimestamps;

	UPROPERTY(BlueprintReadWrite, Category = "Internal")
	TArray<float> RecentVictoryHealthPercents;

	UPROPERTY(BlueprintReadWrite, Category = "Internal")
	TArray<float> RecentCombatDurations;
};

/**
 * DDA adjustment parameters
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAParameters
{
	GENERATED_BODY()

	// Enemy Adjustments
	UPROPERTY(BlueprintReadWrite, Category = "Enemy")
	float EnemyHealthMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Enemy")
	float EnemyDamageMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Enemy")
	float EnemyAggressionMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Enemy")
	float EnemyReactionTimeMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Enemy")
	float EnemyAttackFrequencyMultiplier = 1.0f;

	// Player Adjustments (Hidden Assistance)
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	float PlayerDamageMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	float PlayerDefenseMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	float ParryWindowMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	float DodgeIFrameMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Player")
	float HealingMultiplier = 1.0f;

	// Resource Adjustments
	UPROPERTY(BlueprintReadWrite, Category = "Resources")
	float ItemDropRateMultiplier = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Resources")
	float ResourceGainMultiplier = 1.0f;

	// AI Behavior Adjustments
	UPROPERTY(BlueprintReadWrite, Category = "AI")
	float AIDelayBetweenAttacks = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	float AIAccuracyPenalty = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "AI")
	bool bReduceGroupAggression = false;
};

/**
 * DDA Profile (preset configurations)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FName ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FText Description;

	// Skill rating range this profile applies to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	float MinSkillRating = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	float MaxSkillRating = 100.0f;

	// Base parameters for this profile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FHarmoniaDDAParameters BaseParameters;
};

/**
 * Delegate for DDA adjustments
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDDAParametersChanged, const FHarmoniaDDAParameters&, NewParameters);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerSkillRatingChanged, float, OldRating, float, NewRating);

/**
 * Harmonia Dynamic Difficulty Adjustment Subsystem
 * 
 * Advanced DDA system that analyzes player performance and adjusts game difficulty
 * in real-time to maintain optimal challenge and engagement.
 *
 * Features:
 * - Player performance tracking (deaths, victories, skill actions)
 * - Skill rating calculation
 * - Invisible difficulty adjustments (no UI indication)
 * - Multiple adjustment profiles
 * - Smooth transitions between difficulty levels
 * - Boss encounter specific adjustments
 * - Frustration detection and mitigation
 * - Skilled player detection and challenge increase
 *
 * Philosophy:
 * - Adjustments should be invisible to the player
 * - Never make the game trivial, just remove unfair spikes
 * - Skilled players should be challenged, not punished
 * - Struggling players should get subtle assistance
 */
UCLASS(Config = Game)
class HARMONIAKIT_API UHarmoniaDynamicDifficultySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ============================================================================
	// Performance Tracking
	// ============================================================================

	/**
	 * Report a player death
	 * @param DeathCause - Optional tag describing death cause
	 * @param KillerInfo - Optional info about what killed the player
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportDeath(FGameplayTag DeathCause = FGameplayTag(), const FString& KillerInfo = TEXT(""));

	/**
	 * Report a player victory (enemy killed)
	 * @param HealthRemaining - Player health percent when enemy died
	 * @param CombatDuration - How long the fight lasted
	 * @param EnemyType - Tag identifying enemy type
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportVictory(float HealthRemaining, float CombatDuration, FGameplayTag EnemyType = FGameplayTag());

	/**
	 * Report a successful parry
	 * @param bPerfect - True if timing was perfect
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportParry(bool bPerfect = false);

	/**
	 * Report a failed parry (got hit while attempting)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportParryFailure();

	/**
	 * Report a successful dodge
	 * @param bPerfect - True if I-frame was used
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportDodge(bool bPerfect = false);

	/**
	 * Report a failed dodge (got hit during/after dodge)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportDodgeFailure();

	/**
	 * Report damage dealt by player
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportDamageDealt(float Damage);

	/**
	 * Report damage taken by player
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportDamageTaken(float Damage);

	/**
	 * Report player hit (accuracy tracking)
	 * @param bHit - True if attack connected
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportAttackAttempt(bool bHit);

	/**
	 * Report boss encounter started
	 * @param BossID - Unique boss identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportBossEncounterStarted(FName BossID);

	/**
	 * Report boss defeated
	 * @param BossID - Boss that was defeated
	 * @param Attempts - Number of attempts to defeat
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ReportBossDefeated(FName BossID, int32 Attempts);

	// ============================================================================
	// DDA Parameters
	// ============================================================================

	/**
	 * Get current DDA parameters
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	FHarmoniaDDAParameters GetCurrentParameters() const;

	/**
	 * Get specific multiplier (convenience function)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetEnemyHealthMultiplier() const { return CurrentParameters.EnemyHealthMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetEnemyDamageMultiplier() const { return CurrentParameters.EnemyDamageMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetPlayerDamageMultiplier() const { return CurrentParameters.PlayerDamageMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetParryWindowMultiplier() const { return CurrentParameters.ParryWindowMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetDodgeIFrameMultiplier() const { return CurrentParameters.DodgeIFrameMultiplier; }

	/**
	 * Get combined multiplier (DDA * manual difficulty settings)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetCombinedEnemyHealthMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetCombinedEnemyDamageMultiplier() const;

	// ============================================================================
	// Player Skill Rating
	// ============================================================================

	/**
	 * Get player's current skill rating (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	float GetPlayerSkillRating() const;

	/**
	 * Get player's skill tier (for UI)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	FText GetPlayerSkillTier() const;

	/**
	 * Get full performance metrics
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	FHarmoniaPerformanceMetrics GetPerformanceMetrics() const;

	// ============================================================================
	// System Control
	// ============================================================================

	/**
	 * Enable/disable DDA system
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void SetDDAEnabled(bool bEnabled);

	/**
	 * Check if DDA is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	bool IsDDAEnabled() const { return bDDAEnabled; }

	/**
	 * Force recalculation of DDA parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ForceRecalculate();

	/**
	 * Reset all performance metrics (new game)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void ResetMetrics();

	/**
	 * Load profiles from DataTable
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void LoadProfilesFromDataTable(UDataTable* DataTable);

	// ============================================================================
	// Save/Load
	// ============================================================================

	/**
	 * Get metrics data for saving
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|DDA")
	FHarmoniaPerformanceMetrics GetMetricsForSave() const { return Metrics; }

	/**
	 * Set metrics from save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DDA")
	void SetMetricsFromSave(const FHarmoniaPerformanceMetrics& SavedMetrics);

	// ============================================================================
	// Events
	// ============================================================================

	/** Fired when DDA parameters change */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|DDA")
	FOnDDAParametersChanged OnDDAParametersChanged;

	/** Fired when player skill rating changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|DDA")
	FOnPlayerSkillRatingChanged OnPlayerSkillRatingChanged;

protected:
	/** Recalculate all DDA parameters based on current metrics */
	void RecalculateParameters();

	/** Calculate player skill rating from metrics */
	float CalculateSkillRating() const;

	/** Interpolate parameters between profiles based on skill rating */
	FHarmoniaDDAParameters InterpolateParameters(float SkillRating) const;

	/** Check for frustration indicators */
	bool IsPlayerFrustrated() const;

	/** Check if player is skilled */
	bool IsPlayerSkilled() const;

	/** Update metrics after time passes */
	void UpdateTimeBasedMetrics();

	/** Tick function for periodic updates */
	void TickDDA();

private:
	/** Current performance metrics */
	UPROPERTY()
	FHarmoniaPerformanceMetrics Metrics;

	/** Current DDA parameters */
	UPROPERTY()
	FHarmoniaDDAParameters CurrentParameters;

	/** Target parameters (for smooth transitions) */
	UPROPERTY()
	FHarmoniaDDAParameters TargetParameters;

	/** DDA Profiles */
	UPROPERTY()
	TArray<FHarmoniaDDAProfile> Profiles;

	/** Is DDA enabled */
	UPROPERTY(Config)
	bool bDDAEnabled = true;

	/** Transition speed for parameter changes */
	UPROPERTY(Config)
	float ParameterTransitionSpeed = 0.1f;

	/** Minimum time between recalculations */
	UPROPERTY(Config)
	float RecalculationCooldown = 30.0f;

	/** Timer handle for periodic updates */
	FTimerHandle TickTimerHandle;

	/** Last recalculation time */
	float LastRecalculationTime = 0.0f;

	/** Session start time */
	float SessionStartTime = 0.0f;

	/** Current boss encounter (if any) */
	FName CurrentBossEncounter;

	/** Deaths during current boss encounter */
	int32 BossEncounterDeaths = 0;

	// Tracking arrays for rate calculations
	int32 ParryAttempts = 0;
	int32 ParrySuccesses = 0;
	int32 DodgeAttempts = 0;
	int32 DodgeSuccesses = 0;
	int32 AttackAttempts = 0;
	int32 AttackHits = 0;
};
