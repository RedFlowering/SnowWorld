// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaInventoryComponent.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Actors/HarmoniaItemActor.h"
#include "Net/UnrealNetwork.h"

UHarmoniaInventoryComponent::UHarmoniaInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void UHarmoniaInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryData.Slots.SetNum(InventoryData.MaxSlotCount);
}

void UHarmoniaInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 소유자(오너) 클라이언트만 복제화
	DOREPLIFETIME_CONDITION(UHarmoniaInventoryComponent, InventoryData, COND_OwnerOnly);
}

void UHarmoniaInventoryComponent::OnRep_InventoryData()
{
	// 클라이언트에서 브로드캐스트 및 UI 자동 업데이트
	OnInventoryChanged.Broadcast();
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

void UHarmoniaInventoryComponent::RequestAddItem(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		AddItem(ItemID, Count, Durability);
	}
	else
	{
		ServerAddItem(ItemID, Count, Durability);
	}
}

void UHarmoniaInventoryComponent::RequestRemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RemoveItem(ItemID, Count, Durability);
	}
	else
	{
		ServerRemoveItem(ItemID, Count, Durability);
	}
}

void UHarmoniaInventoryComponent::RequestSwapSlots(int32 SlotA, int32 SlotB)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SwapSlots(SlotA, SlotB);
	}
	else
	{
		ServerSwapSlots(SlotA, SlotB);
	}
}

void UHarmoniaInventoryComponent::RequestClear()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Clear();
	}
	else
	{
		ServerClear();
	}
}

bool UHarmoniaInventoryComponent::AddItem(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	// Server-only execution
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (Count > 0)
	{
		for (FInventorySlot& Slot : InventoryData.Slots)
		{
			if (Slot.ItemID == ItemID && Slot.Count > 0)
			{
				Slot.Durability = Durability;
				Slot.Count += Count;
				OnInventoryChanged.Broadcast(); // Server-side broadcast
				return true;
			}
		}

		// Num != MaxSlotCount 체크는 BeginPlay에서 이미 했으므로 불필요
		for (FInventorySlot& Slot : InventoryData.Slots)
		{
			if (Slot.Count == 0)
			{
				Slot.ItemID = ItemID;
				Slot.Durability = Durability;
				Slot.Count = Count;
				OnInventoryChanged.Broadcast(); // Server-side broadcast
				return true;
			}
		}
	}

	return false;
}

bool UHarmoniaInventoryComponent::RemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	// Server-only execution
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (Count <= 0) return false;

	for (FInventorySlot& Slot : InventoryData.Slots)
	{
		if (Slot.ItemID == ItemID && Slot.Count > 0)
		{
			if (Slot.Count >= Count)
			{
				Slot.Count -= Count;

				if (Slot.Count == 0)
				{
					Slot.ItemID = FHarmoniaID();
					Slot.Durability = 0.f;
				}
				OnInventoryChanged.Broadcast(); // Server-side broadcast
				return true;
			}
		}
	}
	return false;
}

void UHarmoniaInventoryComponent::SwapSlots(int32 SlotA, int32 SlotB)
{
	// Server-only execution
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (InventoryData.Slots.IsValidIndex(SlotA) && InventoryData.Slots.IsValidIndex(SlotB) && SlotA != SlotB)
	{
		InventoryData.Slots.Swap(SlotA, SlotB);
		OnInventoryChanged.Broadcast(); // Server-side broadcast
	}
}

void UHarmoniaInventoryComponent::Clear()
{
	// Server-only execution
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (FInventorySlot& Slot : InventoryData.Slots)
	{
		Slot.ItemID = FHarmoniaID();
		Slot.Count = 0;
	}
	OnInventoryChanged.Broadcast(); // Server-side broadcast
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
	if (GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		if (AddItem(Item->ItemID, Item->Count, Item->Durability))
		{
			Item->Destroy();
		}
	}
}

void UHarmoniaInventoryComponent::ServerPickupItem_Implementation(AHarmoniaItemActor* Item)
{
	PickupItem(Item);
}

bool UHarmoniaInventoryComponent::ServerPickupItem_Validate(AHarmoniaItemActor* Item)
{
	// Anti-cheat: Validate item is not null
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerPickupItem: Item is null"));
		return false;
	}

	// Validate item is within pickup range
	AActor* Owner = GetOwner();
	if (Owner)
	{
		float Distance = FVector::Dist(Owner->GetActorLocation(), Item->GetActorLocation());
		const float MaxPickupDistance = 500.0f;
		if (Distance > MaxPickupDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerPickupItem: Item too far (%.1f > %.1f)"), Distance, MaxPickupDistance);
			return false;
		}
	}

	return true;
}

void UHarmoniaInventoryComponent::DropItem(int32 SlotIndex)
{
	if (GetOwner() && GetOwner()->HasAuthority() && InventoryData.Slots.IsValidIndex(SlotIndex))
	{
		FInventorySlot& Slot = InventoryData.Slots[SlotIndex];

		// 슬롯에서 아이템 정보를 가져옴
		if (Slot.ItemID.IsValid() && Slot.Count != 0)
		{
			UDataTable* ItemDataTable = GETITEMDATATABLE();

			if (ItemDataTable)
			{
				FItemData* Item = ItemDataTable->FindRow<FItemData>(Slot.ItemID.Id, TEXT("FindItemRow"));

				// 월드에 아이템 액터 스폰
				if (Item && Item->WorldActorClass)
				{
					AActor* OwnerActor = GetOwner();
					FVector DropLocation = OwnerActor ? OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * 100.f : FVector::ZeroVector;
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = OwnerActor;

					AHarmoniaItemActor* DropActor = GetWorld()->SpawnActor<AHarmoniaItemActor>(
						Item->WorldActorClass.LoadSynchronous(),
						DropLocation,
						FRotator::ZeroRotator,
						SpawnParams
					);

					if (DropActor)
					{
						DropActor->InitItem(Slot.ItemID, Slot.Count, Slot.Durability);
					}
				}

				// 인벤토리 슬롯 초기화
				Slot = FInventorySlot();
			}
		}
	}
}

void UHarmoniaInventoryComponent::ServerDropItem_Implementation(int32 SlotIndex)
{
	DropItem(SlotIndex);
}

bool UHarmoniaInventoryComponent::ServerDropItem_Validate(int32 SlotIndex)
{
	// Anti-cheat: Validate slot index
	if (!InventoryData.Slots.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerDropItem: Invalid slot index %d (Max: %d)"), SlotIndex, InventoryData.Slots.Num() - 1);
		return false;
	}

	return true;
}

void UHarmoniaInventoryComponent::ServerAddItem_Implementation(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	AddItem(ItemID, Count, Durability);
}

bool UHarmoniaInventoryComponent::ServerAddItem_Validate(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	// Anti-cheat: Validate item count
	if (Count <= 0 || Count > 9999)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerAddItem: Invalid count %d for item %s"), Count, *ItemID.ToString());
		return false;
	}

	// Validate item ID
	if (!ItemID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerAddItem: Invalid ItemID"));
		return false;
	}

	return true;
}

void UHarmoniaInventoryComponent::ServerRemoveItem_Implementation(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	RemoveItem(ItemID, Count, Durability);
}

bool UHarmoniaInventoryComponent::ServerRemoveItem_Validate(const FHarmoniaID& ItemID, int32 Count, float Durability)
{
	// Anti-cheat: Validate item count
	if (Count <= 0 || Count > 9999)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerRemoveItem: Invalid count %d for item %s"), Count, *ItemID.ToString());
		return false;
	}

	// Validate item ID
	if (!ItemID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerRemoveItem: Invalid ItemID"));
		return false;
	}

	// Validate player actually has the item
	int32 CurrentCount = GetTotalCount(ItemID);
	if (CurrentCount < Count)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerRemoveItem: Player doesn't have enough items (Has: %d, Trying to remove: %d)"), CurrentCount, Count);
		return false;
	}

	return true;
}

void UHarmoniaInventoryComponent::ServerSwapSlots_Implementation(int32 SlotA, int32 SlotB)
{
	SwapSlots(SlotA, SlotB);
}

bool UHarmoniaInventoryComponent::ServerSwapSlots_Validate(int32 SlotA, int32 SlotB)
{
	// Anti-cheat: Validate slot indices
	if (!InventoryData.Slots.IsValidIndex(SlotA) || !InventoryData.Slots.IsValidIndex(SlotB))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSwapSlots: Invalid slot indices (A: %d, B: %d, Max: %d)"),
			SlotA, SlotB, InventoryData.Slots.Num() - 1);
		return false;
	}

	if (SlotA == SlotB)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerSwapSlots: Cannot swap slot with itself"));
		return false;
	}

	return true;
}

void UHarmoniaInventoryComponent::ServerClear_Implementation()
{
	Clear();
}

bool UHarmoniaInventoryComponent::ServerClear_Validate()
{
	// [SECURITY] This is a dangerous operation that allows clearing the entire inventory
	// Consider requiring GM/Admin permission or removing this RPC entirely
	// For now, disabled for security - implement permission check if needed
	UE_LOG(LogTemp, Warning, TEXT("[SECURITY] ServerClear called - this operation is restricted for security"));

	// TODO: Add permission check here if this functionality is needed
	// Example: return HasAdminPermission(GetOwner());

	return false; // Disabled by default for security
}