// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Interact.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "AbilitySystemComponent.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"

DEFINE_LOG_CATEGORY(LogHarmoniaInteraction);

UHarmoniaGameplayAbility_Interact::UHarmoniaGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default to InstancedPerActor so we can have state if needed
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// ============================================================================
	// Tag Configuration: Do NOT hardcode tags in constructor!
	// Configure these in Blueprint or derived class CDO:
	// ============================================================================
	// ActivationOwnedTags:
	//   - State.Interacting
	// ActivationBlockedTags:
	//   - State.Combat.Attacking
	//   - State.HitReaction
	//   - State.Dodging
	// AbilityTriggers:
	//   - TriggerTag: Event.Interaction.TryInteract
	// ============================================================================
}

void UHarmoniaGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Validate event data
	if (!TriggerEventData)
	{
		UE_LOG(LogHarmoniaInteraction, Warning, TEXT("GA_Interact failed: No event data provided"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!TriggerEventData->Target)
	{
		UE_LOG(LogHarmoniaInteraction, Warning, TEXT("GA_Interact failed: No target in event data"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Safely cast to Actor (Target is const UObject*)
	AActor* InteractableActor = const_cast<AActor*>(Cast<const AActor>(TriggerEventData->Target));
	if (!InteractableActor)
	{
		UE_LOG(LogHarmoniaInteraction, Error,
			TEXT("GA_Interact failed: Target is not an Actor (Type: %s)"),
			*TriggerEventData->Target->GetClass()->GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Check if actor implements the interaction interface
	if (!InteractableActor->Implements<UHarmoniaInteractableInterface>())
	{
		UE_LOG(LogHarmoniaInteraction, Warning,
			TEXT("GA_Interact failed: Actor '%s' does not implement HarmoniaInteractableInterface"),
			*InteractableActor->GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Build interaction context
	FHarmoniaInteractionContext Context;
	Context.Interactor = ActorInfo->AvatarActor.Get();
	Context.Interactable = InteractableActor;
	Context.InteractionType = DeriveInteractionType(TriggerEventData);

	// Execute interaction
	FHarmoniaInteractionResult Result;
	IHarmoniaInteractableInterface::Execute_OnInteract(InteractableActor, Context, Result);

	if (Result.bSuccess)
	{
		UE_LOG(LogHarmoniaInteraction, Log,
			TEXT("Interaction successful: %s -> %s (Type: %d)"),
			*Context.Interactor->GetName(),
			*Context.Interactable->GetName(),
			static_cast<int32>(Context.InteractionType));
		// Could play montage here
	}
	else
	{
		UE_LOG(LogHarmoniaInteraction, Warning,
			TEXT("Interaction failed: %s -> %s (Reason: %s)"),
			*Context.Interactor->GetName(),
			*Context.Interactable->GetName(),
			*Result.Message);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

EHarmoniaInteractionType UHarmoniaGameplayAbility_Interact::DeriveInteractionType(const FGameplayEventData* EventData) const
{
	if (!EventData)
	{
		return EHarmoniaInteractionType::Custom;
	}

	// Parse interaction type from event tag
	// Configure specific interaction type tags in Blueprint or derived classes
	// Example tags: Event.Interaction.Type.Pickup, Event.Interaction.Type.Gather, etc.

	return EHarmoniaInteractionType::Custom;
}
