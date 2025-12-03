// Copyright 2025 Snow Game Studio.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Misc/Guid.h"
#include "Definitions/HarmoniaInstancingSystemDefinitions.h"

#include "HarmoniaInstancedObjectManagerBase.generated.h"

class AActor;
class AController;

/**
 * �ν��Ͻ� ������Ʈ ���̽� �Ŵ���(�߻�)
 * - �ν��Ͻ� �޽� �� ���� ���� �� ���� ���� ����
 * - ���� ����/������ Ȯ���� �Ļ����� ����
 */
UCLASS(Abstract)
class HARMONIAKIT_API UHarmoniaInstancedObjectManagerBase : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // �ν��Ͻ� �߰�/���� (����)
    FGuid AddInstance(const FHarmoniaInstancedObjectData& Data);
    bool RemoveInstance(const FGuid& InstanceGuid);

    // ����/��ȣ�ۿ� Ʈ����(���� Only)
    virtual void OnPlayerApproachingInstance(const FGuid& InstanceGuid, AController* Requestor);

    // (����) �ν��Ͻ� �޽� �� ���� ���� �Լ�
    void SwapInstanceToActor(const FGuid& InstanceGuid, AController* Requestor);
    void SwapActorToInstance(const FGuid& InstanceGuid);

    // �Ļ����� ���� ���� ����/�ı� ���� (�߻�)
    virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) PURE_VIRTUAL(UHarmoniaInstancedObjectManagerBase::SpawnWorldActor, return nullptr;);
    virtual void DestroyWorldActor(AActor * Actor) PURE_VIRTUAL(UHarmoniaInstancedObjectManagerBase::DestroyWorldActor, );

    // ����-�ν��Ͻ� ���� ���/����
    virtual void RegisterSpawnedActor(const FGuid& InstanceGuid, AActor* SpawnedActor);
    virtual void UnregisterSpawnedActor(const FGuid& InstanceGuid);

    // (����) ������ ������
    TMap<FGuid, FHarmoniaInstancedObjectData> InstanceMap;
    TMap<FGuid, TWeakObjectPtr<AActor>> GuidToActorMap;

protected:
    virtual void Initialize(FSubsystemCollectionBase & Collection) override;
    virtual void Deinitialize() override;
};
