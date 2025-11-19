// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HarmoniaGameplayAbility_Interact.generated.h"

/**
 * Base Gameplay Ability for Harmonia Interactions
 * Triggered by Event.Interaction.TryInteract
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Interact : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Interact();

	/** Actually activate ability, either instantly or asynchronously */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Tag to listen for */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FGameplayTag InteractionEventTag;
};
