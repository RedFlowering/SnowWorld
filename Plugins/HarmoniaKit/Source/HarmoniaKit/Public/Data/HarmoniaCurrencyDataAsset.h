// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaCurrencyDataAsset.h
 * @brief Currency data asset for defining in-game currencies
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaCurrencyDataAsset.generated.h"

/**
 * @class UHarmoniaCurrencyDataAsset
 * @brief Currency data asset
 * 
 * Defines display info, limits, and conversion rates for each currency type.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaCurrencyDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaCurrencyDataAsset();

	//~ Currency Basic Info
	
	/** Currency type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	EHarmoniaCurrencyType CurrencyType;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText Description;

	/** Currency color for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	FLinearColor CurrencyColor;

	/** Currency icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	//~ Currency Limits
	
	/** Max carry amount (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Limits", meta = (ClampMin = "0"))
	int32 MaxCarryAmount;

	/** Can drop on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death")
	bool bCanDropOnDeath;

	/** Drop percentage on death (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bCanDropOnDeath"))
	float DropPercentage;

	/** Is tradeable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Trade")
	bool bIsTradeable;

	/** Conversion rates to other currencies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Conversion")
	TMap<EHarmoniaCurrencyType, float> ConversionRates;

	//~ Utility Functions
	
	/** Get display text with amount included */
	UFUNCTION(BlueprintPure, Category = "Currency")
	FText GetDisplayTextWithAmount(int32 Amount) const;

	/** Check if amount is within carry limit */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool IsWithinCarryLimit(int32 Amount) const;

	/** Convert to another currency */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 ConvertTo(EHarmoniaCurrencyType TargetCurrency, int32 Amount) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
