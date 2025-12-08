// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_StaminaRegen.generated.h"

/**
 * UHarmoniaGameplayAbility_StaminaRegen
 *
 * Passive Gameplay Ability for stamina regeneration.
 * Applies a periodic GameplayEffect that restores stamina over time.
 *
 * This ability is designed to be granted at spawn and remain active indefinitely.
 * The regeneration can be blocked by applying the Debuff.StaminaRecoveryBlocked tag.
 *
 * Usage:
 * - Add to AbilitySet with no InputTag (passive ability)
 * - Configure StaminaRegenEffectClass in Blueprint
 * - The GE should be Infinite duration with Period
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_StaminaRegen : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_StaminaRegen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** GameplayEffect class for stamina regeneration (should be Infinite with Period) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	TSubclassOf<UGameplayEffect> StaminaRegenEffectClass;

	/** Active effect handle for the stamina regen effect */
	FActiveGameplayEffectHandle ActiveRegenEffectHandle;
};
