// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaDDAConfigDataAsset.h
 * @brief Dynamic Difficulty Adjustment (DDA) configuration data asset
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HarmoniaDynamicDifficultySubsystem.h"
#include "HarmoniaDDAConfigDataAsset.generated.h"

/**
 * @struct FHarmoniaDDAWeightConfig
 * @brief DDA weight configuration
 * 
 * Defines how each metric affects skill rating.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAWeightConfig
{
	GENERATED_BODY()

	/** Death count weight (higher = more skill impact from deaths) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathWeight = 0.25f;

	/** Remaining health on victory weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VictoryHealthWeight = 0.15f;

	/** Parry success rate weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryWeight = 0.20f;

	/** Dodge success rate weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeWeight = 0.15f;

	/** Accuracy weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccuracyWeight = 0.10f;

	/** Damage ratio (dealt/received) weight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageRatioWeight = 0.15f;
};

/**
 * @struct FHarmoniaDDAThresholdConfig
 * @brief DDA threshold configuration
 * 
 * Skill rating thresholds for profile transitions.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAThresholdConfig
{
	GENERATED_BODY()

	/** Beginner profile max skill rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 BeginnerMaxRating = 25;

	/** Learning profile max skill rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 LearningMaxRating = 45;

	/** Standard profile max skill rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 StandardMaxRating = 65;

	/** Skilled profile max skill rating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 SkilledMaxRating = 85;

	// 85+ is Master profile
};

/**
 * @struct FHarmoniaFrustrationConfig
 * @brief Frustration detection configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaFrustrationConfig
{
	GENERATED_BODY()

	/** Consecutive death threshold for frustration detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 ConsecutiveDeathThreshold = 3;

	/** Deaths per hour threshold for frustration detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 DeathsPerHourThreshold = 5;

	/** Assist bonus multiplier when frustrated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float FrustrationAssistMultiplier = 1.2f;

	/** Wins required to recover from frustration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RecoveryWinsRequired = 2;
};

/**
 * @class UHarmoniaDDAConfigDataAsset
 * @brief DDA configuration data asset
 * 
 * Manages all dynamic difficulty adjustment settings in a data-driven manner.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDDAConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDDAConfigDataAsset();

	// ============================================================================
	// Weights and Thresholds
	// ============================================================================

	/** Skill rating calculation weights */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Weights")
	FHarmoniaDDAWeightConfig Weights;

	/** Profile transition thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Thresholds")
	FHarmoniaDDAThresholdConfig Thresholds;

	/** Frustration detection settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Frustration")
	FHarmoniaFrustrationConfig FrustrationConfig;

	// ============================================================================
	// DDA Profiles
	// ============================================================================

	/** Beginner profile (easiest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile BeginnerProfile;

	/** Learning profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile LearningProfile;

	/** Standard profile (default) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile StandardProfile;

	/** Skilled profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile SkilledProfile;

	/** Master profile (hardest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile MasterProfile;

	// ============================================================================
	// Advanced Settings
	// ============================================================================

	/** Skill rating change speed (0.1 = slow, 1.0 = fast) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SkillRatingChangeSpeed = 0.5f;

	/** Metrics retention time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "300", ClampMax = "7200"))
	float MetricsRetentionTime = 3600.0f; // 1 hour

	/** Enable DDA for boss encounters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bEnableDDAForBosses = true;

	/** Use average skill rating in multiplayer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bUseAverageInMultiplayer = true;

	// ============================================================================
	// Utility Functions
	// ============================================================================

	/** Get profile for given skill rating */
	UFUNCTION(BlueprintPure, Category = "DDA")
	const FHarmoniaDDAProfile& GetProfileForSkillRating(float SkillRating) const;

	/** Get profile name for given skill rating */
	UFUNCTION(BlueprintPure, Category = "DDA")
	FString GetProfileName(float SkillRating) const;
};
