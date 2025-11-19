// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInteractionManager.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/HarmoniaGameplayAbility_Interact.h"

void UHarmoniaInteractionManager::TryInteract(const FHarmoniaInteractionContext& Context)
{
    // 1. Validation
    if (!Context.Interactor || !Context.Interactable)
    {
        UE_LOG(LogHarmoniaInteraction, Warning, TEXT("TryInteract: Invalid Interactor or Interactable!"));
        FHarmoniaInteractionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("Invalid interaction target.");
        OnInteractionCompleted.Broadcast(Context, Result);
        return;
    }

    // 2. Try to use GAS first
    if (SendInteractionEvent(Context))
    {
        UE_LOG(LogHarmoniaInteraction, Verbose, TEXT("Interaction delegated to GAS: %s -> %s"),
            *Context.Interactor->GetName(), *Context.Interactable->GetName());
        return;
    }

    // 3. Fallback to direct interaction
    UE_LOG(LogHarmoniaInteraction, Verbose, TEXT("Using direct interaction (no GAS): %s -> %s"),
        *Context.Interactor->GetName(), *Context.Interactable->GetName());
    HandleInteraction(Context);
}

void UHarmoniaInteractionManager::HandleInteraction(const FHarmoniaInteractionContext& Context)
{
    FHarmoniaInteractionResult Result;
    Result.bSuccess = false;
    Result.Message = TEXT("Interaction failed.");

    // Check interface using recommended method
    if (Context.Interactable->Implements<UHarmoniaInteractableInterface>())
    {
        IHarmoniaInteractableInterface::Execute_OnInteract(Context.Interactable, Context, Result);
    }
    else
    {
        UE_LOG(LogHarmoniaInteraction, Warning,
            TEXT("Interactable '%s' does not implement HarmoniaInteractableInterface."),
            *Context.Interactable->GetName());
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
    static const FGameplayTag InteractionEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.TryInteract"));

    // Validate that ASC has an ability that can handle this event
    bool bHasHandler = false;
    const TArray<FGameplayAbilitySpec>& Specs = ASC->GetActivatableAbilities();
    for (const FGameplayAbilitySpec& Spec : Specs)
    {
        if (Spec.Ability)
        {
            for (const FAbilityTriggerData& Trigger : Spec.Ability->AbilityTriggers)
            {
                if (Trigger.TriggerTag == InteractionEventTag)
                {
                    bHasHandler = true;
                    break;
                }
            }
            if (bHasHandler)
            {
                break;
            }
        }
    }

    if (!bHasHandler)
    {
        UE_LOG(LogHarmoniaInteraction, Verbose,
            TEXT("No ability registered for interaction event '%s' on actor '%s'. Falling back to direct interaction."),
            *InteractionEventTag.ToString(),
            *Context.Interactor->GetName());
        return false;
    }

    // Create payload
    FGameplayEventData Payload;
    Payload.Instigator = Context.Interactor;
    Payload.Target = Context.Interactable;
    Payload.OptionalObject = Context.Interactable;
    Payload.EventTag = InteractionEventTag;

    // Send event
    // Note: In UE 5.7, SendGameplayEventToActor returns void.
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Context.Interactor, InteractionEventTag, Payload);

    UE_LOG(LogHarmoniaInteraction, Log,
        TEXT("Interaction event sent to GAS: %s -> %s (Event: %s)"),
        *Context.Interactor->GetName(),
        *Context.Interactable->GetName(),
        *InteractionEventTag.ToString());
    return true;
}
