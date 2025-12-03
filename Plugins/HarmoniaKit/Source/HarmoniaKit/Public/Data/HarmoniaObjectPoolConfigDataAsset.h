// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "System/HarmoniaObjectPoolSubsystem.h"
#include "HarmoniaObjectPoolConfigDataAsset.generated.h"

/**
 * ?�브?�트 ?� ?�역 ?�정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPoolGlobalSettings
{
	GENERATED_BODY()

	/** ?�역 ?� ?�성???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bEnablePooling = true;

	/** ?� ?�계 ?�집 ?�성??(?�버그용, ?�로?�션?�서??비활?�화 권장) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bCollectStatistics = true;

	/** ?� 축소 검??주기 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global", meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float ShrinkCheckInterval = 30.0f;

	/** ?� ?�열 (게임 ?�작 ??미리 ?�성) ?�성??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bWarmUpOnStart = true;

	/** ?�?�서 꺼낸 Actor??기본 ?�치 (?�면 �? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	FVector HiddenLocation = FVector(0.0f, 0.0f, -100000.0f);
};

/**
 * ?�브?�트 ?� ?�정 ?�이???�셋
 * 
 * ?�브?�트 ?��??�스?�의 ?�역 ?�정 �?미리 ?�의???� 구성??관리합?�다.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaObjectPoolConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaObjectPoolConfigDataAsset();

	/** ?�역 ?� ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Global")
	FHarmoniaPoolGlobalSettings GlobalSettings;

	/** 미리 ?�의???� 구성 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Configs")
	TArray<FHarmoniaPoolConfig> PoolConfigs;

	/** ?�그�??� 구성 찾기 */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByTag(FGameplayTag PoolTag, FHarmoniaPoolConfig& OutConfig) const;

	/** ?�래?�로 ?� 구성 찾기 */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByClass(TSubclassOf<AActor> ActorClass, FHarmoniaPoolConfig& OutConfig) const;
};
