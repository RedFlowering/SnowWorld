// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_LockOn.generated.h"

class UHarmoniaLockOnComponent;

/**
 * UHarmoniaGameplayAbility_LockOn
 *
 * Gameplay Ability for lock-on targeting (toggle-based, duration ability).
 * 
 * Tag Management via GAS (NO hardcoding in component):
 * - Set ActivationOwnedTags in Blueprint (e.g., State.LockOn.Active)
 * - Tag is automatically added when ability activates
 * - Tag is automatically removed when ability ends
 * - Other GAs can check this tag via their Activation Required/Blocked Tags
 *
 * Toggle Flow:
 * 1. First input: Activates ability -> EnableLockOn() -> ActivationOwnedTags applied
 * 2. Second input: InputPressed() called -> EndAbility() -> DisableLockOn() -> Tags removed
 *
 * Usage:
 * - Add to AbilitySet with InputTag.LockOn
 * - Set ActivationOwnedTags: State.LockOn.Active
 * - Ensure character has UHarmoniaLockOnComponent
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_LockOn : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_LockOn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Called when input is pressed while ability is active - toggles off */
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	/** Get the LockOn component from the owning actor */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	UHarmoniaLockOnComponent* GetLockOnComponent() const;

	/** Handle when target is lost */
	UFUNCTION()
	void OnTargetLost(bool bIsLockedOn);
};
