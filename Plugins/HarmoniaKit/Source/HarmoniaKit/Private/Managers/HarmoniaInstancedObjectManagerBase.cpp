// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInstancedObjectManagerBase.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"

void UHarmoniaInstancedObjectManagerBase::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InstanceMap.Empty();
    GuidToActorMap.Empty();
}

void UHarmoniaInstancedObjectManagerBase::Deinitialize()
{
    InstanceMap.Empty();
    GuidToActorMap.Empty();
    Super::Deinitialize();
}

// ===== Instance Data Management =====

FGuid UHarmoniaInstancedObjectManagerBase::AddInstance(const FHarmoniaInstancedObjectData& Data)
{
    FGuid Guid = Data.InstanceGuid.IsValid() ? Data.InstanceGuid : FGuid::NewGuid();
    FHarmoniaInstancedObjectData Inserted = Data;
    Inserted.InstanceGuid = Guid;
    InstanceMap.Add(Guid, Inserted);
    return Guid;
}

bool UHarmoniaInstancedObjectManagerBase::RemoveInstance(const FGuid& InstanceGuid)
{
    if (!InstanceMap.Contains(InstanceGuid))
        return false;

    // Actor�� ������ ���¸� ���� �ı�
    SwapActorToInstance(InstanceGuid);

    InstanceMap.Remove(InstanceGuid);
    return true;
}

// ===== ���� ���� ���� (Instance <-> Actor) =====

void UHarmoniaInstancedObjectManagerBase::OnPlayerApproachingInstance(const FGuid& InstanceGuid, AController* Requestor)
{
    // �Ļ����� �ʿ��ϸ� �������̵��Ͽ� ���� �ý��� ����
    // �⺻: ���� �� �ν��Ͻ��� ���ͷ� ����
    SwapInstanceToActor(InstanceGuid, Requestor);
}

void UHarmoniaInstancedObjectManagerBase::SwapInstanceToActor(const FGuid& InstanceGuid, AController* Requestor)
{
    if (!InstanceMap.Contains(InstanceGuid)) return;
    if (GuidToActorMap.Contains(InstanceGuid) && GuidToActorMap[InstanceGuid].IsValid())
        return;

    AActor* Spawned = SpawnWorldActor(InstanceMap[InstanceGuid], Requestor);
    if (Spawned)
        GuidToActorMap.Add(InstanceGuid, Spawned);
}

void UHarmoniaInstancedObjectManagerBase::SwapActorToInstance(const FGuid& InstanceGuid)
{
    if (!GuidToActorMap.Contains(InstanceGuid)) return;

    TWeakObjectPtr<AActor> ActorPtr = GuidToActorMap[InstanceGuid];
    if (ActorPtr.IsValid())
        DestroyWorldActor(ActorPtr.Get());

    GuidToActorMap.Remove(InstanceGuid);
}

// ===== ���� �Լ�(Spawn/Destroy)�� �Ļ� �Ŵ������� �ݵ�� ���� =====
void UHarmoniaInstancedObjectManagerBase::RegisterSpawnedActor(const FGuid& InstanceGuid, AActor* SpawnedActor)
{
    if (InstanceGuid.IsValid() && SpawnedActor)
    {
        GuidToActorMap.Add(InstanceGuid, SpawnedActor);
    }
}

void UHarmoniaInstancedObjectManagerBase::UnregisterSpawnedActor(const FGuid& InstanceGuid)
{
    GuidToActorMap.Remove(InstanceGuid);
}