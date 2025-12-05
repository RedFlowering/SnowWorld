// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_SwitchTarget.generated.h"

class UHarmoniaLockOnComponent;

/**
 * UHarmoniaGameplayAbility_SwitchTarget
 *
 * Gameplay Ability for switching lock-on targets left/right.
 * Wraps UHarmoniaLockOnComponent::SwitchTargetLeft/Right() in a GA.
 *
 * Tag Checking via GAS (NO hardcoding):
 * - Set ActivationRequiredTags in Blueprint (e.g., State.LockOn.Active)
 * - GAS automatically blocks activation if required tags are missing
 * - No need for C++ tag checking code
 *
 * Usage:
 * - Add to AbilitySet with InputTag.SwitchTarget
 * - Set ActivationRequiredTags: State.LockOn.Active
 * - Uses Axis1D input (positive = right, negative = left)
 * - Ensure character has UHarmoniaLockOnComponent
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_SwitchTarget : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_SwitchTarget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Get the LockOn component from the owning actor */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	UHarmoniaLockOnComponent* GetLockOnComponent() const;
};
