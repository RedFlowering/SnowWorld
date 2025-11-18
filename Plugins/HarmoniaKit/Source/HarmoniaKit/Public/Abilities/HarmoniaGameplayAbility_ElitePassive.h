// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_ElitePassive.generated.h"

/**
 * Elite Monster Passive Ability
 * - Increased stats (2x rewards already handled by loot system)
 * - Special visual effects
 * - Enhanced abilities
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_ElitePassive : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_ElitePassive();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Health multiplier for elite monsters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite")
	float HealthMultiplier = 2.0f;

	/** Damage multiplier for elite monsters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite")
	float DamageMultiplier = 1.5f;

	/** Movement speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite")
	float MovementSpeedMultiplier = 1.2f;

	/** Gameplay effect to apply for elite status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite")
	TSubclassOf<UGameplayEffect> EliteStatusEffect;

	/** Visual effect to spawn (aura, glow, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Elite")
	TSubclassOf<AActor> EliteVisualEffect;

	/** Handle for applied elite effect */
	FActiveGameplayEffectHandle EliteEffectHandle;

	/** Spawned visual effect */
	UPROPERTY()
	AActor* SpawnedVisualEffect = nullptr;

	/** Apply elite stat bonuses */
	void ApplyEliteBonuses();

	/** Spawn elite visual effects */
	void SpawnVisualEffects();
};
