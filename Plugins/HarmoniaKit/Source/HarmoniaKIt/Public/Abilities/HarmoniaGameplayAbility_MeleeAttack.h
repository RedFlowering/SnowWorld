// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaGameplayAbility_MeleeAttack.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseAttackComponent;
class UAnimMontage;

/**
 * Melee Attack Gameplay Ability
 * Soul-like melee combat ability with weapon-specific combos
 *
 * Features:
 * - Light/Heavy attacks
 * - Weapon-specific combo chains
 * - Stamina consumption
 * - Hit detection via HarmoniaSenseAttackComponent
 * - Animation-driven combat
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_MeleeAttack : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_MeleeAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	// ============================================================================
	// Attack Execution
	// ============================================================================

	/** Perform the melee attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Attack")
	virtual void PerformMeleeAttack();

	/** Get current attack montage based on combo state */
	UFUNCTION(BlueprintCallable, Category = "Melee Attack")
	UAnimMontage* GetCurrentAttackMontage() const;

	/** Get current attack step data */
	UFUNCTION(BlueprintCallable, Category = "Melee Attack")
	bool GetCurrentAttackStep(FHarmoniaComboAttackStep& OutStep) const;

	// ============================================================================
	// Combo Management
	// ============================================================================

	/** Queue next combo attack */
	UFUNCTION(BlueprintCallable, Category = "Melee Attack|Combo")
	void QueueNextCombo();

	/** Check if in combo window */
	UFUNCTION(BlueprintCallable, Category = "Melee Attack|Combo")
	bool IsInComboWindow() const;

	// ============================================================================
	// Animation Callbacks
	// ============================================================================

	/** Called when montage completes */
	UFUNCTION()
	void OnMontageCompleted();

	/** Called when montage is cancelled */
	UFUNCTION()
	void OnMontageCancelled();

	/** Called when montage is interrupted */
	UFUNCTION()
	void OnMontageInterrupted();

	/** Called when montage is blended out */
	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	// ============================================================================
	// Hit Detection
	// ============================================================================

	/** Called when attack hits target */
	UFUNCTION()
	void OnAttackHit(const FHarmoniaAttackHitResult& HitResult);

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/** Is this a heavy attack? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack")
	bool bIsHeavyAttack = false;

	/** Tags to apply while attacking */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack|Tags")
	FGameplayTagContainer AttackingTags;

	/** Tags to block while attacking */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack|Tags")
	FGameplayTagContainer BlockedTags;

	/** Damage gameplay effect to apply on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** Additional effects to apply on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack|Damage")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalHitEffects;

	/** Gameplay cue to trigger on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack|Visual")
	FGameplayTag HitGameplayCueTag;

	/** Camera shake on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack|Visual")
	TSubclassOf<class UCameraShakeBase> HitCameraShakeClass;

	// ============================================================================
	// State
	// ============================================================================

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent;

	/** Cached attack component */
	UPROPERTY()
	TObjectPtr<UHarmoniaSenseAttackComponent> AttackComponent;

	/** Current combo sequence */
	UPROPERTY()
	FHarmoniaComboAttackSequence CurrentComboSequence;

private:
	/** Get melee combat component from owner */
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** Get attack component from owner */
	UHarmoniaSenseAttackComponent* GetAttackComponent() const;

	/** Apply damage to hit actor */
	void ApplyDamageToTarget(AActor* TargetActor, const FHarmoniaAttackHitResult& HitResult);

	/** Trigger visual effects for hit */
	void TriggerHitEffects(const FHarmoniaAttackHitResult& HitResult);
};
