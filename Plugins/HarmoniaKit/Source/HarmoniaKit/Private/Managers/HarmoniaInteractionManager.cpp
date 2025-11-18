// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInteractionManager.h"
#include "Interfaces/HarmoniaInteractableInterface.h"

void UHarmoniaInteractionManager::TryInteract(const FHarmoniaInteractionContext& Context)
{
    // 1. ��ȿ�� üũ
    if (!Context.Interactor || !Context.Interactable)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryInteract: Invalid Interactor or Interactable!"));
        FHarmoniaInteractionResult Result;
        Result.bSuccess = false;
        Result.Message = TEXT("��ȿ���� ���� ����Դϴ�.");
        OnInteractionCompleted.Broadcast(Context, Result);
        return;
    }

    // 2. ���� ��ȣ�ۿ� ó��
    HandleInteraction(Context);
}

void UHarmoniaInteractionManager::HandleInteraction(const FHarmoniaInteractionContext& Context)
{
    FHarmoniaInteractionResult Result;
    Result.bSuccess = false;
    Result.Message = TEXT("��ȣ�ۿ뿡 �����߽��ϴ�.");

    // �������̽��� �����Ǿ� �ִٸ� ȣ��
    if (Context.Interactable->GetClass()->ImplementsInterface(UHarmoniaInteractableInterface::StaticClass()))
    {
        IHarmoniaInteractableInterface::Execute_OnInteract(Context.Interactable, Context, Result);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Interactable does not implement HarmoniaInteractableInterface."));
        Result.bSuccess = false;
        Result.Message = TEXT("��ȣ�ۿ� �Ұ�: �������̽� �̱���.");
    }

    // ��� ��ε�ĳ��Ʈ
    OnInteractionCompleted.Broadcast(Context, Result);
}