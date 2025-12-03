// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInteractionComponent.h"
#include "Core/HarmoniaCoreBFL.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "Camera/CameraComponent.h"

UHarmoniaInteractionComponent::UHarmoniaInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHarmoniaInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache manager reference
    InteractionManager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaInteractionManager>(this);
}

void UHarmoniaInteractionComponent::SetTargetActor(AActor* Target)
{
    TargetActor = Target;
}

AActor* UHarmoniaInteractionComponent::GetTargetActor()
{
    return TargetActor;
}

void UHarmoniaInteractionComponent::Interact()
{
    AActor* Target = GetTargetActor();

	if (InteractionManager && Target)
	{
		// Create interaction context
		FHarmoniaInteractionContext Context;
		Context.Interactor = GetOwner();
		Context.Interactable = Target;
		Context.InteractionType = EHarmoniaInteractionType::Custom; // Set based on situation

		// Request interaction from manager
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			// Process directly on server
			InteractionManager->TryInteract(Context);
		}
		else
		{
			// Call RPC on client
			Server_TryInteract(Context);
		}
	}
}

void UHarmoniaInteractionComponent::Server_TryInteract_Implementation(const FHarmoniaInteractionContext& Context)
{
    if (InteractionManager)
    {
        InteractionManager->TryInteract(Context);
    }
}

bool UHarmoniaInteractionComponent::Server_TryInteract_Validate(const FHarmoniaInteractionContext& Context)
{
    if (Context.Interactor && Context.Interactable)
    {
        // 거리 체크
        float MaxDistance = TraceDistance;
        FVector Start = Context.Interactor->GetActorLocation();
        FVector End = Context.Interactable->GetActorLocation();
        float Dist = FVector::Dist(Start, End);

        if (Dist <= MaxDistance)
        {
            // Success - perform distance and other checks

            return true;
        }
    }

    return false;
}
