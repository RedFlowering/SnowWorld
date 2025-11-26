// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaCurrencyDataAsset.generated.h"

/**
 * 통화 데이터 에셋
 * 각 통화 타입의 표시 정보, 제한, 변환 비율 등을 정의
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaCurrencyDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaCurrencyDataAsset();

	//~ 통화 기본 정보
	/** 통화 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	EHarmoniaCurrencyType CurrencyType;

	/** 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	FText Description;

	/** 통화 색상 (UI 표시용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	FLinearColor CurrencyColor;

	/** 통화 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	//~ 통화 제한
	/** 최대 소지량 (0 = 무제한) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Limits", meta = (ClampMin = "0"))
	int32 MaxCarryAmount;

	/** 사망 시 드롭 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death")
	bool bCanDropOnDeath;

	/** 사망 시 드롭 비율 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Death", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bCanDropOnDeath"))
	float DropPercentage;

	/** 거래 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Trade")
	bool bIsTradeable;

	/** 다른 통화로의 변환 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Conversion")
	TMap<EHarmoniaCurrencyType, float> ConversionRates;

	//~ 유틸리티 함수
	/** 수량이 포함된 표시 텍스트 반환 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	FText GetDisplayTextWithAmount(int32 Amount) const;

	/** 소지 한도 이내인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool IsWithinCarryLimit(int32 Amount) const;

	/** 다른 통화로 변환 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 ConvertTo(EHarmoniaCurrencyType TargetCurrency, int32 Amount) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
