// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "System/HarmoniaObjectPoolSubsystem.h"
#include "HarmoniaObjectPoolConfigDataAsset.generated.h"

/**
 * 오브젝트 풀 전역 설정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPoolGlobalSettings
{
	GENERATED_BODY()

	/** 전역 풀 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bEnablePooling = true;

	/** 풀 통계 수집 활성화 (디버그용, 프로덕션에서는 비활성화 권장) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bCollectStatistics = true;

	/** 풀 축소 검사 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global", meta = (ClampMin = "1.0", ClampMax = "300.0"))
	float ShrinkCheckInterval = 30.0f;

	/** 풀 예열 (게임 시작 시 미리 생성) 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	bool bWarmUpOnStart = true;

	/** 풀에서 꺼낸 Actor의 기본 위치 (화면 밖) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global")
	FVector HiddenLocation = FVector(0.0f, 0.0f, -100000.0f);
};

/**
 * 오브젝트 풀 설정 데이터 에셋
 * 
 * 오브젝트 풀링 시스템의 전역 설정 및 미리 정의된 풀 구성을 관리합니다.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaObjectPoolConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaObjectPoolConfigDataAsset();

	/** 전역 풀 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Global")
	FHarmoniaPoolGlobalSettings GlobalSettings;

	/** 미리 정의된 풀 구성 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool|Configs")
	TArray<FHarmoniaPoolConfig> PoolConfigs;

	/** 태그로 풀 구성 찾기 */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByTag(FGameplayTag PoolTag, FHarmoniaPoolConfig& OutConfig) const;

	/** 클래스로 풀 구성 찾기 */
	UFUNCTION(BlueprintPure, Category = "Pool")
	bool GetPoolConfigByClass(TSubclassOf<AActor> ActorClass, FHarmoniaPoolConfig& OutConfig) const;
};
