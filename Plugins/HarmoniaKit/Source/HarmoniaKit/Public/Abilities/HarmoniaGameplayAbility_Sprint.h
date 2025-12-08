// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Sprint.generated.h"

class AAlsCharacter;

/**
 * UHarmoniaGameplayAbility_Sprint
 *
 * Gameplay Ability for ALS Sprint integration.
 * Wraps AAlsCharacter::SetDesiredGait() in a GA for GAS-based input handling.
 *
 * Behavior:
 * - Hold-based: Sprint while input is held, stop when released
 * - Sets DesiredGait to Sprinting on activate, Running on end
 * - Uses ActivationOwnedTags for state management (e.g., State.Movement.Sprinting)
 *
 * Flow:
 * 1. Input pressed: ActivateAbility() -> SetDesiredGait(Sprinting) -> ActivationOwnedTags applied
 * 2. Input released: InputReleased() -> EndAbility() -> SetDesiredGait(Running) -> Tags removed
 *
 * Usage:
 * - Add to AbilitySet with InputTag.Sprint
 * - Bind to Shift key with Hold trigger in IMC
 * - Set ActivationOwnedTags: State.Movement.Sprinting (optional)
 * - Ensure character inherits from AAlsCharacter
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Sprint : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Called when input is released - ends sprint */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	/** Get the ALS character from the owning actor */
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	AAlsCharacter* GetAlsCharacter() const;

	/** Handle for the active sprint cost effect */
	FActiveGameplayEffectHandle SprintCostEffectHandle;

	/** Callback when stamina changes to check if we ran out */
	void OnOutOfStamina(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* EffectSpec, float Magnitude, float OldValue, float NewValue);

	/** Gait tag to restore when sprint ends (default: Running) */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	FGameplayTag RestoreGaitTag;

	/** Minimum stamina required to start sprinting (prevents immediate reactivation) */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint|Stamina")
	float MinStaminaToActivate = 10.0f;
};
