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
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.5 for tag configuration
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
