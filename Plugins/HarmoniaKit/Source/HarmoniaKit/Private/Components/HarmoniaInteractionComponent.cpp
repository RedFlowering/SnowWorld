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

    // 매니?� 캐싱 ?�득
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
		// ?�터?�션 컨텍?�트 ?�성
		FHarmoniaInteractionContext Context;
		Context.Interactor = GetOwner();
		Context.Interactable = Target;
		Context.InteractionType = EHarmoniaInteractionType::Custom; // ?��? ?�황??맞게

		// 매니?�?�게 ?�터?�션 ?�청
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			// ?�버?�서??바로 처리
			InteractionManager->TryInteract(Context);
		}
		else
		{
			// ?�라?�언?�에?�는 RPC ?�출
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
            // ?�공?? 거리?� ??검�?

            return true;
        }
    }

    return false;
}
