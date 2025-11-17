// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInteractionComponent.h"
#include "Interface/HarmoniaInteractableInterface.h"
#include "Camera/CameraComponent.h"

UHarmoniaInteractionComponent::UHarmoniaInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHarmoniaInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // �Ŵ��� ���� ȹ��
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
		// ��ȣ�ۿ� ���ؽ�Ʈ ����
		FHarmoniaInteractionContext Context;
		Context.Interactor = GetOwner();
		Context.Interactable = Target;
		Context.InteractionType = EHarmoniaInteractionType::Custom; // Ȥ�� ��Ȳ�� ����

		// �Ŵ����� ��ȣ�ۿ� ��û
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			// ���������� ���� ó��
			InteractionManager->TryInteract(Context);
		}
		else
		{
			// Ŭ���̾�Ʈ�漭�� RPC ȣ��
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
        // �Ÿ� üũ
        float MaxDistance = TraceDistance;
        FVector Start = Context.Interactor->GetActorLocation();
        FVector End = Context.Interactable->GetActorLocation();
        float Dist = FVector::Dist(Start, End);

        if (Dist <= MaxDistance)
        {
            // ������, ��Ÿ�� �� ����

            return true;
        }
    }

    return false;
}
