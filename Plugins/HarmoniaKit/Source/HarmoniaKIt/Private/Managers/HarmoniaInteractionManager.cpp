// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInteractionManager.h"
#include "Interface/HarmoniaInteractableInterface.h"

void UHarmoniaInteractionManager::TryInteract(const FHarmoniaInteractionContext& Context)
{
    // 1. 유효성 체크
    if (!Context.Interactor || !Context.Interactable)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryInteract: Invalid Interactor or Interactable!"));
        FHarmoniaInteractionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("유효하지 않은 대상입니다.");
        OnInteractionCompleted.Broadcast(Context, Result);
        return;
    }

    // 2. 실제 상호작용 처리
    HandleInteraction(Context);
}

void UHarmoniaInteractionManager::HandleInteraction(const FHarmoniaInteractionContext& Context)
{
    FHarmoniaInteractionResult Result;
    Result.bSuccess = false;
    Result.Message = TEXT("상호작용에 실패했습니다.");

    // 인터페이스가 구현되어 있다면 호출
    if (Context.Interactable->GetClass()->ImplementsInterface(UHarmoniaInteractableInterface::StaticClass()))
    {
        IHarmoniaInteractableInterface::Execute_OnInteract(Context.Interactable, Context, Result);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Interactable does not implement HarmoniaInteractableInterface."));
        Result.bSuccess = false;
        Result.Message = TEXT("상호작용 불가: 인터페이스 미구현.");
    }

    // 결과 브로드캐스트
    OnInteractionCompleted.Broadcast(Context, Result);
}