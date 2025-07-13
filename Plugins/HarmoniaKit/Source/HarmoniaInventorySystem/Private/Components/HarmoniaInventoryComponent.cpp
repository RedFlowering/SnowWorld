// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInventoryComponent.h"

UHarmoniaInventoryComponent::UHarmoniaInventoryComponent()
{
    // PrimaryComponentTick.bCanEverTick = false;

    InventoryData.Slots.SetNum(InventoryData.MaxSlotCount);
}

void UHarmoniaInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UHarmoniaInventoryComponent::AddItem(const FHarmoniaID& ItemID, int32 Count, float Durabiliry)
{
    if (Count > 0)
    {
        for (FInventorySlot& Slot : InventoryData.Slots)
        {
            if (Slot.ItemID == ItemID && Slot.Count > 0)
            {
                Slot.Durability = Durabiliry;
                Slot.Count += Count;
                return true;
            }
        }

        if (InventoryData.Slots.Num() != InventoryData.MaxSlotCount)
        {
            for (FInventorySlot& Slot : InventoryData.Slots)
            {
                if (Slot.Count == 0)
                {
                    Slot.ItemID = ItemID;
                    Slot.Durability = Durabiliry;
                    Slot.Count = Count;
                    return true;
                }
            }
        }
    }

    return false;
}

bool UHarmoniaInventoryComponent::RemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durabiliry)
{
    if (Count <= 0) return false;

    for (FInventorySlot& Slot : InventoryData.Slots)
    {
        if (Slot.ItemID == ItemID && Slot.Count > 0)
        {
            if (Slot.Count >= Count)
            {
                Slot.Durability = 1.f;
                Slot.Count -= Count;

                if (Slot.Count == 0)
                {
                    Slot.ItemID = FHarmoniaID();
                    Slot.Durability = 0.f;
                }
                return true;
            }
        }
    }
    return false;
}

void UHarmoniaInventoryComponent::SwapSlots(int32 SlotA, int32 SlotB)
{
    if (InventoryData.Slots.IsValidIndex(SlotA) && InventoryData.Slots.IsValidIndex(SlotB) && SlotA != SlotB)
    {
        InventoryData.Slots.Swap(SlotA, SlotB);
    }
}

void UHarmoniaInventoryComponent::Clear()
{
    for (FInventorySlot& Slot : InventoryData.Slots)
    {
        Slot.ItemID = FHarmoniaID();
        Slot.Count = 0;
    }
}

int32 UHarmoniaInventoryComponent::GetTotalCount(const FHarmoniaID& ItemID) const
{
    int32 Total = 0;
    for (const FInventorySlot& Slot : InventoryData.Slots)
    {
        if (Slot.ItemID == ItemID)
        {
            Total += Slot.Count;
        }
    }
    return Total;
}