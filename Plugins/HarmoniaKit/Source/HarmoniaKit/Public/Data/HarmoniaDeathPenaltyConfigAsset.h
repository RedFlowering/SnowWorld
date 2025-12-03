// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaDeathPenaltyConfigAsset.h
 * @brief Death penalty configuration data asset
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaDeathPenaltyConfigAsset.generated.h"

/**
 * @class UHarmoniaDeathPenaltyConfigAsset
 * @brief Death penalty configuration data asset
 * 
 * Defines currency drops, memory echo, and difficulty adjustments on death.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDeathPenaltyConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDeathPenaltyConfigAsset();

	//~ Basic Settings
	
	/** Enable death penalty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty")
	bool bEnableDeathPenalty;

	/** Show memory echo marker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowMemoryEchoMarker;

	/** Show distance to memory echo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowDistanceToMemoryEcho;

	/** Memory echo marker color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo|Visual")
	FLinearColor MemoryEchoMarkerColor;

	/** Difficulty multiplier (applies to drop/loss rates) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Difficulty", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DifficultyMultiplier;

	//~ Detailed Configuration
	
	/** Full death penalty configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	FHarmoniaDeathPenaltyConfig Config;

	//~ Utility Functions
	
	/** Get drop percentage for specific currency */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyDropPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** Get permanent loss percentage for specific currency */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyPermanentLossPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** Check if specific currency has drop configuration */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	bool HasCurrencyDropConfig(EHarmoniaCurrencyType CurrencyType) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
