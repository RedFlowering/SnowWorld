// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaDeathPenaltyConfigAsset.generated.h"

/**
 * 사망 페널티 설정 데이터 에셋
 * 사망 시 통화 드롭, 메모리 에코, 난이도 조절 등을 정의
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDeathPenaltyConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDeathPenaltyConfigAsset();

	//~ 기본 설정
	/** 사망 페널티 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty")
	bool bEnableDeathPenalty;

	/** 메모리 에코 마커 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowMemoryEchoMarker;

	/** 메모리 에코까지의 거리 표시 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo")
	bool bShowDistanceToMemoryEcho;

	/** 메모리 에코 마커 색상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Memory Echo|Visual")
	FLinearColor MemoryEchoMarkerColor;

	/** 난이도 배율 (드롭/손실률에 적용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Difficulty", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float DifficultyMultiplier;

	//~ 상세 설정
	/** 전체 사망 페널티 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Penalty|Config")
	FHarmoniaDeathPenaltyConfig Config;

	//~ 유틸리티 함수
	/** 특정 통화의 드롭 비율 반환 */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyDropPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** 특정 통화의 영구 손실 비율 반환 */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	float GetCurrencyPermanentLossPercentage(EHarmoniaCurrencyType CurrencyType) const;

	/** 특정 통화에 대한 드롭 설정이 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Death Penalty")
	bool HasCurrencyDropConfig(EHarmoniaCurrencyType CurrencyType) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
