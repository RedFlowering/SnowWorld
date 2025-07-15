// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaInstancedObjectManagerBase.h"
#include "Definitions/HarmoniaInstancingSystemDefinitions.h"

#include "HarmoniaInstancedItemManager.generated.h"

class AActor;
class AController;

/**
 * 오픈월드용 인스턴스 오브젝트 매니저 (UWorldSubsystem)
 * - 데이터만 관리 (서버 오소리티)
 * - 플레이어 근처 진입 시 서버에서 액터 스폰/파괴 요청 처리
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaInstancedItemManager : public UHarmoniaInstancedObjectManagerBase
{
    GENERATED_BODY()

protected:
    // 액터 스폰: 아이템 테이블에서 클래스 찾아 월드에 Spawn
    virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) override;

    // 액터 파괴: 필요시 데이터 반영 등 추가 구현 가능
    virtual void DestroyWorldActor(AActor* Actor) override;
};
