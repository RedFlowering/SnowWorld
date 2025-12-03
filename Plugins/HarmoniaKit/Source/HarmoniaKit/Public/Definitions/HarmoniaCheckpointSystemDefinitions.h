// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HarmoniaCheckpointSystemDefinitions.generated.h"

/**
 * Checkpoint State
 * 체크?�인???�태
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointState : uint8
{
	// 비활?�화 - ?�직 발견?��? 못함
	Inactive UMETA(DisplayName = "Inactive"),

	// ?�성?�됨 - 발견?��?�?공명?��? ?�음
	Activated UMETA(DisplayName = "Activated"),

	// 공명 �?- ?�재 ?�레?�어가 공명?�고 ?�음
	Resonating UMETA(DisplayName = "Resonating")
};

/**
 * Resonance Frequency Type
 * 공명 주파???�??- �??�리?�탈??고유???�성
 */
UENUM(BlueprintType)
enum class EHarmoniaResonanceFrequency : uint8
{
	// ?�른 공명 - 차분?�고 ?�화로운
	Azure UMETA(DisplayName = "Azure (Blue)"),

	// 붉�? 공명 - ?�정?�이�?강렬??
	Crimson UMETA(DisplayName = "Crimson (Red)"),

	// ?�색 공명 - ?�명?�과 ?�복
	Verdant UMETA(DisplayName = "Verdant (Green)"),

	// 금색 공명 - ?�성?�고 고�???
	Aurum UMETA(DisplayName = "Aurum (Gold)"),

	// 보라 공명 - ?�비�?�� 마법?�인
	Violet UMETA(DisplayName = "Violet (Purple)"),

	// ?�색 공명 - ?�수?�고 중립?�인
	Luminous UMETA(DisplayName = "Luminous (White)")
};

/**
 * Checkpoint Upgrade Type
 * 체크?�인??강화 ?�??
 */
UENUM(BlueprintType)
enum class EHarmoniaCheckpointUpgradeType : uint8
{
	// ?�복??증�?
	EnhancedRestoration UMETA(DisplayName = "Enhanced Restoration"),

	// 공명 ?�도 증�?
	FasterResonance UMETA(DisplayName = "Faster Resonance"),

	// ?�레?�트 비용 감소
	ReducedTeleportCost UMETA(DisplayName = "Reduced Teleport Cost"),

	// 공명 범위 증�? (주�? ?�레?�어???�복)
	ExtendedRange UMETA(DisplayName = "Extended Range"),

	// 부????추�? 보너??
	RespawnBonus UMETA(DisplayName = "Respawn Bonus")
};

/**
 * Checkpoint Data
 * 체크?�인???�???�이??
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointData
{
	GENERATED_BODY()

	// 체크?�인??고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FName CheckpointID;

	// 체크?�인???�름 (UI ?�시??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointName;

	// 체크?�인???�명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FText CheckpointDescription;

	// 공명 주파??(?�상/?�성)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	EHarmoniaResonanceFrequency ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// 체크?�인???�치
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	// 체크?�인???�전
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	// ?�성???��?
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	bool bActivated = false;

	// 마�?�?공명 ?�간
	UPROPERTY(BlueprintReadWrite, Category = "Checkpoint")
	FDateTime LastResonanceTime;

	// 강화 ?�벨
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

	// ?�공 ?��?
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bSuccess = false;

	// ?�복??체력
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float HealthRestored = 0.0f;

	// ?�복??마나/?�태미나
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	float ResourceRestored = 0.0f;

	// 리스?�된 ????
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	int32 EnemiesRespawned = 0;

	// 게임 ?�???��?
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	bool bGameSaved = false;

	// ?�패 ?�유
	UPROPERTY(BlueprintReadWrite, Category = "Resonance")
	FText FailureReason;
};

/**
 * Teleport Result
 * ?�레?�트 결과
 */
USTRUCT(BlueprintType)
struct FHarmoniaTeleportResult
{
	GENERATED_BODY()

	// ?�공 ?��?
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	bool bSuccess = false;

	// ?�레?�트??체크?�인??ID
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FName DestinationCheckpointID;

	// ?�모??리소??(?�울, 마나 ??
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	int32 ResourceCost = 0;

	// ?�패 ?�유
	UPROPERTY(BlueprintReadWrite, Category = "Teleport")
	FText FailureReason;
};

/**
 * Checkpoint Configuration
 * 체크?�인???�정
 */
USTRUCT(BlueprintType)
struct FHarmoniaCheckpointConfig
{
	GENERATED_BODY()

	// 공명 ??체력 ?�복�?(0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRestorationRate = 1.0f;

	// 공명 ??리소???�복�?(0.0 ~ 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ResourceRestorationRate = 1.0f;

	// 공명 ?�요 ?�간 (�?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.1"))
	float ResonanceDuration = 3.0f;

	// 공명 �??�직이�?취소 ?��?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnMovement = true;

	// 공명 �??�격 ??취소 ?��?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bCancelOnDamage = true;

	// ?�레?�트 기본 비용 (?�울)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 BaseTeleportCost = 100;

	// ?�레?�트 거리??추�? 비용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float TeleportCostPerDistance = 0.1f;

	// 죽었????마�?�?체크?�인?�에??리스??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnAtLastCheckpoint = true;

	// 공명 ??주�? ??리스??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRespawnEnemiesOnResonance = true;

	// 공명 ???�동 ?�??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAutoSaveOnResonance = true;

	// 공명 범위 (멀?�플?�이 ??주�? ?�레?�어???�복)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ResonanceRange = 500.0f;

	FHarmoniaCheckpointConfig()
	{
	}
};
