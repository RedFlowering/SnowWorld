// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaDeathPenaltyConfigAsset.generated.h"

/**
 * ?�망 ?�널???�정 ?�이???�셋
 * ?�망 ???�화 ?�롭, 메모�??�코, ?�이??조절 ?�을 ?�의
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDeathPenaltyConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDeathPenaltyConfigAsset();

	//~ 기본 ?�정
	/** ?�망 ?�널???�성???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty")
	bool bEnableDeathPenalty;

	/** 메모�??�코 마커 ?�시 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowMemoryEchoMarker;

	/** 메모�??�코까�???거리 ?�시 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowDistanceToMemoryEcho;

	/** 메모�??�코 마커 ?�상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo|Visual")
	FLinearColor MemoryEchoMarkerColor;

	/** ?�이??배율 (?�롭/?�실률에 ?�용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Difficulty", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DifficultyMultiplier;

	//~ ?�세 ?�정
	/** ?�체 ?�망 ?�널???�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	FHarmoniaDeathPenaltyConfig Config;

	//~ ?�틸리티 ?�수
	/** ?�정 ?�화???�롭 비율 반환 */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyDropPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** ?�정 ?�화???�구 ?�실 비율 반환 */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyPermanentLossPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** ?�정 ?�화???�???�롭 ?�정???�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	bool HasCurrencyDropConfig(EHarmoniaCurrencyType CurrencyType) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
