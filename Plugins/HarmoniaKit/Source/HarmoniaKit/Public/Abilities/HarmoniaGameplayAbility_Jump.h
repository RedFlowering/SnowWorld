// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Jump.generated.h"

class ACharacter;

/**
 * UHarmoniaGameplayAbility_Jump
 *
 * Simple Jump ability with stamina integration.
 * Uses ALS/Character's built-in jump system.
 *
 * Behavior:
 * - Instant activation: Triggers jump on activate
 * - Stamina check: Requires minimum stamina to jump
 * - Optional stamina cost: Can apply GE for stamina consumption
 *
 * Flow:
 * 1. Input pressed: ActivateAbility() -> Check stamina -> Character->Jump()
 * 2. Ability ends immediately after jump is triggered
 *
 * Usage:
 * - Add to AbilitySet with InputTag.Jump
 * - Bind to Space key with Pressed trigger in IMC
 * - Set CostGameplayEffectClass for stamina consumption (optional)
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Jump : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** Minimum stamina required to jump */
	UPROPERTY(EditDefaultsOnly, Category = "Jump|Stamina")
	float MinStaminaToJump = 10.0f;

	/** Stamina cost for jumping (applied via CostGameplayEffectClass) */
	UPROPERTY(EditDefaultsOnly, Category = "Jump|Stamina")
	float StaminaCost = 10.0f;
};
