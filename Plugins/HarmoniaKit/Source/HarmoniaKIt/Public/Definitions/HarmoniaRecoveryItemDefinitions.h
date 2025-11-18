// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "Definitions/HarmoniaCheckpointSystemDefinitions.h"
#include "HarmoniaRecoveryItemDefinitions.generated.h"

class UGameplayEffect;
class UNiagaraSystem;

/**
 * Recovery Item Type
 * 회복 아이템 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryItemType : uint8
{
	// 공명 파편 - 체크포인트에서 충전되는 기본 회복 아이템
	ResonanceShard UMETA(DisplayName = "Resonance Shard"),

	// 얼어붙은 시간의 눈송이 - 시간 역행 회복 (희귀)
	FrozenTimeSnowflake UMETA(DisplayName = "Frozen Time Snowflake"),

	// 온천 보온병 - 지속 회복 + 버프
	ThermalSpringFlask UMETA(DisplayName = "Thermal Spring Flask"),

	// 생명의 루미네센스 - 설치형 범위 회복
	LifeLuminescence UMETA(DisplayName = "Life Luminescence"),

	// 커스텀 - 확장용
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Recovery Effect Type
 * 회복 효과 타입
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryEffectType : uint8
{
	// 즉시 회복 (Instant)
	Instant UMETA(DisplayName = "Instant"),

	// 지속 회복 (HoT - Heal over Time)
	OverTime UMETA(DisplayName = "Over Time"),

	// 시간 역행 (최근 피해 복구)
	TimeReversal UMETA(DisplayName = "Time Reversal"),

	// 설치형 범위 회복
	AreaDeployable UMETA(DisplayName = "Area Deployable")
};

/**
 * Resonance Shard Variant
 * 공명 파편 색상별 효과
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceShardVariant
{
	GENERATED_BODY()

	// 공명 주파수 (색상)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	EHarmoniaResonanceFrequency Frequency = EHarmoniaResonanceFrequency::Azure;

	// 파편 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardName;

	// 파편 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardDescription;

	// 체력 회복량 (%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	// 추가 효과 (GE)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	// 파편 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	FLinearColor ShardColor = FLinearColor::Blue;

	// 사용 시 VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	// 사용 시 SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Audio")
	TObjectPtr<USoundBase> UsageSound;

	FHarmoniaResonanceShardVariant()
	{
		ShardName = FText::FromString(TEXT("Azure Resonance Shard"));
		ShardDescription = FText::FromString(TEXT("A fragment imbued with azure resonance energy. Restores health and grants brief defense."));
	}
};

/**
 * Recovery Item Configuration
 * 회복 아이템 설정 (재사용 가능한 기본 클래스)
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemConfig
{
	GENERATED_BODY()

	// 아이템 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemName;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemDescription;

	// 아이템 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UTexture2D> ItemIcon;

	// 최대 보유 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 MaxCharges = 5;

	// 초기 충전 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 InitialCharges = 5;

	// 체크포인트에서 충전 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	bool bRechargeableAtCheckpoint = true;

	// 사용 시간 (시전 시간, 초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage", meta = (ClampMin = "0.0"))
	float UsageDuration = 1.5f;

	// 사용 중 이동 시 취소 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnMovement = true;

	// 사용 중 피격 시 취소 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnDamage = true;

	// 회복 효과 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	EHarmoniaRecoveryEffectType EffectType = EHarmoniaRecoveryEffectType::Instant;

	// 체력 회복량 (고정값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	float HealthRecoveryAmount = 0.0f;

	// 체력 회복량 (최대 체력 대비 %)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	// 지속 회복 시간 (HoT용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0"))
	float RecoveryDuration = 3.0f;

	// 적용할 Gameplay Effect (버프, 추가 효과 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

	// 사용 시 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	// 사용 시 VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	// 사용 시 SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Audio")
	TObjectPtr<USoundBase> UsageSound;

	// 사용 실패 시 SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Audio")
	TObjectPtr<USoundBase> FailureSound;

	FHarmoniaRecoveryItemConfig()
	{
		ItemName = FText::FromString(TEXT("Recovery Item"));
		ItemDescription = FText::FromString(TEXT("A basic recovery item."));
	}
};

/**
 * Time Reversal Data
 * 시간 역행 데이터 (Frozen Time Snowflake용)
 */
USTRUCT(BlueprintType)
struct FHarmoniaTimeReversalSnapshot
{
	GENERATED_BODY()

	// 스냅샷 시간
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Timestamp = 0.0f;

	// 체력
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Health = 0.0f;

	// 위치
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FVector Location = FVector::ZeroVector;

	// 회전
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Deployable Recovery Area Configuration
 * 설치형 회복 구역 설정 (Life Luminescence용)
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeployableRecoveryConfig
{
	GENERATED_BODY()

	// 회복 구역 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float RecoveryRadius = 300.0f;

	// 회복 구역 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float Duration = 60.0f;

	// 틱당 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	float HealthPerTick = 5.0f;

	// 회복 틱 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.1"))
	float TickInterval = 1.0f;

	// 설치 시 생성할 Actor 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	TSubclassOf<AActor> DeployableActorClass;

	// 회복 구역 VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Visual")
	TObjectPtr<UNiagaraSystem> AreaVFX;

	// 회복 구역 SFX (루프)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Audio")
	TObjectPtr<USoundBase> AreaSound;

	FHarmoniaDeployableRecoveryConfig()
	{
	}
};

/**
 * Recovery Item Runtime State
 * 회복 아이템 런타임 상태 (저장/로드용)
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemState
{
	GENERATED_BODY()

	// 아이템 타입
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	// 현재 충전 횟수
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 CurrentCharges = 0;

	// 최대 충전 횟수 (강화로 증가 가능)
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 MaxCharges = 5;

	// 공명 파편 주파수 (ResonanceShard 전용)
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	// 마지막 사용 시간
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	FDateTime LastUsedTime;

	FHarmoniaRecoveryItemState()
	{
	}

	bool operator==(const FHarmoniaRecoveryItemState& Other) const
	{
		return ItemType == Other.ItemType && ShardFrequency == Other.ShardFrequency;
	}
};

/**
 * Recovery Item Use Result
 * 회복 아이템 사용 결과
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemUseResult
{
	GENERATED_BODY()

	// 사용 성공 여부
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	bool bSuccess = false;

	// 회복된 체력
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	float HealthRecovered = 0.0f;

	// 남은 충전 횟수
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 RemainingCharges = 0;

	// 실패 사유
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	FText FailureReason;
};
