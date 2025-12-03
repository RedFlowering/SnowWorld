// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HarmoniaAnimationEffectDefinitions.generated.h"

/**
 * Animation Effect Data - DataTable row for animation effects
 * Stores VFX, SFX, and parameters for animation-triggered effects
 *
 * Usage:
 * - Create DataTable with this struct
 * - Set EffectTag as row name or in the struct
 * - Use tag in AnimNotify to spawn effects
 *
 * Example Tags:
 * - Effect.Hit.Slash.Light
 * - Effect.Hit.Slash.Heavy
 * - Effect.Hit.Blunt.Light
 * - Effect.Trail.Sword.Normal
 * - Effect.Trail.Axe.Heavy
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAnimationEffectData : public FTableRowBase
{
	GENERATED_BODY()

	// ============================================================================
	// Identification
	// ============================================================================

	/** Effect identifier tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FGameplayTag EffectTag;

	/** Display name for editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FText DisplayName;

	/** Description of this effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FText Description;

	// ============================================================================
	// Visual Effects
	// ============================================================================

	/** Niagara system to spawn (preferred) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<class UNiagaraSystem> NiagaraSystem;

	/** Particle system to spawn (fallback) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ParticleSystem;

	/** Scale of the effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector EffectScale = FVector(1.0f);

	/** Effect color tint (if supported by the effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FLinearColor EffectColor = FLinearColor::White;

	/** Socket to attach effect to (leave empty for world space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FName AttachSocketName = NAME_None;

	/** Location offset from socket/hit location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector LocationOffset = FVector::ZeroVector;

	/** Rotation offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Whether to attach to socket or spawn in world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bAttachToSocket = false;

	/** Whether to use hit location from attack component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bUseHitLocation = false;

	/** Whether to align to hit normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bAlignToHitNormal = false;

	// ============================================================================
	// Sound Effects
	// ============================================================================

	/** Sound to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> Sound;

	/** Volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VolumeMultiplier = 1.0f;

	/** Pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PitchMultiplier = 1.0f;

	/** Whether to attach sound to socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	bool bAttachSound = false;

	// ============================================================================
	// Trail-Specific Settings
	// ============================================================================

	/** Socket for trail start point (for weapon trails) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	FName TrailStartSocketName = FName("weapon_trail_start");

	/** Socket for trail end point (for weapon trails) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	FName TrailEndSocketName = FName("weapon_trail_end");

	/** Trail width multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float TrailWidthMultiplier = 1.0f;
};

/**
 * Animation Effect Category - For organizing effects in editor
 */
UENUM(BlueprintType)
enum class EHarmoniaEffectCategory : uint8
{
	Hit UMETA(DisplayName = "Hit Effects"),
	Trail UMETA(DisplayName = "Weapon Trails"),
	Impact UMETA(DisplayName = "Impact Effects"),
	Projectile UMETA(DisplayName = "Projectile Effects"),
	Buff UMETA(DisplayName = "Buff/Debuff Effects"),
	Environment UMETA(DisplayName = "Environment Effects"),
	Custom UMETA(DisplayName = "Custom Effects")
};
