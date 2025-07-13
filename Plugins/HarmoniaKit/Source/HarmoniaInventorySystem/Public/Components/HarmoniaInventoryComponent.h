// Copyright 2025 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"

#include "HarmoniaInventoryComponent.generated.h"

UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAINVENTORYSYSTEM_API UHarmoniaInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHarmoniaInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool AddItem(const FHarmoniaID& ItemID, int32 Count, float Durabiliry);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool RemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durabiliry);

    UFUNCTION(BlueprintCallable)
    void SwapSlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    void Clear();

    UFUNCTION(BlueprintCallable, Category="Inventory")
    int32 GetTotalCount(const FHarmoniaID& ItemID) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FInventoryData InventoryData = FInventoryData();
};