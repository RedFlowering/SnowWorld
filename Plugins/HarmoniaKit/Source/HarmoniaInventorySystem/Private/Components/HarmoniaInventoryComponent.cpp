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

bool UHarmoniaInventoryComponent::AddItem(const FItemData& Item, int32 Count)
{
    if (Count > 0)
    {
        for (FInventorySlot& Slot : Slots)
        {
            if (Slot.ItemID == Item.ItemId && Slot.Count > 0)
            {
                Slot.Durability = Item.Durabirity;
                Slot.Count += Count;
                return true;
            }
        }

        for (FInventorySlot& Slot : Slots)
        {
            if (Slot.Count == 0)
            {
                Slot.ItemID = Item.ItemId;
                Slot.Durability = Item.Durabirity;
                Slot.Count = Count;
                return true;
            }
        }
    }

    return false;
}

bool UHarmoniaInventoryComponent::RemoveItem(const FItemData& Item, int32 Count)
{
    if (Count <= 0) return false;

    for (FInventorySlot& Slot : Slots)
    {
        if (Slot.ItemID == Item.ItemId && Slot.Count > 0)
        {
            if (Slot.Count >= Count)
            {
                Slot.Durability = 1.f;
                Slot.Count -= Count;

                if (Slot.Count == 0)
                {
                    Slot.ItemID = FItemID();
                    Slot.Durability = 0.f;
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

int32 UHarmoniaInventoryComponent::GetTotalCount(const FItemData& Item) const
{
    int32 Total = 0;
    for (const FInventorySlot& Slot : Slots)
    {
        if (Slot.ItemID == Item.ItemId)
        {
            Total += Slot.Count;
        }
    }
    return Total;
}