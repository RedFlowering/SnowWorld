// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayHitEffect.generated.h"

/**
 * Play Hit Effect Notify
 * Spawns VFX and plays SFX when melee attack hits
 *
 * Usage:
 * - Place at hit frame in attack animation
 * - Configure particle system and sound
 * - Automatically spawns at weapon socket or impact location
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Play Hit Effect"))
class HARMONIAKIT_API UAnimNotify_PlayHitEffect : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_PlayHitEffect();

	//~UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotify interface

protected:
	// ============================================================================
	// Visual Effects
	// ============================================================================

	/** Particle system to spawn on hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	TObjectPtr<UParticleSystem> HitParticleSystem;

	/** Niagara system to spawn on hit (takes priority over particle system) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	TObjectPtr<class UNiagaraSystem> HitNiagaraSystem;

	/** Scale of the effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	FVector EffectScale = FVector(1.0f);

	/** Socket to attach effect to (leave empty for world space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	FName AttachSocketName = NAME_None;

	/** Offset from socket/hit location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	FVector LocationOffset = FVector::ZeroVector;

	/** Rotation offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Whether to attach to socket or spawn in world */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|VFX")
	bool bAttachToSocket = false;

	// ============================================================================
	// Sound Effects
	// ============================================================================

	/** Sound to play on hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|SFX")
	TObjectPtr<USoundBase> HitSound;

	/** Volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|SFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VolumeMultiplier = 1.0f;

	/** Pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|SFX", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PitchMultiplier = 1.0f;

	/** Whether to attach sound to socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|SFX")
	bool bAttachSound = false;

	// ============================================================================
	// Advanced
	// ============================================================================

	/** Whether to use hit location from attack component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|Advanced")
	bool bUseHitLocation = false;

	/** Whether to use hit normal for rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Effect|Advanced")
	bool bAlignToHitNormal = false;

private:
	/** Get spawn location and rotation */
	void GetSpawnTransform(USkeletalMeshComponent* MeshComp, FVector& OutLocation, FRotator& OutRotation) const;

	/** Get last hit result from attack component */
	bool GetLastHitResult(AActor* Owner, FVector& OutLocation, FVector& OutNormal) const;
};
