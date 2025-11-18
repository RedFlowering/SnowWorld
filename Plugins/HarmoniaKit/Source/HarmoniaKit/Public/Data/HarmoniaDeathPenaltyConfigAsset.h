// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaDeathPenaltyConfigAsset.generated.h"

/**
 * Data asset defining death penalty rules and configuration
 * This allows designers to tune the death penalty system without code changes
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDeathPenaltyConfigAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDeathPenaltyConfigAsset();

	/** Complete death penalty configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Penalty")
	FHarmoniaDeathPenaltyConfig Config;

	/** Enable death penalty system globally */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Penalty")
	bool bEnableDeathPenalty;

	/** Show UI markers for memory echo locations */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Penalty|UI")
	bool bShowMemoryEchoMarker;

	/** Show distance to memory echo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Penalty|UI")
	bool bShowDistanceToMemoryEcho;

	/** Memory echo marker color */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Penalty|UI", meta = (EditCondition = "bShowMemoryEchoMarker"))
	FLinearColor MemoryEchoMarkerColor;

	/** Difficulty multiplier for penalties (1.0 = normal, 2.0 = double penalties) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Penalty|Difficulty", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DifficultyMultiplier;

	/** Get drop percentage for a specific currency type */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	float GetCurrencyDropPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** Get permanent loss percentage for a specific currency type */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	float GetCurrencyPermanentLossPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** Check if a currency type has drop configuration */
	UFUNCTION(BlueprintCallable, Category = "Death Penalty")
	bool HasCurrencyDropConfig(EHarmoniaCurrencyType CurrencyType) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
