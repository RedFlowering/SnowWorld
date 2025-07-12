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
    bool AddItem(const FItemData& Item, int32 Count);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool RemoveItem(const FItemData& Item, int32 Count);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    void Clear();

    UFUNCTION(BlueprintCallable, Category="Inventory")
    int32 GetTotalCount(const FItemData& Item) const;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    TArray<FInventorySlot> Slots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    int32 MaxSlotCount = 30;
};