// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/HarmoniaDynamicDifficultySubsystem.h"
#include "HarmoniaDDAConfigDataAsset.generated.h"

/**
 * DDA (Dynamic Difficulty Adjustment) 가중치 ?�정
 * �?메트�?�� ?�킬 ?�이?�에 미치???�향?��? ?�의
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAWeightConfig
{
	GENERATED_BODY()

	/** ?�망 ?�수 가중치 (?�을?�록 ?�망???�킬??많이 ?�향) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeathWeight = 0.25f;

	/** ?�리 ???��? 체력 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VictoryHealthWeight = 0.15f;

	/** ?�링 ?�공�?가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryWeight = 0.20f;

	/** ?�피 ?�공�?가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeWeight = 0.15f;

	/** 명중�?가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccuracyWeight = 0.10f;

	/** ?�해 비율 (주는 ?�해/받는 ?�해) 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weights", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageRatioWeight = 0.15f;
};

/**
 * DDA ?�계�??�정
 * �??�로?�로 ?�환?�는 ?�킬 ?�이??기�?
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDDAThresholdConfig
{
	GENERATED_BODY()

	/** Beginner ?�로??최�? ?�킬 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 BeginnerMaxRating = 25;

	/** Learning ?�로??최�? ?�킬 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 LearningMaxRating = 45;

	/** Standard ?�로??최�? ?�킬 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 StandardMaxRating = 65;

	/** Skilled ?�로??최�? ?�킬 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds", meta = (ClampMin = "0", ClampMax = "100"))
	int32 SkilledMaxRating = 85;

	// 85 ?�상?� Master ?�로??
};

/**
 * 좌절 감�? ?�정
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaFrustrationConfig
{
	GENERATED_BODY()

	/** 좌절 감�?�??�한 ?�속 ?�망 ?�계�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 ConsecutiveDeathThreshold = 3;

	/** 좌절 감�?�??�한 ?�간???�망 ?�계�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "20"))
	int32 DeathsPerHourThreshold = 5;

	/** 좌절 ???�용???�겨�?보너??배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1.0", ClampMax = "2.0"))
	float FrustrationAssistMultiplier = 1.2f;

	/** 좌절 ???�상?�되기까지 ?�요???�리 ?�수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frustration", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RecoveryWinsRequired = 2;
};

/**
 * DDA ?�정 ?�이???�셋
 * 
 * ?�적 ?�이??조절 ?�스?�의 모든 ?�정???�이???�리�?방식?�로 관리합?�다.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaDDAConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDDAConfigDataAsset();

	// ============================================================================
	// 가중치 �??�계�?
	// ============================================================================

	/** ?�킬 ?�이??계산 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Weights")
	FHarmoniaDDAWeightConfig Weights;

	/** ?�로???�환 ?�계�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Thresholds")
	FHarmoniaDDAThresholdConfig Thresholds;

	/** 좌절 감�? ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Frustration")
	FHarmoniaFrustrationConfig FrustrationConfig;

	// ============================================================================
	// DDA ?�로??
	// ============================================================================

	/** Beginner ?�로??(가???��?) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile BeginnerProfile;

	/** Learning ?�로??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile LearningProfile;

	/** Standard ?�로??(기본) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile StandardProfile;

	/** Skilled ?�로??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile SkilledProfile;

	/** Master ?�로??(가???�려?�) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Profiles")
	FHarmoniaDDAProfile MasterProfile;

	// ============================================================================
	// 고급 ?�정
	// ============================================================================

	/** ?�킬 ?�이??변???�도 (0.1 = ?�림, 1.0 = 빠름) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SkillRatingChangeSpeed = 0.5f;

	/** 메트�?기록 보�? ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced", meta = (ClampMin = "300", ClampMax = "7200"))
	float MetricsRetentionTime = 3600.0f; // 1?�간

	/** 보스?�에??DDA ?�성???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bEnableDDAForBosses = true;

	/** 멀?�플?�이?�서 DDA 계산 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DDA|Advanced")
	bool bUseAverageInMultiplayer = true;

	// ============================================================================
	// ?�틸리티 ?�수
	// ============================================================================

	/** ?�킬 ?�이?�에 ?�당?�는 ?�로??반환 */
	UFUNCTION(BlueprintPure, Category = "DDA")
	const FHarmoniaDDAProfile& GetProfileForSkillRating(float SkillRating) const;

	/** ?�로???�름 반환 */
	UFUNCTION(BlueprintPure, Category = "DDA")
	FString GetProfileName(float SkillRating) const;
};
