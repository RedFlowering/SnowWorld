// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Parry.generated.h"

class UHarmoniaMeleeCombatComponent;

/**
 * Parry Gameplay Ability
 * Deflect incoming attacks with precise timing
 *
 * Features:
 * - Short parry window
 * - On success: stuns attacker
 * - On failure: treated as normal hit
 * - Consumes less stamina than blocking
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.4 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Parry : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Parry(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

protected:
	/** Parry animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	TObjectPtr<UAnimMontage> ParryMontage;

	/** Parry success animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry|Animation")
	TObjectPtr<UAnimMontage> ParrySuccessMontage;

	/** Parry window duration (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry")
	float ParryWindowDuration = 0.2f;

	/** Stamina cost for parry attempt */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Parry")
	float ParryStaminaCost = 10.0f;

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent;

private:
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** End parry window */
	void EndParryWindow();

	FTimerHandle ParryWindowTimerHandle;
};
