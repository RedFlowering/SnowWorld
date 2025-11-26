// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaGameplayAbility_Riposte.generated.h"

class UHarmoniaMeleeCombatComponent;
class UHarmoniaSenseAttackComponent;
class UAnimMontage;

/**
 * Riposte Gameplay Ability
 * Counter attack after successful parry
 *
 * Features:
 * - Can only be used during riposte window (after successful parry)
 * - Deals massive damage (typically 3x normal)
 * - Guaranteed critical hit
 * - Targets are stunned during the animation
 * - Consumes stamina
 *
 * ============================================================================
 * Required Tag Configuration (set in Blueprint or derived class):
 * ============================================================================
 *
 * AbilityTags:
 *   - Ability.Combat.Riposte (identifies this ability)
 *
 * ActivationOwnedTags (tags applied during riposte):
 *   - State.Combat.Riposting
 *   - State.Invincible (during animation)
 *
 * ActivationRequiredTags (tags required to activate):
 *   - State.Combat.RiposteWindow (applied after successful parry)
 *
 * ActivationBlockedTags (tags that prevent riposte):
 *   - State.HitReaction
 *   - State.Dodging
 *
 * BlockAbilitiesWithTag (abilities to block during riposte):
 *   - State.Combat.Attacking
 *   - State.Blocking
 *   - State.Dodging
 *
 * CancelAbilitiesWithTag (abilities to cancel on riposte):
 *   - State.Blocking
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Riposte : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Riposte(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

protected:
	/** Riposte configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Riposte")
	FHarmoniaRiposteConfig RiposteConfig;

	/** Damage gameplay effect to apply on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Riposte|Damage")
	TSubclassOf<UGameplayEffect> RiposteDamageEffectClass;

	/** Stun effect to apply to target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Riposte|Effects")
	TSubclassOf<UGameplayEffect> TargetStunEffectClass;

	/** Gameplay cue to trigger on riposte hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Riposte|Visual")
	FGameplayTag RiposteHitGameplayCueTag;

	/** Camera shake on riposte hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Riposte|Visual")
	TSubclassOf<class UCameraShakeBase> RiposteHitCameraShakeClass;

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent;

	/** Cached attack component */
	UPROPERTY()
	TObjectPtr<UHarmoniaSenseAttackComponent> AttackComponent;

	/** Parried target (set by parry ability) */
	UPROPERTY()
	TWeakObjectPtr<AActor> ParriedTarget;

private:
	/** Get melee combat component */
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** Get attack component */
	UHarmoniaSenseAttackComponent* GetAttackComponent() const;

	/** Called when riposte attack hits */
	UFUNCTION()
	void OnRiposteHit(const FHarmoniaAttackHitResult& HitResult);

	/** Called when montage completes */
	UFUNCTION()
	void OnMontageCompleted();

	/** Apply riposte damage to target */
	void ApplyRiposteDamage(AActor* Target);
};
