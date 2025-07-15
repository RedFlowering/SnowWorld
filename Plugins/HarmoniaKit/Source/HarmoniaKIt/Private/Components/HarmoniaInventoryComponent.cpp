// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInventoryComponent.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Actors/HarmoniaItemActor.h"


UHarmoniaInventoryComponent::UHarmoniaInventoryComponent()
{
    // PrimaryComponentTick.bCanEverTick = false;

    InventoryData.Slots.SetNum(InventoryData.MaxSlotCount);
}

void UHarmoniaInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UHarmoniaInventoryComponent::RequestPickupItem(AHarmoniaItemActor* Item)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        PickupItem(Item);
    }
    else
    {
        ServerPickupItem(Item);
    }
}

void UHarmoniaInventoryComponent::RequestDropItem(int32 SlotIndex)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        DropItem(SlotIndex);
    }
    else
    {
        ServerDropItem(SlotIndex);
    }
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

void UHarmoniaInventoryComponent::PickupItem(AHarmoniaItemActor* Item)
{
    AHarmoniaItemActor* ItemActor = Cast<AHarmoniaItemActor>(Item);

    if (GetOwner() && GetOwner()->HasAuthority() && ItemActor)
    {
        if (AddItem(ItemActor->ItemID, ItemActor->Count, ItemActor->Durability))
        {
            ItemActor->Destroy();
        }
    }
}

void UHarmoniaInventoryComponent::ServerPickupItem_Implementation(AHarmoniaItemActor* Item)
{
    PickupItem(Item);
}

void UHarmoniaInventoryComponent::DropItem(int32 SlotIndex)
{
    if (GetOwner() && GetOwner()->HasAuthority() && InventoryData.Slots.IsValidIndex(SlotIndex))
    {
        FInventorySlot& Slot = InventoryData.Slots[SlotIndex];

        // 아이템이 비어 있으면 패스
        if (Slot.ItemID.Id != NAME_None && Slot.Count != 0)
        {
            UDataTable* ItemDataTable = GETITEMDATATABLE();

            if (ItemDataTable)
            {
                FItemData* Item = ItemDataTable->FindRow<FItemData>(Slot.ItemID.Id, TEXT("FindItemRow"));

                // 월드에 드랍 액터 생성
                if (Item && Item->WorldActorClass)
                {
                    // 드랍 위치는 보통 소유 액터의 위치 or 카메라 전방 (더 고도화 가능)
                    AActor* OwnerActor = GetOwner();
                    FVector DropLocation = OwnerActor ? OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 100.f : FVector::ZeroVector;
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.Owner = OwnerActor;

                    AHarmoniaItemActor* DropActor = GetWorld()->SpawnActor<AHarmoniaItemActor>(Item->WorldActorClass.LoadSynchronous(), DropLocation, FRotator::ZeroRotator, SpawnParams);

                    if (DropActor)
                    {
                        // DropActor에 아이템 데이터 전달 (슬롯 데이터, 수량 등)
                        DropActor->InitItem(Slot.ItemID, Slot.Count, Slot.Durability);
                    }
                }

                // 인벤토리 슬롯 비우기
                Slot = FInventorySlot(); // (멤버 변수가 전부 0, None 등으로 초기화된 빈 슬롯)

                // 변경 델리게이트 브로드캐스트 (위젯에 바인딩되어 있으면 자동 갱신)
                OnInventoryChanged.Broadcast();
            }
        }
    }
}

void UHarmoniaInventoryComponent::ServerDropItem_Implementation(int32 SlotIndex)
{
    DropItem(SlotIndex);
}