// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_PlayWeaponTrail.generated.h"

/**
 * Play Weapon Trail Notify State
 * Spawns and manages weapon trail VFX during attack
 *
 * Usage:
 * - Place over attack swing duration
 * - Trail appears during state, disappears when state ends
 * - Supports both Niagara and Cascade particles
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Play Weapon Trail"))
class HARMONIAKIT_API UAnimNotify_PlayWeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotify_PlayWeaponTrail();

	//~UAnimNotifyState interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotifyState interface

protected:
	// ============================================================================
	// Trail Settings
	// ============================================================================

	/** Particle system for weapon trail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|VFX")
	TObjectPtr<UParticleSystem> TrailParticleSystem;

	/** Niagara system for weapon trail (takes priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|VFX")
	TObjectPtr<class UNiagaraSystem> TrailNiagaraSystem;

	/** Socket to attach trail start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|VFX")
	FName StartSocketName = FName("weapon_trail_start");

	/** Socket to attach trail end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|VFX")
	FName EndSocketName = FName("weapon_trail_end");

	/** Trail width multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|VFX", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float WidthMultiplier = 1.0f;

	/** Trail color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|VFX")
	FLinearColor TrailColor = FLinearColor::White;

	// ============================================================================
	// Sound Settings
	// ============================================================================

	/** Swoosh sound to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|SFX")
	TObjectPtr<USoundBase> SwooshSound;

	/** Volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|SFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VolumeMultiplier = 0.5f;

	/** Pitch multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trail|SFX", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PitchMultiplier = 1.0f;

private:
	/** Spawned trail component */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> SpawnedTrailComponent;
};
