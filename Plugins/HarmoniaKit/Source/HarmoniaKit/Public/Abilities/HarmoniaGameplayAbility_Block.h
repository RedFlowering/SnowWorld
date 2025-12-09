// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "GameplayEffect.h"
#include "HarmoniaGameplayAbility_Block.generated.h"

class UHarmoniaMeleeCombatComponent;

/**
 * Block Gameplay Ability
 * Allows blocking incoming attacks with shield or weapon
 *
 * Features:
 * - Hold to block
 * - Reduces incoming damage
 * - Consumes stamina on block
 * - Guard break on insufficient stamina
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.3 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Block : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Block(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~End of UGameplayAbility interface

	/** Called when montage is interrupted or cancelled */
	UFUNCTION()
	void OnMontageInterrupted();

public:
	/** Called when successfully blocking an attack - applies stamina cost */
	UFUNCTION()
	void OnBlockHit(AActor* Attacker, float IncomingDamage);

protected:
	/** Block animation montage (with Start, Loop, End sections) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	TObjectPtr<UAnimMontage> BlockMontage = nullptr;

	/** Section name for block start */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	FName BlockStartSectionName = FName("Start");

	/** Section name for block loop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	FName BlockLoopSectionName = FName("Loop");

	/** Section name for block end */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	FName BlockEndSectionName = FName("End");

	// ============================================================================
	// Stamina Cost Configuration
	// ============================================================================

	/** Minimum stamina required to activate the block ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Cost")
	float MinStaminaToActivate = 10.0f;

	/** GE applied when successfully blocking an attack (Instant) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Cost")
	TSubclassOf<UGameplayEffect> BlockHitStaminaCostEffectClass;

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent = nullptr;

	/** Active handle for hold stamina cost effect */
	FActiveGameplayEffectHandle BlockHoldCostEffectHandle;

private:
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** Called when stamina runs out while blocking */
	void OnOutOfStamina(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* EffectSpec, float Magnitude, float OldValue, float NewValue);
};
