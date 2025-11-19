// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInteractionManager.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UHarmoniaInteractionManager::TryInteract(const FHarmoniaInteractionContext& Context)
{
    // 1. Validation
    if (!Context.Interactor || !Context.Interactable)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryInteract: Invalid Interactor or Interactable!"));
        FHarmoniaInteractionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("Invalid interaction target.");
        OnInteractionCompleted.Broadcast(Context, Result);
        return;
    }

    // 2. Try to use GAS first
    if (SendInteractionEvent(Context))
    {
        return;
    }

    // 3. Fallback to direct interaction
    HandleInteraction(Context);
}

void UHarmoniaInteractionManager::HandleInteraction(const FHarmoniaInteractionContext& Context)
{
    FHarmoniaInteractionResult Result;
    Result.bSuccess = false;
    Result.Message = TEXT("Interaction failed.");

    // Check interface
    if (Context.Interactable->GetClass()->ImplementsInterface(UHarmoniaInteractableInterface::StaticClass()))
    {
        IHarmoniaInteractableInterface::Execute_OnInteract(Context.Interactable, Context, Result);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Interactable does not implement HarmoniaInteractableInterface."));
        Result.bSuccess = false;
        Result.Message = TEXT("Interaction failed: Interface missing.");
    }

    // Broadcast result
    OnInteractionCompleted.Broadcast(Context, Result);
}

bool UHarmoniaInteractionManager::SendInteractionEvent(const FHarmoniaInteractionContext& Context)
{
    if (!Context.Interactor)
    {
        return false;
    }

    // Check if Interactor has ASC
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Context.Interactor);
    if (!ASC)
    {
        return false;
    }

    // Define the interaction event tag
    FGameplayTag InteractionEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.TryInteract"));

    // Create payload
    FGameplayEventData Payload;
    Payload.Instigator = Context.Interactor;
    Payload.Target = Context.Interactable;
    Payload.OptionalObject = Context.Interactable;
    Payload.EventTag = InteractionEventTag;

    // Send event
    // Note: In UE 5.7, SendGameplayEventToActor returns void. 
    // If we have an ASC, we assume the event system is available for handling.
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Context.Interactor, InteractionEventTag, Payload);
    
    UE_LOG(LogTemp, Log, TEXT("Interaction event sent to GAS (Event: %s)"), *InteractionEventTag.ToString());
    return true;
}