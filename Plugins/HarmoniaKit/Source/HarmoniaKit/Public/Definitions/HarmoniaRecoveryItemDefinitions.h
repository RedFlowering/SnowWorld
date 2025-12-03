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
 * ?�복 ?�이???�??
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryItemType : uint8
{
	// 공명 ?�편 - 체크?�인?�에??충전?�는 기본 ?�복 ?�이??
	ResonanceShard UMETA(DisplayName = "Resonance Shard"),

	// ?�어붙�? ?�간???�송??- ?�간 ??�� ?�복 (?��?)
	FrozenTimeSnowflake UMETA(DisplayName = "Frozen Time Snowflake"),

	// ?�천 보온�?- 지???�복 + 버프
	ThermalSpringFlask UMETA(DisplayName = "Thermal Spring Flask"),

	// ?�명??루�??�센??- ?�치??범위 ?�복
	LifeLuminescence UMETA(DisplayName = "Life Luminescence"),

	// 커스?� - ?�장??
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Recovery Effect Type
 * ?�복 ?�과 ?�??
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryEffectType : uint8
{
	// 즉시 ?�복 (Instant)
	Instant UMETA(DisplayName = "Instant"),

	// 지???�복 (HoT - Heal over Time)
	OverTime UMETA(DisplayName = "Over Time"),

	// ?�간 ??�� (최근 ?�해 복구)
	TimeReversal UMETA(DisplayName = "Time Reversal"),

	// ?�치??범위 ?�복
	AreaDeployable UMETA(DisplayName = "Area Deployable")
};

/**
 * Resonance Shard Variant
 * 공명 ?�편 ?�상�??�과
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceShardVariant
{
	GENERATED_BODY()

	// 공명 주파??(?�상)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	EHarmoniaResonanceFrequency Frequency = EHarmoniaResonanceFrequency::Azure;

	// ?�편 ?�름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardName;

	// ?�편 ?�명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardDescription;

	// 체력 ?�복??(%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	// 추�? ?�과 (GE)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	// ?�편 ?�상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	FLinearColor ShardColor = FLinearColor::Blue;

	// ?�용 ??VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	// ?�용 ??SFX
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
 * ?�복 ?�이???�정 (?�사??가?�한 기본 ?�래??
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemConfig
{
	GENERATED_BODY()

	// ?�이???�??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	// ?�이???�름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemName;

	// ?�이???�명
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemDescription;

	// ?�이???�이�?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UTexture2D> ItemIcon;

	// 최�? 보유 ?�수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 MaxCharges = 5;

	// 초기 충전 ?�수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 InitialCharges = 5;

	// 체크?�인?�에??충전 가???��?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	bool bRechargeableAtCheckpoint = true;

	// ?�용 ?�간 (?�전 ?�간, �?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage", meta = (ClampMin = "0.0"))
	float UsageDuration = 1.5f;

	// ?�용 �??�동 ??취소 ?��?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnMovement = true;

	// ?�용 �??�격 ??취소 ?��?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnDamage = true;

	// ?�복 ?�과 ?�??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	EHarmoniaRecoveryEffectType EffectType = EHarmoniaRecoveryEffectType::Instant;

	// 체력 ?�복??(고정�?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	float HealthRecoveryAmount = 0.0f;

	// 체력 ?�복??(최�? 체력 ?��?%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	// 지???�복 ?�간 (HoT??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0"))
	float RecoveryDuration = 3.0f;

	// ?�용??Gameplay Effect (버프, 추�? ?�과 ??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

	// ?�용 ???�니메이??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	// ?�용 ??VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	// ?�용 ??SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Audio")
	TObjectPtr<USoundBase> UsageSound;

	// ?�용 ?�패 ??SFX
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
 * ?�간 ??�� ?�이??(Frozen Time Snowflake??
 */
USTRUCT(BlueprintType)
struct FHarmoniaTimeReversalSnapshot
{
	GENERATED_BODY()

	// ?�냅???�간
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Timestamp = 0.0f;

	// 체력
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Health = 0.0f;

	// ?�치
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FVector Location = FVector::ZeroVector;

	// ?�전
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Deployable Recovery Area Configuration
 * ?�치???�복 구역 ?�정 (Life Luminescence??
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeployableRecoveryConfig
{
	GENERATED_BODY()

	// ?�복 구역 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float RecoveryRadius = 300.0f;

	// ?�복 구역 지???�간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float Duration = 60.0f;

	// ?�당 ?�복??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	float HealthPerTick = 5.0f;

	// ?�복 ??간격 (�?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.1"))
	float TickInterval = 1.0f;

	// ?�치 ???�성??Actor ?�래??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	TSubclassOf<AActor> DeployableActorClass;

	// ?�복 구역 VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Visual")
	TObjectPtr<UNiagaraSystem> AreaVFX;

	// ?�복 구역 SFX (루프)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Audio")
	TObjectPtr<USoundBase> AreaSound;

	FHarmoniaDeployableRecoveryConfig()
	{
	}
};

/**
 * Recovery Item Runtime State
 * ?�복 ?�이???��????�태 (?�??로드??
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemState
{
	GENERATED_BODY()

	// ?�이???�??
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	// ?�재 충전 ?�수
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 CurrentCharges = 0;

	// 최�? 충전 ?�수 (강화�?증�? 가??
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 MaxCharges = 5;

	// 공명 ?�편 주파??(ResonanceShard ?�용)
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	// 마�?�??�용 ?�간
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
 * ?�복 ?�이???�용 결과
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemUseResult
{
	GENERATED_BODY()

	// ?�용 ?�공 ?��?
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	bool bSuccess = false;

	// ?�복??체력
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	float HealthRecovered = 0.0f;

	// ?��? 충전 ?�수
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 RemainingCharges = 0;

	// ?�패 ?�유
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	FText FailureReason;
};
