// Copyright 2025 Snow Game Studio.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Definiitons/HarmoniaInstancingSystemDefinitions.h"
#include "Misc/Guid.h"

#include "HarmoniaInstancedObjectManagerBase.generated.h"

class AActor;
class AController;

/**
 * 인스턴스 오브젝트 베이스 매니저(추상)
 * - 인스턴싱 메시 ↔ 액터 스왑 등 공통 로직 제공
 * - 실제 스폰/데이터 확장은 파생에서 구현
 */
UCLASS(Abstract)
class HARMONIAINSTANCINGSYSTEM_API UHarmoniaInstancedObjectManagerBase : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // 인스턴스 추가/삭제 (공통)
    FGuid AddInstance(const FHarmoniaInstancedObjectData& Data);
    bool RemoveInstance(const FGuid& InstanceGuid);

    // 근접/상호작용 트리거(서버 Only)
    virtual void OnPlayerApproachingInstance(const FGuid& InstanceGuid, AController* Requestor);

    // (공통) 인스턴싱 메시 ↔ 액터 스왑 함수
    void SwapInstanceToActor(const FGuid& InstanceGuid, AController* Requestor);
    void SwapActorToInstance(const FGuid& InstanceGuid);

    // 파생에서 실제 액터 스폰/파괴 구현 (추상)
    virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) PURE_VIRTUAL(UHarmoniaInstancedObjectManagerBase::SpawnWorldActor, return nullptr;);
    virtual void DestroyWorldActor(AActor * Actor) PURE_VIRTUAL(UHarmoniaInstancedObjectManagerBase::DestroyWorldActor, );

    // 액터-인스턴스 매핑 등록/해제
    virtual void RegisterSpawnedActor(const FGuid& InstanceGuid, AActor* SpawnedActor);
    virtual void UnregisterSpawnedActor(const FGuid& InstanceGuid);

    // (공통) 관리용 데이터
    TMap<FGuid, FHarmoniaInstancedObjectData> InstanceMap;
    TMap<FGuid, TWeakObjectPtr<AActor>> GuidToActorMap;

protected:
    virtual void Initialize(FSubsystemCollectionBase & Collection) override;
    virtual void Deinitialize() override;
};
