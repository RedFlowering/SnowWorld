// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInteractionComponent.h"
#include "Interfaces/HarmoniaInteractableInterface.h"
#include "Camera/CameraComponent.h"

UHarmoniaInteractionComponent::UHarmoniaInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHarmoniaInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 매니저 캐싱 획득
    if (GetWorld() && GetWorld()->GetGameInstance())
    {
        InteractionManager = GetWorld()->GetGameInstance()->GetSubsystem<UHarmoniaInteractionManager>();
    }
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
		// 인터랙션 컨텍스트 생성
		FHarmoniaInteractionContext Context;
		Context.Interactor = GetOwner();
		Context.Interactable = Target;
		Context.InteractionType = EHarmoniaInteractionType::Custom; // 혹은 상황에 맞게

		// 매니저에게 인터랙션 요청
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			// 서버에서는 바로 처리
			InteractionManager->TryInteract(Context);
		}
		else
		{
			// 클라이언트에서는 RPC 호출
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
            // 성공적, 거리와 널 검증

            return true;
        }
    }

    return false;
}
