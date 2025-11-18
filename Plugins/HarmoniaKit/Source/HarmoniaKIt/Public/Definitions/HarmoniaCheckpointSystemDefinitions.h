// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaCheckpointSystemDefinitions.generated.h"

/**
 * Checkpoint State
 * 체크포인트 상태
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointState : uint8
{
	// 비활성화 - 아직 발견하지 못함
	Inactive UMETA(DisplayName = "Inactive"),

	// 활성화됨 - 발견했지만 공명하지 않음
	Activated UMETA(DisplayName = "Activated"),

	// 공명 중 - 현재 플레이어가 공명하고 있음
	Resonating UMETA(DisplayName = "Resonating")
};

/**
 * Resonance Frequency Type
 * 공명 주파수 타입 - 각 크리스탈의 고유한 특성
 */
UENUM(BlueprintType)
enum class EHarmoniaResonanceFrequency : uint8
{
	// 푸른 공명 - 차분하고 평화로운
	Azure UMETA(DisplayName = "Azure (Blue)"),

	// 붉은 공명 - 열정적이고 강렬한
	Crimson UMETA(DisplayName = "Crimson (Red)"),

	// 녹색 공명 - 생명력과 회복
	Verdant UMETA(DisplayName = "Verdant (Green)"),

	// 금색 공명 - 신성하고 고귀한
	Aurum UMETA(DisplayName = "Aurum (Gold)"),

	// 보라 공명 - 신비롭고 마법적인
	Violet UMETA(DisplayName = "Violet (Purple)"),

	// 흰색 공명 - 순수하고 중립적인
	Luminous UMETA(DisplayName = "Luminous (White)")
};

/**
 * Checkpoint Upgrade Type
 * 체크포인트 강화 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointUpgradeType : uint8
{
	// 회복량 증가
	EnhancedRestoration UMETA(DisplayName = "Enhanced Restoration"),

	// 공명 속도 증가
	FasterResonance UMETA(DisplayName = "Faster Resonance"),

	// 텔레포트 비용 감소
	ReducedTeleportCost UMETA(DisplayName = "Reduced Teleport Cost"),

	// 공명 범위 증가 (주변 플레이어도 회복)
	ExtendedRange UMETA(DisplayName = "Extended Range"),

	// 부활 시 추가 보너스
	RespawnBonus UMETA(DisplayName = "Respawn Bonus")
};

/**
 * Checkpoint Data
 * 체크포인트 저장 데이터
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointData
{
	GENERATED_BODY()

	// 체크포인트 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	// 체크포인트 이름 (UI 표시용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	// 체크포인트 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointDescription;

	// 공명 주파수 (색상/특성)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// 체크포인트 위치
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	// 체크포인트 회전
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	// 활성화 여부
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	bool bActivated = false;

	// 마지막 공명 시간
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FDateTime LastResonanceTime;

	// 강화 레벨
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	TMap<EHarmoniaCheckpointUpgradeType, int32> UpgradeLevels;

	FHarmoniaCheckpointData()
	{
		CheckpointID = NAME_None;
		CheckpointName = FText::GetEmpty();
		CheckpointDescription = FText::GetEmpty();
	}

	bool operator==(const FHarmoniaCheckpointData& Other) const
	{
		return CheckpointID == Other.CheckpointID;
	}
};

/**
 * Resonance Result
 * 공명 결과
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceResult
{
	GENERATED_BODY()

	// 성공 여부
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bSuccess = false;

	// 회복된 체력
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float HealthRestored = 0.0f;

	// 회복된 마나/스태미나
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float ResourceRestored = 0.0f;

	// 리스폰된 적 수
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	int32 EnemiesRespawned = 0;

	// 게임 저장 여부
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bGameSaved = false;

	// 실패 사유
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	FText FailureReason;
};

/**
 * Teleport Result
 * 텔레포트 결과
 */
USTRUCT(BlueprintType)
struct FHarmoniaTeleportResult
{
	GENERATED_BODY()

	// 성공 여부
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	bool bSuccess = false;

	// 텔레포트한 체크포인트 ID
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FName DestinationCheckpointID;

	// 소모된 리소스 (소울, 마나 등)
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	int32 ResourceCost = 0;

	// 실패 사유
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FText FailureReason;
};

/**
 * Checkpoint Configuration
 * 체크포인트 설정
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointConfig
{
	GENERATED_BODY()

	// 공명 시 체력 회복률 (0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRestorationRate = 1.0f;

	// 공명 시 리소스 회복률 (0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ResourceRestorationRate = 1.0f;

	// 공명 소요 시간 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.1"))
	float ResonanceDuration = 3.0f;

	// 공명 중 움직이면 취소 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnMovement = true;

	// 공명 중 피격 시 취소 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnDamage = true;

	// 텔레포트 기본 비용 (소울)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 BaseTeleportCost = 100;

	// 텔레포트 거리당 추가 비용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TeleportCostPerDistance = 0.1f;

	// 죽었을 때 마지막 체크포인트에서 리스폰
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnAtLastCheckpoint = true;

	// 공명 시 주변 적 리스폰
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnEnemiesOnResonance = true;

	// 공명 시 자동 저장
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAutoSaveOnResonance = true;

	// 공명 범위 (멀티플레이 시 주변 플레이어도 회복)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ResonanceRange = 500.0f;

	FHarmoniaCheckpointConfig()
	{
	}
};
