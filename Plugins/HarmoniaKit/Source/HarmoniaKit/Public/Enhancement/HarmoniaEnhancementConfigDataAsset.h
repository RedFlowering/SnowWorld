// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaEnhancementSystemDefinitions.h"
#include "HarmoniaEnhancementConfigDataAsset.generated.h"

/**
 * 강화 ?�스???�정 ?�이???�셋
 * Enhancement System Configuration Data Asset
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaEnhancementConfigDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// General Settings
	/** 최�? 강화 ?�벨 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|General", meta = (ClampMin = "1", ClampMax = "30"))
	int32 MaxEnhancementLevel = 15;

	/** 최�? ?�켓 ??*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|General", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxSockets = 3;

	// Pity System Settings
	/** 기본 천장 ?�계�?*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Pity", meta = (ClampMin = "1"))
	int32 BasePityThreshold = 10;

	/** ?�벨??천장 증�???*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Pity", meta = (ClampMin = "0"))
	int32 PityThresholdPerLevel = 1;

	/** ?�패???�공�?보너??(0.0 = 비활?�화) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Pity", meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float PityBonusPerFailure = 0.01f;

	// Enhancement Level Definitions
	/** 기본 강화 ?�벨 ?�의 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Levels")
	TArray<FHarmoniaEnhancementLevelConfig> DefaultEnhancementLevels;

	// Enchantment Definitions
	/** 기본 마법부???�의 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Enchantments")
	TArray<FEnchantmentDefinition> DefaultEnchantments;

	// Cost Scaling
	/** ?�벨??비용 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Cost", meta = (ClampMin = "1.0"))
	float CostMultiplierPerLevel = 1.5f;

	/** 기본 강화 비용 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Cost")
	int64 BaseCurrencyCost = 1000;

	// UI Settings
	/** 강화 결과 ?�시 ?�간 (�? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|UI", meta = (ClampMin = "0.0"))
	float ResultDisplayDuration = 2.0f;

	/** 강화 ?�니메이???�생 ?�간 (�? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|UI", meta = (ClampMin = "0.0"))
	float EnhancementAnimationDuration = 1.5f;

	/** 강화 ?�공 ?�펙???�그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|UI")
	FGameplayTag SuccessEffectTag;

	/** 강화 ?�패 ?�펙???�그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|UI")
	FGameplayTag FailureEffectTag;

	/** ?�이???�괴 ?�펙???�그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|UI")
	FGameplayTag DestructionEffectTag;

public:
	UHarmoniaEnhancementConfigDataAsset()
	{
		// Initialize default enhancement levels
		InitializeDefaultLevels();
	}

	/** 기본 강화 ?�벨 초기??*/
	void InitializeDefaultLevels()
	{
		DefaultEnhancementLevels.Reset();

		// +1 ~ +5: High success rate, no penalties
		for (int32 i = 1; i <= 5; ++i)
		{
			FHarmoniaEnhancementLevelConfig Level;
			Level.Level = i;
			Level.SuccessChance = 1.0f - (i * 0.05f);  // 95% -> 75%
			Level.GreatSuccessChance = 0.1f;
			Level.LevelDownChance = 0.0f;
			Level.DestructionChance = 0.0f;
			Level.StatMultiplier = 1.0f + (i * 0.03f);  // +3% per level
			Level.RequiredCurrency = static_cast<int32>(BaseCurrencyCost * FMath::Pow(CostMultiplierPerLevel, i - 1));
			DefaultEnhancementLevels.Add(Level);
		}

		// +6 ~ +10: Medium success rate, downgrade possible
		for (int32 i = 6; i <= 10; ++i)
		{
			FHarmoniaEnhancementLevelConfig Level;
			Level.Level = i;
			Level.SuccessChance = 0.7f - ((i - 6) * 0.05f);  // 70% -> 50%
			Level.GreatSuccessChance = 0.05f;
			Level.LevelDownChance = 0.1f + ((i - 6) * 0.05f);  // 10% -> 30%
			Level.DestructionChance = 0.0f;
			Level.StatMultiplier = 1.0f + (i * 0.05f);  // +5% per level
			Level.RequiredCurrency = static_cast<int32>(BaseCurrencyCost * FMath::Pow(CostMultiplierPerLevel, i - 1));
			DefaultEnhancementLevels.Add(Level);
		}

		// +11 ~ +15: Low success rate, destruction possible
		for (int32 i = 11; i <= 15; ++i)
		{
			FHarmoniaEnhancementLevelConfig Level;
			Level.Level = i;
			Level.SuccessChance = 0.45f - ((i - 11) * 0.05f);  // 45% -> 25%
			Level.GreatSuccessChance = 0.02f;
			Level.LevelDownChance = 0.3f;
			Level.DestructionChance = 0.05f + ((i - 11) * 0.02f);  // 5% -> 13%
			Level.StatMultiplier = 1.0f + (i * 0.07f);  // +7% per level
			Level.RequiredCurrency = static_cast<int32>(BaseCurrencyCost * FMath::Pow(CostMultiplierPerLevel, i - 1));
			DefaultEnhancementLevels.Add(Level);
		}
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("HarmoniaEnhancementConfig"), GetFName());
	}
};
