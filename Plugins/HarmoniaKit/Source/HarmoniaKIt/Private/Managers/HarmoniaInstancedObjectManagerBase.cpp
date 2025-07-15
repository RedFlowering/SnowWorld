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

    // Actor가 스폰된 상태면 먼저 파괴
    SwapActorToInstance(InstanceGuid);

    InstanceMap.Remove(InstanceGuid);
    return true;
}

// ===== 공통 스왑 로직 (Instance <-> Actor) =====

void UHarmoniaInstancedObjectManagerBase::OnPlayerApproachingInstance(const FGuid& InstanceGuid, AController* Requestor)
{
    // 파생에서 필요하면 오버라이드하여 센스 시스템 연동
    // 기본: 근접 시 인스턴스를 액터로 스왑
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

// ===== 가상 함수(Spawn/Destroy)는 파생 매니저에서 반드시 구현 =====
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