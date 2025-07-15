// Copyright 2025 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"

#include "HarmoniaInventoryComponent.generated.h"

class AHarmoniaItemActor;

// 드랍 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHarmoniaInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable, Category="Inventory")
    void RequestPickupItem(AHarmoniaItemActor* Item);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    void RequestDropItem(int32 SlotIndex);

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

protected:
    void PickupItem(AHarmoniaItemActor* Item);

	UFUNCTION(Server, Reliable)
    void ServerPickupItem(AHarmoniaItemActor* Item);

    void DropItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerDropItem(int32 SlotIndex);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FInventoryData InventoryData = FInventoryData();

    // UI/위젯에서 바인딩할 수 있는 변경 이벤트
    UPROPERTY(BlueprintAssignable)
    FOnInventoryChanged OnInventoryChanged;
};