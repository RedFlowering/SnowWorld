// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Interact.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "AbilitySystemComponent.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"

UHarmoniaGameplayAbility_Interact::UHarmoniaGameplayAbility_Interact()
{
	// Default to InstancedPerActor so we can have state if needed
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// Trigger on event
	InteractionEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.TryInteract"));
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = InteractionEventTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UHarmoniaGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get Interactable from event data
	AActor* InteractableActor = const_cast<AActor*>(TriggerEventData ? Cast<AActor>(TriggerEventData->Target) : nullptr);
	
	if (!InteractableActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GA_Interact: No target found in event data."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Execute interaction
	if (InteractableActor->GetClass()->ImplementsInterface(UHarmoniaInteractableInterface::StaticClass()))
	{
		FHarmoniaInteractionContext Context;
		Context.Interactor = ActorInfo->AvatarActor.Get();
		Context.Interactable = InteractableActor;
		Context.InteractionType = EHarmoniaInteractionType::Custom; // Or derive from tag

		FHarmoniaInteractionResult Result;
		IHarmoniaInteractableInterface::Execute_OnInteract(InteractableActor, Context, Result);

		if (Result.bSuccess)
		{
			// Could play montage here
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
