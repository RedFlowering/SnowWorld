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

    // Îß§Îãà?Ä Ï∫êÏã± ?çÎìù
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
		// ?∏ÌÑ∞?ôÏÖò Ïª®ÌÖç?§Ìä∏ ?ùÏÑ±
		FHarmoniaInteractionContext Context;
		Context.Interactor = GetOwner();
		Context.Interactable = Target;
		Context.InteractionType = EHarmoniaInteractionType::Custom; // ?πÏ? ?ÅÌô©??ÎßûÍ≤å

		// Îß§Îãà?Ä?êÍ≤å ?∏ÌÑ∞?ôÏÖò ?îÏ≤≠
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			// ?úÎ≤Ñ?êÏÑú??Î∞îÎ°ú Ï≤òÎ¶¨
			InteractionManager->TryInteract(Context);
		}
		else
		{
			// ?¥Îùº?¥Ïñ∏?∏Ïóê?úÎäî RPC ?∏Ï∂ú
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
        // Í±∞Î¶¨ Ï≤¥ÌÅ¨
        float MaxDistance = TraceDistance;
        FVector Start = Context.Interactor->GetActorLocation();
        FVector End = Context.Interactable->GetActorLocation();
        float Dist = FVector::Dist(Start, End);

        if (Dist <= MaxDistance)
        {
            // ?±Í≥µ?? Í±∞Î¶¨?Ä ??Í≤ÄÏ¶?

            return true;
        }
    }

    return false;
}
