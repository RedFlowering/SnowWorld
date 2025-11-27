// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HarmoniaDynamicDifficultySubsystem.h"
#include "HarmoniaDDAConfigDataAsset.generated.h"

/**
 * DDA (Dynamic Difficulty Adjustment) 가중치 설정
 * 각 메트릭이 스킬 레이팅에 미치는 영향도를 정의
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAWeightConfig
{
	GENERATED_BODY()

	/** 사망 횟수 가중치 (높을수록 사망이 스킬에 많이 영향) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathWeight = 0.25f;

	/** 승리 시 남은 체력 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VictoryHealthWeight = 0.15f;

	/** 패링 성공률 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryWeight = 0.20f;

	/** 회피 성공률 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeWeight = 0.15f;

	/** 명중률 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccuracyWeight = 0.10f;

	/** 피해 비율 (주는 피해/받는 피해) 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageRatioWeight = 0.15f;
};

/**
 * DDA 임계값 설정
 * 각 프로필로 전환되는 스킬 레이팅 기준
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAThresholdConfig
{
	GENERATED_BODY()

	/** Beginner 프로필 최대 스킬 레이팅 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 BeginnerMaxRating = 25;

	/** Learning 프로필 최대 스킬 레이팅 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 LearningMaxRating = 45;

	/** Standard 프로필 최대 스킬 레이팅 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 StandardMaxRating = 65;

	/** Skilled 프로필 최대 스킬 레이팅 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 SkilledMaxRating = 85;

	// 85 이상은 Master 프로필
};

/**
 * 좌절 감지 설정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaFrustrationConfig
{
	GENERATED_BODY()

	/** 좌절 감지를 위한 연속 사망 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 ConsecutiveDeathThreshold = 3;

	/** 좌절 감지를 위한 시간당 사망 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 DeathsPerHourThreshold = 5;

	/** 좌절 시 적용할 숨겨진 보너스 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float FrustrationAssistMultiplier = 1.2f;

	/** 좌절 후 정상화되기까지 필요한 승리 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RecoveryWinsRequired = 2;
};

/**
 * DDA 설정 데이터 에셋
 * 
 * 동적 난이도 조절 시스템의 모든 설정을 데이터 드리븐 방식으로 관리합니다.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDDAConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDDAConfigDataAsset();

	// ============================================================================
	// 가중치 및 임계값
	// ============================================================================

	/** 스킬 레이팅 계산 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Weights")
	FHarmoniaDDAWeightConfig Weights;

	/** 프로필 전환 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Thresholds")
	FHarmoniaDDAThresholdConfig Thresholds;

	/** 좌절 감지 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Frustration")
	FHarmoniaFrustrationConfig FrustrationConfig;

	// ============================================================================
	// DDA 프로필
	// ============================================================================

	/** Beginner 프로필 (가장 쉬움) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile BeginnerProfile;

	/** Learning 프로필 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile LearningProfile;

	/** Standard 프로필 (기본) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile StandardProfile;

	/** Skilled 프로필 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile SkilledProfile;

	/** Master 프로필 (가장 어려움) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile MasterProfile;

	// ============================================================================
	// 고급 설정
	// ============================================================================

	/** 스킬 레이팅 변화 속도 (0.1 = 느림, 1.0 = 빠름) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SkillRatingChangeSpeed = 0.5f;

	/** 메트릭 기록 보관 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "300", ClampMax = "7200"))
	float MetricsRetentionTime = 3600.0f; // 1시간

	/** 보스전에서 DDA 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bEnableDDAForBosses = true;

	/** 멀티플레이에서 DDA 계산 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bUseAverageInMultiplayer = true;

	// ============================================================================
	// 유틸리티 함수
	// ============================================================================

	/** 스킬 레이팅에 해당하는 프로필 반환 */
	UFUNCTION(BlueprintPure, Category = "DDA")
	const FHarmoniaDDAProfile& GetProfileForSkillRating(float SkillRating) const;

	/** 프로필 이름 반환 */
	UFUNCTION(BlueprintPure, Category = "DDA")
	FString GetProfileName(float SkillRating) const;
};
