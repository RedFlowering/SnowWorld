// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInstancedItemManager.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Definitions/HarmoniaItemSystemDefinitions.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"

AActor* UHarmoniaInstancedItemManager::SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor)
{
    // 예시: 아이템 테이블 또는 매핑 맵에서 액터 클래스 찾기
    TSubclassOf<AActor> ItemActorClass = nullptr;

    // ------ 예시 매핑 코드 시작 ------
    // (실전에서는 테이블 매니저/에셋 매니저 등에서 조회)
    UDataTable* ItemDataTable = GETITEMDATATABLE();

    if (ItemDataTable)
    {
        const FItemData* Item = ItemDataTable->FindRow<FItemData>(Data.DataId, TEXT("FindItemRow"));
        ItemActorClass = Item->WorldActorClass->GetClass();

        if (!ItemActorClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("UHarmoniaInstancedItemManager::SpawnWorldActor - No ActorClass for ItemId: %s"), *Data.DataId.ToString());
            return nullptr;
        }

        UWorld* World = GetWorld();
        if (!World) return nullptr;

        FActorSpawnParameters Params;
        Params.Owner = Requestor;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* SpawnedActor = World->SpawnActor<AActor>(ItemActorClass, Data.WorldTransform, Params);
        if (SpawnedActor)
        {
            RegisterSpawnedActor(Data.InstanceGuid, SpawnedActor);

            // (필요하다면) 액터에 아이템 데이터 주입 등 추가 처리
            // ex) IWorldItemInterface::SetItemData(Data);

            // (옵션) 상호작용 바인딩, 이펙트 등
        }
        return SpawnedActor;
    }

    return nullptr;
}

void UHarmoniaInstancedItemManager::DestroyWorldActor(AActor* Actor)
{
    if (Actor)
    {
        // (필요시) 파괴 전에 데이터, 이펙트, 루팅 처리 등
        Actor->Destroy();
    }
}