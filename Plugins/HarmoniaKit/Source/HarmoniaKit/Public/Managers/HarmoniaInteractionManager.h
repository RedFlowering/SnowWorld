// Copyright 2025 Snow Game Studio.

#pragma once

#include "UObject/Object.h"
#include "Delegates/DelegateCombinations.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "../Definitions/HarmoniaInteractionSystemDefinitions.h"
#include "HarmoniaInteractionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionCompleted, const FHarmoniaInteractionContext&, Context, const FHarmoniaInteractionResult&, Result);

UCLASS()
class HARMONIAKIT_API UHarmoniaInteractionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Try to interact: Process interaction with given context information */
    UFUNCTION(BlueprintCallable, Category="Interaction")
    void TryInteract(const FHarmoniaInteractionContext& Context);

    /** Fired when interaction is completed (can be subscribed in Blueprint) */
    UPROPERTY(BlueprintAssignable, Category="Interaction")
    FOnInteractionCompleted OnInteractionCompleted;

protected:
    /** Handle interaction logic */
    void HandleInteraction(const FHarmoniaInteractionContext& Context);

    /** Send Gameplay Event to Interactor if they have ASC */
    bool SendInteractionEvent(const FHarmoniaInteractionContext& Context);
};