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
 * ============================================================================
 * Required Tag Configuration (set in Blueprint or derived class):
 * ============================================================================
 *
 * AbilityTags:
 *   - Ability.Interaction (identifies this ability)
 *
 * AbilityTriggers:
 *   - TriggerTag: Event.Interaction.TryInteract
 *   - TriggerSource: GameplayEvent
 *
 * ActivationOwnedTags (tags applied while interacting):
 *   - State.Interacting
 *
 * ActivationBlockedTags (tags that prevent interaction):
 *   - State.Combat.Attacking
 *   - State.HitReaction
 *   - State.Dodging
 *
 * BlockAbilitiesWithTag (abilities to block while interacting):
 *   - State.Combat.Attacking
 *
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
