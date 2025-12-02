// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaDeathPenaltyConfigAsset.generated.h"

/**
 * ?¬ë§ ?˜ë„???¤ì • ?°ì´???ì…‹
 * ?¬ë§ ???µí™” ?œë¡­, ë©”ëª¨ë¦??ì½”, ?œì´??ì¡°ì ˆ ?±ì„ ?•ì˜
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDeathPenaltyConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDeathPenaltyConfigAsset();

	//~ ê¸°ë³¸ ?¤ì •
	/** ?¬ë§ ?˜ë„???œì„±???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty")
	bool bEnableDeathPenalty;

	/** ë©”ëª¨ë¦??ì½” ë§ˆì»¤ ?œì‹œ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowMemoryEchoMarker;

	/** ë©”ëª¨ë¦??ì½”ê¹Œì???ê±°ë¦¬ ?œì‹œ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowDistanceToMemoryEcho;

	/** ë©”ëª¨ë¦??ì½” ë§ˆì»¤ ?‰ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo|Visual")
	FLinearColor MemoryEchoMarkerColor;

	/** ?œì´??ë°°ìœ¨ (?œë¡­/?ì‹¤ë¥ ì— ?ìš©) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Difficulty", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DifficultyMultiplier;

	//~ ?ì„¸ ?¤ì •
	/** ?„ì²´ ?¬ë§ ?˜ë„???¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	FHarmoniaDeathPenaltyConfig Config;

	//~ ? í‹¸ë¦¬í‹° ?¨ìˆ˜
	/** ?¹ì • ?µí™”???œë¡­ ë¹„ìœ¨ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyDropPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** ?¹ì • ?µí™”???êµ¬ ?ì‹¤ ë¹„ìœ¨ ë°˜í™˜ */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyPermanentLossPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** ?¹ì • ?µí™”???€???œë¡­ ?¤ì •???ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	bool HasCurrencyDropConfig(EHarmoniaCurrencyType CurrencyType) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
