// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaInstancedObjectManagerBase.h"
#include "Definitions/HarmoniaInstancingSystemDefinitions.h"

#include "HarmoniaInstancedItemManager.generated.h"

class AActor;
class AController;

/**
 * ���¿���� �ν��Ͻ� ������Ʈ �Ŵ��� (UWorldSubsystem)
 * - �����͸� ���� (���� ���Ҹ�Ƽ)
 * - �÷��̾� ��ó ���� �� �������� ���� ����/�ı� ��û ó��
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaInstancedItemManager : public UHarmoniaInstancedObjectManagerBase
{
    GENERATED_BODY()

protected:
    // ���� ����: ������ ���̺���� Ŭ���� ã�� ���忡 Spawn
    virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) override;

    // ���� �ı�: �ʿ�� ������ �ݿ� �� �߰� ���� ����
    virtual void DestroyWorldActor(AActor* Actor) override;
};
