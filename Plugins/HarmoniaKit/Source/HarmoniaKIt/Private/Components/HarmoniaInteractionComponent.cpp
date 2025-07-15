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

    // 매니저 참조 획득
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
    if (InteractionManager)
    {
        // 트레이스 통해 대상 획득
        AActor* Target = GetTargetActor();
        if (Target)
        {
            // 상호작용 컨텍스트 구성
            FHarmoniaInteractionContext Context;
            Context.Interactor = GetOwner();
            Context.Interactable = Target;
            Context.InteractionType = EHarmoniaInteractionType::Custom; // 혹은 상황별 지정

            // 매니저에 상호작용 요청
            if (GetOwner() && GetOwner()->HasAuthority())
            {
                // 서버에서만 직접 처리
                InteractionManager->TryInteract(Context); 
            }
            else
            {
                // 클라이언트→서버 RPC 호출
                Server_TryInteract(Context); 
            }
        }
    }
}

void UHarmoniaInteractionComponent::Server_TryInteract_Implementation(const FHarmoniaInteractionContext& Context)
{
    if (InteractionManager)
    {
        InteractionManager->TryInteract(Context);

        // 처리 결과 알림
        // Client_OnInteractionResult(Result);
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
            // 소유권, 쿨타임 등 검증

            return true;
        }
    }

    return false;
}

void UHarmoniaInteractionComponent::Client_OnInteractionResult_Implementation(const FHarmoniaInteractionResult& Result)
{
    if (GEngine)
    {
        FString Msg = Result.Message.IsEmpty() ? (Result.bSuccess ? TEXT("상호작용 성공!") : TEXT("상호작용 실패!")) : Result.Message;
        FColor Color = Result.bSuccess ? FColor::Green : FColor::Red;
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, Color, Msg);
    }

    // 나중에 UMG 위젯에 결과 표시
    // if (ResultWidget) { ResultWidget->ShowInteractionResult(Result); }
}
