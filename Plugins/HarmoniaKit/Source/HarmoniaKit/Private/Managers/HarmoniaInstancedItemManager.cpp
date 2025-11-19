// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaInstancedItemManager.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Definitions/HarmoniaItemSystemDefinitions.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"

AActor* UHarmoniaInstancedItemManager::SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor)
{
    // TODO: Find actor class from data table or other sources as needed
    TSubclassOf<AActor> ItemActorClass = nullptr;

    // ------ Example implementation below ------
    // (lookup from data table/asset manager as appropriate)
    UDataTable* ItemDataTable = GETITEMDATATABLE();

    if (ItemDataTable)
    {
        const FItemData* Item = ItemDataTable->FindRow<FItemData>(Data.DataId, TEXT("FindItemRow"));
        if (!Item)
        {
            UE_LOG(LogTemp, Warning, TEXT("UHarmoniaInstancedItemManager::SpawnWorldActor - Item data not found for ItemId: %s"), *Data.DataId.ToString());
            return nullptr;
        }

        ItemActorClass = Item->WorldActorClass.LoadSynchronous();

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

            // (Optional) Set item data on spawned actor if needed
            // ex) IWorldItemInterface::SetItemData(Data);

            // (Optional) Interaction binding, effects, etc.
        }
        return SpawnedActor;
    }

    return nullptr;
}

void UHarmoniaInstancedItemManager::DestroyWorldActor(AActor* Actor)
{
    if (Actor)
    {
        // (Optional) Pre-destroy cleanup, effects, audio, etc.
        Actor->Destroy();
    }
}