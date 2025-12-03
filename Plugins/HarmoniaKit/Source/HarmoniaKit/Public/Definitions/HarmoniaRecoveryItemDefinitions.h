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
 * @file HarmoniaRecoveryItemDefinitions.h
 * @brief Recovery item system definitions for healing and restoration items
 * 
 * This file contains enums and structs for the recovery item system,
 * including resonance shards, time reversal items, and deployable healing zones.
 */

/**
 * @enum EHarmoniaRecoveryItemType
 * @brief Types of recovery items available
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryItemType : uint8
{
	/** Resonance Shard - Basic recovery item that charges at checkpoints */
	ResonanceShard UMETA(DisplayName = "Resonance Shard"),

	/** Frozen Time Snowflake - Time reversal recovery (restores to past state) */
	FrozenTimeSnowflake UMETA(DisplayName = "Frozen Time Snowflake"),

	/** Thermal Spring Flask - Gradual recovery with buff effect */
	ThermalSpringFlask UMETA(DisplayName = "Thermal Spring Flask"),

	/** Life Luminescence - Deployable area of effect healing */
	LifeLuminescence UMETA(DisplayName = "Life Luminescence"),

	/** Custom - For extension purposes */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * @enum EHarmoniaRecoveryEffectType
 * @brief How recovery effects are applied
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryEffectType : uint8
{
	/** Instant - Immediate health restoration */
	Instant UMETA(DisplayName = "Instant"),

	/** Over Time - Heal over Time (HoT) effect */
	OverTime UMETA(DisplayName = "Over Time"),

	/** Time Reversal - Restores to recent damage state */
	TimeReversal UMETA(DisplayName = "Time Reversal"),

	/** Area Deployable - Placeable area healing zone */
	AreaDeployable UMETA(DisplayName = "Area Deployable")
};

/**
 * @struct FHarmoniaResonanceShardVariant
 * @brief Resonance shard variant configuration with frequency-based effects
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceShardVariant
{
	GENERATED_BODY()

	/** Resonance frequency (color variant) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	EHarmoniaResonanceFrequency Frequency = EHarmoniaResonanceFrequency::Azure;

	/** Shard display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardName;

	/** Shard description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardDescription;

	/** Health recovery percentage (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	/** Additional gameplay effect applied on use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	/** Shard visual color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	FLinearColor ShardColor = FLinearColor::Blue;

	/** Visual effect played on use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	/** Sound effect played on use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Audio")
	TObjectPtr<USoundBase> UsageSound;

	FHarmoniaResonanceShardVariant()
	{
		ShardName = FText::FromString(TEXT("Azure Resonance Shard"));
		ShardDescription = FText::FromString(TEXT("A fragment imbued with azure resonance energy. Restores health and grants brief defense."));
	}
};

/**
 * @struct FHarmoniaRecoveryItemConfig
 * @brief Base recovery item configuration (designer-editable class defaults)
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemConfig
{
	GENERATED_BODY()

	/** Item type identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	/** Item display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemName;

	/** Item description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemDescription;

	/** Item icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UTexture2D> ItemIcon;

	/** Maximum number of charges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 MaxCharges = 5;

	/** Starting charge count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 InitialCharges = 5;

	/** Whether charges restore at checkpoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	bool bRechargeableAtCheckpoint = true;

	/** Usage duration in seconds (animation time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage", meta = (ClampMin = "0.0"))
	float UsageDuration = 1.5f;

	/** Whether movement cancels usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnMovement = true;

	/** Whether taking damage cancels usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnDamage = true;

	/** Recovery effect application type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	EHarmoniaRecoveryEffectType EffectType = EHarmoniaRecoveryEffectType::Instant;

	/** Fixed health recovery amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	float HealthRecoveryAmount = 0.0f;

	/** Health recovery as percentage of max health (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	/** Duration for heal-over-time effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0"))
	float RecoveryDuration = 3.0f;

	/** Additional gameplay effects applied (buffs, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

	/** Animation montage played during use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	/** Visual effect played on use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	/** Sound effect played on use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Audio")
	TObjectPtr<USoundBase> UsageSound;

	/** Sound effect played on usage failure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Audio")
	TObjectPtr<USoundBase> FailureSound;

	FHarmoniaRecoveryItemConfig()
	{
		ItemName = FText::FromString(TEXT("Recovery Item"));
		ItemDescription = FText::FromString(TEXT("A basic recovery item."));
	}
};

/**
 * @struct FHarmoniaTimeReversalSnapshot
 * @brief Time reversal snapshot data (for Frozen Time Snowflake)
 */
USTRUCT(BlueprintType)
struct FHarmoniaTimeReversalSnapshot
{
	GENERATED_BODY()

	/** Timestamp when snapshot was taken */
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Timestamp = 0.0f;

	/** Health value at snapshot time */
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Health = 0.0f;

	/** World location at snapshot time */
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FVector Location = FVector::ZeroVector;

	/** Rotation at snapshot time */
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * @struct FHarmoniaDeployableRecoveryConfig
 * @brief Deployable recovery zone configuration (for Life Luminescence)
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeployableRecoveryConfig
{
	GENERATED_BODY()

	/** Recovery zone radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float RecoveryRadius = 300.0f;

	/** Duration the zone persists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float Duration = 60.0f;

	/** Health restored per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	float HealthPerTick = 5.0f;

	/** Interval between healing ticks (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.1"))
	float TickInterval = 1.0f;

	/** Actor class spawned when deploying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	TSubclassOf<AActor> DeployableActorClass;

	/** Visual effect for the recovery zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Visual")
	TObjectPtr<UNiagaraSystem> AreaVFX;

	/** Ambient sound for the recovery zone (looping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Audio")
	TObjectPtr<USoundBase> AreaSound;

	FHarmoniaDeployableRecoveryConfig()
	{
	}
};

/**
 * @struct FHarmoniaRecoveryItemState
 * @brief Runtime state of a recovery item (saved/loaded)
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemState
{
	GENERATED_BODY()

	/** Item type identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	/** Current number of charges */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 CurrentCharges = 0;

	/** Maximum charges (can increase with upgrades) */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 MaxCharges = 5;

	/** Resonance shard frequency (for ResonanceShard type) */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	/** Timestamp of last usage */
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
 * @struct FHarmoniaRecoveryItemUseResult
 * @brief Result data from using a recovery item
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemUseResult
{
	GENERATED_BODY()

	/** Whether usage was successful */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	bool bSuccess = false;

	/** Amount of health restored */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	float HealthRecovered = 0.0f;

	/** Remaining charges after use */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 RemainingCharges = 0;

	/** Reason for failure if unsuccessful */
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	FText FailureReason;
};
