// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaRandomItemSystemDefinitions.h"
#include "HarmoniaRandomItemConfigDataAsset.generated.h"

/**
 * Data asset for random item generation configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaRandomItemConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Default Affixes
	// ============================================================================

	/** Default affixes to register */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affixes")
	TArray<FHarmoniaAffixData> DefaultAffixes;

	// ============================================================================
	// Rarity Settings
	// ============================================================================

	/** Rarity weights and affix counts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rarity")
	TArray<FHarmoniaRarityWeight> RarityWeights;

	// ============================================================================
	// Rarity Colors
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor CommonColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor UncommonColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor RareColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor EpicColor = FLinearColor(0.6f, 0.2f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor LegendaryColor = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor MythicColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor UniqueColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	// ============================================================================
	// Generation Settings
	// ============================================================================

	/** Base rarity bonus per item level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float RarityBonusPerLevel = 0.005f;

	/** Maximum rarity bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MaxRarityBonus = 0.5f;

	UHarmoniaRandomItemConfigDataAsset()
	{
		// Default rarity weights will be set up in editor
	}
};
