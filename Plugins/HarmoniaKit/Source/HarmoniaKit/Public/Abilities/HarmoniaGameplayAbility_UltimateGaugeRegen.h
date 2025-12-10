// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_UltimateGaugeRegen.generated.h"

/**
 * UHarmoniaGameplayAbility_UltimateGaugeRegen
 *
 * Passive Gameplay Ability for ultimate gauge regeneration.
 * Applies a periodic GameplayEffect that restores ultimate gauge over time.
 *
 * This ability is designed to be granted at spawn and remain active indefinitely.
 * Unlike stamina regen, there is no recovery block mechanism - the gauge fills
 * continuously until it reaches MaxUltimateGauge.
 *
 * Usage:
 * - Add to AbilitySet with no InputTag (passive ability)
 * - Configure UltimateGaugeRegenEffectClass in Blueprint
 * - The GE should be Infinite duration with Period
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_UltimateGaugeRegen : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_UltimateGaugeRegen(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** GameplayEffect class for ultimate gauge regeneration (should be Infinite with Period) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate Gauge")
	TSubclassOf<UGameplayEffect> UltimateGaugeRegenEffectClass;

	/** Active effect handle for the ultimate gauge regen effect */
	FActiveGameplayEffectHandle ActiveRegenEffectHandle;
};
