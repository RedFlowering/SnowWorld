// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaGameplayAbility_MeleeAttack.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseComponent;
class UAnimMontage;

/**
 * Melee Attack Gameplay Ability
 * Soul-like melee combat ability with weapon-specific combos
 *
 * Features:
 * - Light/Heavy attacks
 * - Weapon-specific combo chains
 * - Stamina consumption
 * - Hit detection via HarmoniaSenseComponent
 * - Animation-driven combat
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.2 for tag configuration
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
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
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

	/** Start waiting for combo input during attack */
	void StartWaitingForComboInput();

	/** Called when combo input is detected */
	UFUNCTION()
	void OnComboInputPressed(float TimeWaited);

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
	void OnMontageBlendOut();

	// ============================================================================
	// Hit Detection
	// ============================================================================

	/** Called when attack hits target */
	UFUNCTION()
	void OnAttackHit(const FHarmoniaAttackHitResult& HitResult);

	// ============================================================================
	// Charge Attack
	// ============================================================================

	/** Start charging (for Charged AttackType) */
	void StartCharging();

	/** Called each tick while charging */
	void OnChargeTick();

	/** Release charge and perform attack based on charge level */
	void ReleaseChargeAttack();

	/** Get current charge level based on charge time */
	int32 GetCurrentChargeLevel() const;

	/** Get charge config from current combo sequence */
	const FHarmoniaChargeConfig& GetChargeConfig() const;

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/** Attack type (Light, Heavy, Ultimate, Charged) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack")
	EHarmoniaAttackType AttackType = EHarmoniaAttackType::Light;

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
	TObjectPtr<UHarmoniaSenseComponent> AttackComponent;

	/** Current combo sequence */
	UPROPERTY()
	FHarmoniaComboAttackSequence CurrentComboSequence;

	// ============================================================================
	// Charge Attack State
	// ============================================================================

	/** Whether currently charging */
	bool bIsCharging = false;

	/** Time spent charging */
	float CurrentChargeTime = 0.0f;

	/** Cached charge level */
	int32 CachedChargeLevel = 0;

	/** Timer handle for charge tick */
	FTimerHandle ChargeTickTimerHandle;

	// ============================================================================
	// Ultimate Attack State
	// ============================================================================

	/** Last time ultimate attack was used (for cooldown check) */
	float LastUltimateUseTime = -9999.0f;

private:
	/** Get melee combat component from owner */
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** Get attack component from owner */
	UHarmoniaSenseComponent* GetAttackComponent() const;

	/** Apply damage to hit actor */
	void ApplyDamageToTarget(AActor* TargetActor, const FHarmoniaAttackHitResult& HitResult);

	/** Trigger visual effects for hit */
	void TriggerHitEffects(const FHarmoniaAttackHitResult& HitResult);

	/** Create target data handle from actor */
	FGameplayAbilityTargetDataHandle MakeTargetData(AActor* TargetActor) const;
};
