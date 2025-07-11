// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInventoryComponent.h"

UHarmoniaInventoryComponent::UHarmoniaInventoryComponent()
{
    // PrimaryComponentTick.bCanEverTick = false;

    Slots.SetNum(MaxSlotCount);
}

void UHarmoniaInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UHarmoniaInventoryComponent::AddItem(const FItemID& ItemId, int32 Count)
{
    if (Count > 0)
    {
        for (FInventorySlot& Slot : Slots)
        {
            if (Slot.ItemID == ItemId && Slot.Count > 0)
            {
                Slot.Count += Count;
                return true;
            }
        }

        for (FInventorySlot& Slot : Slots)
        {
            if (Slot.Count == 0)
            {
                Slot.ItemID = ItemId;
                Slot.Count = Count;
                return true;
            }
        }
    }



    return false;
}

bool UHarmoniaInventoryComponent::RemoveItem(const FItemID& ItemId, int32 Count)
{
    if (Count <= 0) return false;

    for (FInventorySlot& Slot : Slots)
    {
        if (Slot.ItemID == ItemId && Slot.Count > 0)
        {
            if (Slot.Count >= Count)
            {
                Slot.Count -= Count;
                if (Slot.Count == 0)
                {
                    Slot.ItemID = FItemID();
                }
                return true;
            }
        }
    }
    return false;
}

void UHarmoniaInventoryComponent::Clear()
{
    for (FInventorySlot& Slot : Slots)
    {
        Slot.ItemID = FItemID();
        Slot.Count = 0;
    }
}

int32 UHarmoniaInventoryComponent::GetTotalCount(const FItemID& ItemId) const
{
    int32 Total = 0;
    for (const FInventorySlot& Slot : Slots)
    {
        if (Slot.ItemID == ItemId)
        {
            Total += Slot.Count;
        }
    }
    return Total;
}