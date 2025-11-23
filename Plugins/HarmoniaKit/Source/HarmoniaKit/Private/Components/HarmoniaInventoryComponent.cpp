// Copyright 2025 Snow Game Studio.

#include "HarmoniaLogCategories.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Actors/HarmoniaItemActor.h"
#include "Interfaces/HarmoniaAdminInterface.h"
#include "GameFramework/PlayerState.h"
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
	// [SECURITY] Clear is a dangerous operation that requires Admin permissions
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[SECURITY] RequestClear: Must be called on server"));
		return;
	}

	// Check admin permissions via PlayerState
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[SECURITY] RequestClear: Owner is not a Pawn"));
		return;
	}

	APlayerState* PlayerState = OwnerPawn->GetPlayerState();
	if (!PlayerState)
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[SECURITY] RequestClear: No PlayerState found"));
		return;
	}

	// Check if PlayerState implements admin interface
	IHarmoniaAdminInterface* AdminInterface = Cast<IHarmoniaAdminInterface>(PlayerState);
	if (!AdminInterface)
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[SECURITY] RequestClear: Player does not have admin interface - operation denied"));
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[SECURITY] To enable this feature, implement IHarmoniaAdminInterface in your PlayerState"));
		return;
	}

	// Require Admin level or higher
	if (!AdminInterface->Execute_HasAdminPermission(PlayerState, EHarmoniaAdminLevel::Admin))
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[SECURITY] RequestClear: Insufficient admin permissions (requires Admin level)"));
		return;
	}

	// Permission granted - execute clear operation
	UE_LOG(LogHarmoniaInventory, Log, TEXT("[ADMIN] Inventory cleared by player %s with Admin permissions"), *PlayerState->GetPlayerName());
	Clear();
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
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerPickupItem: Item is null"));
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
			UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerPickupItem: Item too far (%.1f > %.1f)"), Distance, MaxPickupDistance);
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
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerDropItem: Invalid slot index %d (Max: %d)"), SlotIndex, InventoryData.Slots.Num() - 1);
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
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerAddItem: Invalid count %d for item %s"), Count, *ItemID.ToString());
		return false;
	}

	// Validate item ID
	if (!ItemID.IsValid())
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerAddItem: Invalid ItemID"));
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
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerRemoveItem: Invalid count %d for item %s"), Count, *ItemID.ToString());
		return false;
	}

	// Validate item ID
	if (!ItemID.IsValid())
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerRemoveItem: Invalid ItemID"));
		return false;
	}

	// Validate player actually has the item
	int32 CurrentCount = GetTotalCount(ItemID);
	if (CurrentCount < Count)
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerRemoveItem: Player doesn't have enough items (Has: %d, Trying to remove: %d)"), CurrentCount, Count);
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
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerSwapSlots: Invalid slot indices (A: %d, B: %d, Max: %d)"),
			SlotA, SlotB, InventoryData.Slots.Num() - 1);
		return false;
	}

	if (SlotA == SlotB)
	{
		UE_LOG(LogHarmoniaInventory, Warning, TEXT("[ANTI-CHEAT] ServerSwapSlots: Cannot swap slot with itself"));
		return false;
	}

	return true;
}

void UHarmoniaInventoryComponent::RequestSortInventory(EInventorySortMethod Method)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SortInventory(Method);
	}
	else
	{
		ServerSortInventory(Method);
	}
}

void UHarmoniaInventoryComponent::SortInventory(EInventorySortMethod Method)
{
	// Server-only execution
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Sort logic
	InventoryData.Slots.Sort([Method](const FInventorySlot& A, const FInventorySlot& B)
	{
		// Empty slots always go to the end
		bool bAValid = A.ItemID.IsValid() && A.Count > 0;
		bool bBValid = B.ItemID.IsValid() && B.Count > 0;

		if (bAValid != bBValid)
		{
			return bAValid; // Valid items come first
		}

		if (!bAValid && !bBValid)
		{
			return false; // Both empty, keep order
		}

		// Both are valid, sort based on method
		switch (Method)
		{
		case EInventorySortMethod::Name:
			// Sort by ID (Name)
			if (A.ItemID.Id != B.ItemID.Id)
			{
				return A.ItemID.Id.LexicalLess(B.ItemID.Id);
			}
			break;

		case EInventorySortMethod::Count:
			// Sort by Count (Descending)
			if (A.Count != B.Count)
			{
				return A.Count > B.Count;
			}
			break;

		case EInventorySortMethod::Type:
		case EInventorySortMethod::Rarity:
			// Fallback to Name sort for now as Type/Rarity requires lookup
			if (A.ItemID.Id != B.ItemID.Id)
			{
				return A.ItemID.Id.LexicalLess(B.ItemID.Id);
			}
			break;
		}

		// Secondary sort: Name (if primary sort was equal or not Name)
		if (Method != EInventorySortMethod::Name)
		{
			return A.ItemID.Id.LexicalLess(B.ItemID.Id);
		}

		return false;
	});

	// Re-assign indices
	for (int32 i = 0; i < InventoryData.Slots.Num(); ++i)
	{
		InventoryData.Slots[i].Index = i;
	}

	OnInventoryChanged.Broadcast(); // Server-side broadcast
}

void UHarmoniaInventoryComponent::ServerSortInventory_Implementation(EInventorySortMethod Method)
{
	SortInventory(Method);
}

bool UHarmoniaInventoryComponent::ServerSortInventory_Validate(EInventorySortMethod Method)
{
	return true;
}