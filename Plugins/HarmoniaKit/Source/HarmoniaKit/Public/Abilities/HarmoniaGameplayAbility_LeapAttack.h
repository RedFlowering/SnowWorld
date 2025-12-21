// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_LeapAttack.generated.h"

class UHarmoniaCharacterMovementComponent;
class UMotionWarpingComponent;

/**
 * UHarmoniaGameplayAbility_LeapAttack
 *
 * Leap attack ability for both players and bosses.
 * Uses CustomMovementMode (MOVE_Leaping) for physics-based parabolic movement.
 * Motion Warping is controlled via AnimNotifyState in the montage.
 *
 * Player: Direction warp only (via LockOnComponent target)
 * Boss: Distance + Direction warp (via AIController blackboard target)
 *
 * Usage:
 * - Set LeapMontage with AnimNotifyState_MotionWarping on landing section
 * - Configure LeapAngle and movement parameters
 * - Warp target is set externally by LockOnComponent or AIController
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_LeapAttack : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_LeapAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	/** Get target location for leap (from WarpComponent or event data) */
	FVector GetLeapTargetLocation() const;

	/** Get movement component */
	UHarmoniaCharacterMovementComponent* GetHarmoniaMovementComponent() const;

	/** Get motion warping component */
	UMotionWarpingComponent* GetMotionWarpingComponent() const;

protected:
	// ============================================================================
	// Leap Settings
	// ============================================================================

	/** Leap angle (degrees from horizontal, 0=horizontal, 90=vertical) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leap", meta = (ClampMin = "15.0", ClampMax = "75.0"))
	float LeapAngle = 45.0f;

	/** Minimum leap distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leap")
	float MinLeapDistance = 200.0f;

	/** Maximum leap distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leap")
	float MaxLeapDistance = 1000.0f;

	/** Offset from target (land this far in front of target) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Leap")
	float TargetLocationOffset = 100.0f;

	// ============================================================================
	// Animation
	// ============================================================================

	/** Leap attack montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> LeapMontage;

	// ============================================================================
	// State
	// ============================================================================

	/** Cached target location for this leap */
	FVector CachedTargetLocation;
};
