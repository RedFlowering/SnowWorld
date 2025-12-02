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
 * ?Œë³µ ?„ì´???€??
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryItemType : uint8
{
	// ê³µëª… ?Œí¸ - ì²´í¬?¬ì¸?¸ì—??ì¶©ì „?˜ëŠ” ê¸°ë³¸ ?Œë³µ ?„ì´??
	ResonanceShard UMETA(DisplayName = "Resonance Shard"),

	// ?¼ì–´ë¶™ì? ?œê°„???ˆì†¡??- ?œê°„ ??–‰ ?Œë³µ (?¬ê?)
	FrozenTimeSnowflake UMETA(DisplayName = "Frozen Time Snowflake"),

	// ?¨ì²œ ë³´ì˜¨ë³?- ì§€???Œë³µ + ë²„í”„
	ThermalSpringFlask UMETA(DisplayName = "Thermal Spring Flask"),

	// ?ëª…??ë£¨ë??¤ì„¼??- ?¤ì¹˜??ë²”ìœ„ ?Œë³µ
	LifeLuminescence UMETA(DisplayName = "Life Luminescence"),

	// ì»¤ìŠ¤?€ - ?•ì¥??
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Recovery Effect Type
 * ?Œë³µ ?¨ê³¼ ?€??
 */
UENUM(BlueprintType)
enum class EHarmoniaRecoveryEffectType : uint8
{
	// ì¦‰ì‹œ ?Œë³µ (Instant)
	Instant UMETA(DisplayName = "Instant"),

	// ì§€???Œë³µ (HoT - Heal over Time)
	OverTime UMETA(DisplayName = "Over Time"),

	// ?œê°„ ??–‰ (ìµœê·¼ ?¼í•´ ë³µêµ¬)
	TimeReversal UMETA(DisplayName = "Time Reversal"),

	// ?¤ì¹˜??ë²”ìœ„ ?Œë³µ
	AreaDeployable UMETA(DisplayName = "Area Deployable")
};

/**
 * Resonance Shard Variant
 * ê³µëª… ?Œí¸ ?‰ìƒë³??¨ê³¼
 */
USTRUCT(BlueprintType)
struct FHarmoniaResonanceShardVariant
{
	GENERATED_BODY()

	// ê³µëª… ì£¼íŒŒ??(?‰ìƒ)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	EHarmoniaResonanceFrequency Frequency = EHarmoniaResonanceFrequency::Azure;

	// ?Œí¸ ?´ë¦„
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardName;

	// ?Œí¸ ?¤ëª…
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	FText ShardDescription;

	// ì²´ë ¥ ?Œë³µ??(%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	// ì¶”ê? ?¨ê³¼ (GE)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard")
	TSubclassOf<UGameplayEffect> AdditionalEffect;

	// ?Œí¸ ?‰ìƒ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	FLinearColor ShardColor = FLinearColor::Blue;

	// ?¬ìš© ??VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance Shard|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	// ?¬ìš© ??SFX
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
 * ?Œë³µ ?„ì´???¤ì • (?¬ì‚¬??ê°€?¥í•œ ê¸°ë³¸ ?´ë˜??
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemConfig
{
	GENERATED_BODY()

	// ?„ì´???€??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	// ?„ì´???´ë¦„
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemName;

	// ?„ì´???¤ëª…
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item")
	FText ItemDescription;

	// ?„ì´???„ì´ì½?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UTexture2D> ItemIcon;

	// ìµœë? ë³´ìœ  ?Ÿìˆ˜
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 MaxCharges = 5;

	// ì´ˆê¸° ì¶©ì „ ?Ÿìˆ˜
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	int32 InitialCharges = 5;

	// ì²´í¬?¬ì¸?¸ì—??ì¶©ì „ ê°€???¬ë?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Charges")
	bool bRechargeableAtCheckpoint = true;

	// ?¬ìš© ?œê°„ (?œì „ ?œê°„, ì´?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage", meta = (ClampMin = "0.0"))
	float UsageDuration = 1.5f;

	// ?¬ìš© ì¤??´ë™ ??ì·¨ì†Œ ?¬ë?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnMovement = true;

	// ?¬ìš© ì¤??¼ê²© ??ì·¨ì†Œ ?¬ë?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Usage")
	bool bCancelOnDamage = true;

	// ?Œë³µ ?¨ê³¼ ?€??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	EHarmoniaRecoveryEffectType EffectType = EHarmoniaRecoveryEffectType::Instant;

	// ì²´ë ¥ ?Œë³µ??(ê³ ì •ê°?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	float HealthRecoveryAmount = 0.0f;

	// ì²´ë ¥ ?Œë³µ??(ìµœë? ì²´ë ¥ ?€ë¹?%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthRecoveryPercent = 0.5f;

	// ì§€???Œë³µ ?œê°„ (HoT??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect", meta = (ClampMin = "0.0"))
	float RecoveryDuration = 3.0f;

	// ?ìš©??Gameplay Effect (ë²„í”„, ì¶”ê? ?¨ê³¼ ??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Effect")
	TArray<TSubclassOf<UGameplayEffect>> GameplayEffects;

	// ?¬ìš© ??? ë‹ˆë©”ì´??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Animation")
	TObjectPtr<UAnimMontage> UsageAnimation;

	// ?¬ìš© ??VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Visual")
	TObjectPtr<UNiagaraSystem> UsageVFX;

	// ?¬ìš© ??SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery Item|Audio")
	TObjectPtr<USoundBase> UsageSound;

	// ?¬ìš© ?¤íŒ¨ ??SFX
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
 * ?œê°„ ??–‰ ?°ì´??(Frozen Time Snowflake??
 */
USTRUCT(BlueprintType)
struct FHarmoniaTimeReversalSnapshot
{
	GENERATED_BODY()

	// ?¤ëƒ…???œê°„
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Timestamp = 0.0f;

	// ì²´ë ¥
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	float Health = 0.0f;

	// ?„ì¹˜
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FVector Location = FVector::ZeroVector;

	// ?Œì „
	UPROPERTY(BlueprintReadWrite, Category = "Time Reversal")
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Deployable Recovery Area Configuration
 * ?¤ì¹˜???Œë³µ êµ¬ì—­ ?¤ì • (Life Luminescence??
 */
USTRUCT(BlueprintType)
struct FHarmoniaDeployableRecoveryConfig
{
	GENERATED_BODY()

	// ?Œë³µ êµ¬ì—­ ë°˜ê²½
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float RecoveryRadius = 300.0f;

	// ?Œë³µ êµ¬ì—­ ì§€???œê°„
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.0"))
	float Duration = 60.0f;

	// ?±ë‹¹ ?Œë³µ??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	float HealthPerTick = 5.0f;

	// ?Œë³µ ??ê°„ê²© (ì´?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery", meta = (ClampMin = "0.1"))
	float TickInterval = 1.0f;

	// ?¤ì¹˜ ???ì„±??Actor ?´ë˜??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery")
	TSubclassOf<AActor> DeployableActorClass;

	// ?Œë³µ êµ¬ì—­ VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Visual")
	TObjectPtr<UNiagaraSystem> AreaVFX;

	// ?Œë³µ êµ¬ì—­ SFX (ë£¨í”„)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deployable Recovery|Audio")
	TObjectPtr<USoundBase> AreaSound;

	FHarmoniaDeployableRecoveryConfig()
	{
	}
};

/**
 * Recovery Item Runtime State
 * ?Œë³µ ?„ì´???°í????íƒœ (?€??ë¡œë“œ??
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemState
{
	GENERATED_BODY()

	// ?„ì´???€??
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaRecoveryItemType ItemType = EHarmoniaRecoveryItemType::ResonanceShard;

	// ?„ì¬ ì¶©ì „ ?Ÿìˆ˜
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 CurrentCharges = 0;

	// ìµœë? ì¶©ì „ ?Ÿìˆ˜ (ê°•í™”ë¡?ì¦ê? ê°€??
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 MaxCharges = 5;

	// ê³µëª… ?Œí¸ ì£¼íŒŒ??(ResonanceShard ?„ìš©)
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	EHarmoniaResonanceFrequency ShardFrequency = EHarmoniaResonanceFrequency::Azure;

	// ë§ˆì?ë§??¬ìš© ?œê°„
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
 * ?Œë³µ ?„ì´???¬ìš© ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FHarmoniaRecoveryItemUseResult
{
	GENERATED_BODY()

	// ?¬ìš© ?±ê³µ ?¬ë?
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	bool bSuccess = false;

	// ?Œë³µ??ì²´ë ¥
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	float HealthRecovered = 0.0f;

	// ?¨ì? ì¶©ì „ ?Ÿìˆ˜
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	int32 RemainingCharges = 0;

	// ?¤íŒ¨ ?¬ìœ 
	UPROPERTY(BlueprintReadWrite, Category = "Recovery Item")
	FText FailureReason;
};
