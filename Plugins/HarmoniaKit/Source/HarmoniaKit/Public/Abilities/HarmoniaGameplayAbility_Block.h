// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
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
 * Tag Configuration (use inherited containers):
 * - ActivationOwnedTags: Tags applied while blocking (e.g., Character.State.Blocking)
 * - ActivationBlockedTags: Tags that prevent blocking (e.g., Character.State.Attacking)
 * - BlockAbilitiesWithTag: Abilities to block while this is active
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
	//~End of UGameplayAbility interface

protected:
	/** Block start animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	TObjectPtr<UAnimMontage> BlockStartMontage = nullptr;

	/** Block loop animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	TObjectPtr<UAnimMontage> BlockLoopMontage = nullptr;

	/** Block end animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block|Animation")
	TObjectPtr<UAnimMontage> BlockEndMontage = nullptr;

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent = nullptr;

private:
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;
};
