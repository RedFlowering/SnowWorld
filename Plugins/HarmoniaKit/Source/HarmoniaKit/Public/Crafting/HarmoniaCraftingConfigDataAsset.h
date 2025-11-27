// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HarmoniaCraftingTypes.h"
#include "HarmoniaCraftingConfigDataAsset.generated.h"

/**
 * Data asset for crafting system configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaCraftingConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Default Recipes
	// ============================================================================

	/** Default recipes to register on startup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipes")
	TArray<FCraftingRecipeData> DefaultRecipes;

	// ============================================================================
	// Difficulty Penalties
	// ============================================================================

	/** Success rate penalty for Trivial difficulty (negative = bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float TrivialDifficultyPenalty = -0.1f;

	/** Success rate penalty for Easy difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float EasyDifficultyPenalty = 0.0f;

	/** Success rate penalty for Normal difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float NormalDifficultyPenalty = 0.1f;

	/** Success rate penalty for Hard difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float HardDifficultyPenalty = 0.2f;

	/** Success rate penalty for Expert difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float ExpertDifficultyPenalty = 0.35f;

	/** Success rate penalty for Master difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float MasterDifficultyPenalty = 0.5f;

	/** Success rate penalty for Legendary difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	float LegendaryDifficultyPenalty = 0.7f;

	// ============================================================================
	// Skill Progression
	// ============================================================================

	/** Base experience required for level 2 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	float BaseExperienceToLevel = 100.0f;

	/** Experience multiplier per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	float ExperienceMultiplierPerLevel = 1.5f;

	/** Maximum crafting skill level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 MaxSkillLevel = 100;

	// ============================================================================
	// Quality Settings
	// ============================================================================

	/** Base quality for new crafters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float BaseQuality = 0.5f;

	/** Quality bonus per skill level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float QualityPerLevel = 0.005f;

	/** Quality bonus per level above requirement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
	float QualityPerLevelAdvantage = 0.02f;

	// ============================================================================
	// Critical Success
	// ============================================================================

	/** Base critical success chance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical")
	float BaseCriticalChance = 0.1f;

	/** Critical success quantity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical")
	float CriticalQuantityMultiplier = 1.5f;

	/** Critical success quality bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical")
	float CriticalQualityBonus = 0.2f;

	/** Critical success experience multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical")
	float CriticalExperienceMultiplier = 1.5f;

	// ============================================================================
	// Failure Settings
	// ============================================================================

	/** Percentage of materials returned on failure (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FailureMaterialRefundRate = 0.0f;

	/** Whether failed crafts still grant experience */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	bool bGrantExpOnFailure = true;

	/** Experience multiplier for failed crafts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure", meta = (EditCondition = "bGrantExpOnFailure"))
	float FailureExperienceMultiplier = 0.25f;
};
