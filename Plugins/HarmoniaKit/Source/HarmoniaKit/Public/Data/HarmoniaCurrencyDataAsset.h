// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaCurrencyDataAsset.generated.h"

/**
 * ?�화 ?�이???�셋
 * �??�화 ?�?�의 ?�시 ?�보, ?�한, 변??비율 ?�을 ?�의
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaCurrencyDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaCurrencyDataAsset();

	//~ ?�화 기본 ?�보
	/** ?�화 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	EHarmoniaCurrencyType CurrencyType;

	/** ?�시 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText DisplayName;

	/** ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText Description;

	/** ?�화 ?�상 (UI ?�시?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	FLinearColor CurrencyColor;

	/** ?�화 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	//~ ?�화 ?�한
	/** 최�? ?��???(0 = 무제?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Limits", meta = (ClampMin = "0"))
	int32 MaxCarryAmount;

	/** ?�망 ???�롭 가???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death")
	bool bCanDropOnDeath;

	/** ?�망 ???�롭 비율 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bCanDropOnDeath"))
	float DropPercentage;

	/** 거래 가???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Trade")
	bool bIsTradeable;

	/** ?�른 ?�화로의 변??비율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Conversion")
	TMap<EHarmoniaCurrencyType, float> ConversionRates;

	//~ ?�틸리티 ?�수
	/** ?�량???�함???�시 ?�스??반환 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	FText GetDisplayTextWithAmount(int32 Amount) const;

	/** ?��? ?�도 ?�내?��? ?�인 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool IsWithinCarryLimit(int32 Amount) const;

	/** ?�른 ?�화�?변??*/
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 ConvertTo(EHarmoniaCurrencyType TargetCurrency, int32 Amount) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
