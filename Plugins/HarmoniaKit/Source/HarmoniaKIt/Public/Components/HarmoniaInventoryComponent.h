// Copyright 2025 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"

#include "HarmoniaInventoryComponent.generated.h"

class AHarmoniaItemActor;

// ��� �̺�Ʈ ��������Ʈ
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION()
	void OnRep_InventoryData();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestPickupItem(AHarmoniaItemActor* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestDropItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestAddItem(const FHarmoniaID& ItemID, int32 Count, float Durability);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestRemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durability);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSwapSlots(int32 SlotA, int32 SlotB);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestClear();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetTotalCount(const FHarmoniaID& ItemID) const;

	// ============================================================================
	// Internal Operations (Server-only, for plugin-internal use)
	// WARNING: Do not call these directly from blueprints or client code!
	// Use Request* functions instead for proper server validation.
	// ============================================================================

	/**
	 * Add item to inventory (Server-only, internal use)
	 * NOTE: Only call from plugin-internal code (Crafting, Building, Quest systems)
	 */
	bool AddItem(const FHarmoniaID& ItemID, int32 Count, float Durability);

	/**
	 * Remove item from inventory (Server-only, internal use)
	 * NOTE: Only call from plugin-internal code (Crafting, Building, Quest systems)
	 */
	bool RemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durability);

	/**
	 * Swap two inventory slots (Server-only, internal use)
	 * NOTE: Only call from plugin-internal code (UI widgets with proper validation)
	 */
	void SwapSlots(int32 SlotA, int32 SlotB);

protected:
	void PickupItem(AHarmoniaItemActor* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickupItem(AHarmoniaItemActor* Item);

	void DropItem(int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItem(int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(const FHarmoniaID& ItemID, int32 Count, float Durability);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(const FHarmoniaID& ItemID, int32 Count, float Durability);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwapSlots(int32 SlotA, int32 SlotB);

	void Clear();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerClear();

public:
	// ���� <-> ���� Ŭ�� ����ȭ
	UPROPERTY(ReplicatedUsing = OnRep_InventoryData, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FInventoryData InventoryData = FInventoryData();

	// Ŭ�� UI���� ���ε��ϴ� ���� �̺�Ʈ
	UPROPERTY(BlueprintAssignable)
	FOnInventoryChanged OnInventoryChanged;
};