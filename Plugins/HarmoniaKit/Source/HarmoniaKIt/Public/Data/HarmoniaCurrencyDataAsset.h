// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaCurrencyDataAsset.generated.h"

/**
 * Data asset defining a currency type and its properties
 * This allows designers to create and configure different currencies without code changes
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaCurrencyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaCurrencyDataAsset();

	/** Unique identifier for this currency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	EHarmoniaCurrencyType CurrencyType;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	FText DisplayName;

	/** Detailed description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency", meta = (MultiLine = true))
	FText Description;

	/** Icon for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Color tint for UI and VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	FLinearColor CurrencyColor;

	/** Maximum amount a player can carry (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency", meta = (ClampMin = "0"))
	int32 MaxCarryAmount;

	/** Can this currency be dropped on death? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Mechanics")
	bool bCanDropOnDeath;

	/** Percentage dropped on death (if droppable) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death Mechanics", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bCanDropOnDeath"))
	float DropPercentage;

	/** Can be traded with other players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trading")
	bool bIsTradeable;

	/** Conversion rate to other currencies (for shops/trading) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trading")
	TMap<EHarmoniaCurrencyType, float> ConversionRates;

	/** Particle system for currency pickup VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<UParticleSystem> PickupEffect;

	/** Sound effect for currency pickup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> PickupSound;

	/** Particle system for memory echo VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual|Memory Echo")
	TSoftObjectPtr<UParticleSystem> MemoryEchoEffect;

	/** Sound for memory echo ambient loop */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|Memory Echo")
	TSoftObjectPtr<USoundBase> MemoryEchoSound;

	/** Get display text with amount */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	FText GetDisplayTextWithAmount(int32 Amount) const;

	/** Check if amount is within carry limit */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool IsWithinCarryLimit(int32 Amount) const;

	/** Convert this currency to another type */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	int32 ConvertTo(EHarmoniaCurrencyType TargetCurrency, int32 Amount) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
