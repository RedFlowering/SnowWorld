// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Interact.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "AbilitySystemComponent.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"

DEFINE_LOG_CATEGORY(LogHarmoniaInteraction);

UHarmoniaGameplayAbility_Interact::UHarmoniaGameplayAbility_Interact()
{
	// Default to InstancedPerActor so we can have state if needed
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Trigger on event - Commented out to avoid callstack issues. Configure in header or Blueprint instead.
	// InteractionEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.TryInteract"));

	// FAbilityTriggerData TriggerData;
	// TriggerData.TriggerTag = InteractionEventTag;
	// TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	// AbilityTriggers.Add(TriggerData);
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
	const FGameplayTag& EventTag = EventData->EventTag;

	// Commented out to avoid callstack issues. Configure tags in Blueprint or use alternative approach.
	// static const FGameplayTag PickupTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.Type.Pickup"), false);
	// static const FGameplayTag GatherTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.Type.Gather"), false);
	// static const FGameplayTag ChopTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.Type.Chop"), false);
	// static const FGameplayTag OpenTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.Type.Open"), false);

	// if (PickupTag.IsValid() && EventTag.MatchesTag(PickupTag))
	// {
	// 	return EHarmoniaInteractionType::Pickup;
	// }
	// else if (GatherTag.IsValid() && EventTag.MatchesTag(GatherTag))
	// {
	// 	return EHarmoniaInteractionType::Gather;
	// }
	// else if (ChopTag.IsValid() && EventTag.MatchesTag(ChopTag))
	// {
	// 	return EHarmoniaInteractionType::Chop;
	// }
	// else if (OpenTag.IsValid() && EventTag.MatchesTag(OpenTag))
	// {
	// 	return EHarmoniaInteractionType::Open;
	// }

	return EHarmoniaInteractionType::Custom;
}
