// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"
#include "HarmoniaGameplayAbility_Interact.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaInteraction, Log, All);

/**
 * Base Gameplay Ability for Harmonia Interactions
 * Triggered by Event.Interaction.TryInteract
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.5.2 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Interact : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Actually activate ability, either instantly or asynchronously */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Tag to listen for */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FGameplayTag InteractionEventTag;

	/** Derive interaction type from gameplay event tag */
	EHarmoniaInteractionType DeriveInteractionType(const FGameplayEventData* EventData) const;
};
