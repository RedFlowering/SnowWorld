// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaCurrencyDataAsset.generated.h"

/**
 * ?µí™” ?°ì´???ì…‹
 * ê°??µí™” ?€?…ì˜ ?œì‹œ ?•ë³´, ?œí•œ, ë³€??ë¹„ìœ¨ ?±ì„ ?•ì˜
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaCurrencyDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaCurrencyDataAsset();

	//~ ?µí™” ê¸°ë³¸ ?•ë³´
	/** ?µí™” ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	EHarmoniaCurrencyType CurrencyType;

	/** ?œì‹œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText DisplayName;

	/** ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText Description;

	/** ?µí™” ?‰ìƒ (UI ?œì‹œ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	FLinearColor CurrencyColor;

	/** ?µí™” ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	//~ ?µí™” ?œí•œ
	/** ìµœë? ?Œì???(0 = ë¬´ì œ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Limits", meta = (ClampMin = "0"))
	int32 MaxCarryAmount;

	/** ?¬ë§ ???œë¡­ ê°€???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death")
	bool bCanDropOnDeath;

	/** ?¬ë§ ???œë¡­ ë¹„ìœ¨ (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bCanDropOnDeath"))
	float DropPercentage;

	/** ê±°ë˜ ê°€???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Trade")
	bool bIsTradeable;

	/** ?¤ë¥¸ ?µí™”ë¡œì˜ ë³€??ë¹„ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Conversion")
	TMap<EHarmoniaCurrencyType, float> ConversionRates;

	//~ ? í‹¸ë¦¬í‹° ?¨ìˆ˜
	/** ?˜ëŸ‰???¬í•¨???œì‹œ ?ìŠ¤??ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Currency")
	FText GetDisplayTextWithAmount(int32 Amount) const;

	/** ?Œì? ?œë„ ?´ë‚´?¸ì? ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool IsWithinCarryLimit(int32 Amount) const;

	/** ?¤ë¥¸ ?µí™”ë¡?ë³€??*/
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 ConvertTo(EHarmoniaCurrencyType TargetCurrency, int32 Amount) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
