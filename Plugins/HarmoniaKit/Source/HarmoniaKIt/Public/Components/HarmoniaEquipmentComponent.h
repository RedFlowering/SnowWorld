// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "GameplayEffectTypes.h"
#include "HarmoniaEquipmentComponent.generated.h"

class UAbilitySystemComponent;
class UHarmoniaAttributeSet;
class UDataTable;
struct FEquipmentData;

/**
 * Delegate for equipment change events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnEquipmentChanged,
	EEquipmentSlot, Slot,
	const FHarmoniaID&, OldEquipmentId,
	const FHarmoniaID&, NewEquipmentId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnEquipmentStatsChanged,
	EEquipmentSlot, Slot,
	const TArray<FEquipmentStatModifier>&, StatModifiers);

/**
 * UHarmoniaEquipmentComponent
 *
 * Component for managing character equipment
 * - Equipment slot management
 * - Equip/Unequip functionality
 * - Stat application to AttributeSet
 * - Gameplay Effect application
 * - Visual mesh management
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaEquipmentComponent();

	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	// ============================================================================
	// Equipment Management
	// ============================================================================

	/**
	 * Equip an item to a specific slot
	 * @param EquipmentId - ID of the equipment to equip
	 * @param Slot - Slot to equip to (if None, will use equipment's default slot)
	 * @return true if successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	bool EquipItem(const FHarmoniaID& EquipmentId, EEquipmentSlot Slot = EEquipmentSlot::None);

	/**
	 * Unequip an item from a specific slot
	 * @param Slot - Slot to unequip from
	 * @return true if successfully unequipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	bool UnequipItem(EEquipmentSlot Slot);

	/**
	 * Swap equipment between two slots
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	bool SwapEquipment(EEquipmentSlot SlotA, EEquipmentSlot SlotB);

	/**
	 * Get equipped item in a specific slot
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	FEquippedItem GetEquippedItem(EEquipmentSlot Slot) const;

	/**
	 * Check if a slot has equipment
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	bool IsSlotEquipped(EEquipmentSlot Slot) const;

	/**
	 * Get all equipped items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	TArray<FEquippedItem> GetAllEquippedItems() const;

	/**
	 * Get equipment data from DataTable
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	bool GetEquipmentData(const FHarmoniaID& EquipmentId, FEquipmentData& OutData) const;

	/**
	 * Get total stat value from all equipped items
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	float GetTotalStatModifier(const FString& AttributeName) const;

	/**
	 * Unequip all items
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	void UnequipAll();

	// ============================================================================
	// Durability
	// ============================================================================

	/**
	 * Damage equipment durability
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	void DamageEquipment(EEquipmentSlot Slot, float DamageAmount);

	/**
	 * Repair equipment
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	void RepairEquipment(EEquipmentSlot Slot, float RepairAmount);

	/**
	 * Get equipment durability percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	float GetEquipmentDurabilityPercent(EEquipmentSlot Slot) const;

	// ============================================================================
	// Save/Load
	// ============================================================================

	/**
	 * Get save data for persistence
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	FEquipmentSaveData GetSaveData() const;

	/**
	 * Load from save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	void LoadFromSaveData(const FEquipmentSaveData& SaveData);

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Set equipment data table
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment")
	void SetEquipmentDataTable(UDataTable* InDataTable);

	/**
	 * Get equipment data table
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	UDataTable* GetEquipmentDataTable() const { return EquipmentDataTable; }

	// ============================================================================
	// Delegates
	// ============================================================================

	/** Called when equipment changes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Equipment")
	FOnEquipmentChanged OnEquipmentChanged;

	/** Called when equipment stats change */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Equipment")
	FOnEquipmentStatsChanged OnEquipmentStatsChanged;

protected:
	// ============================================================================
	// Equipment Data
	// ============================================================================

	/** Equipment slots */
	UPROPERTY(ReplicatedUsing = OnRep_EquippedItems, BlueprintReadOnly, Category = "Harmonia|Equipment")
	TArray<FEquippedItem> EquippedItems;

	/** Equipment data table */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Harmonia|Equipment")
	UDataTable* EquipmentDataTable;

	// ============================================================================
	// Internal Functions
	// ============================================================================

	/**
	 * Apply stat modifiers to attribute set
	 */
	void ApplyStatModifiers(const FEquipmentData& EquipmentData);

	/**
	 * Remove stat modifiers from attribute set
	 */
	void RemoveStatModifiers(const FEquipmentData& EquipmentData);

	/**
	 * Apply gameplay effects
	 */
	void ApplyGameplayEffects(const FEquipmentData& EquipmentData, FEquippedItem& OutEquippedItem);

	/**
	 * Remove gameplay effects
	 */
	void RemoveGameplayEffects(FEquippedItem& EquippedItem);

	/**
	 * Apply visual mesh
	 */
	void ApplyVisualMesh(const FEquipmentData& EquipmentData);

	/**
	 * Remove visual mesh
	 */
	void RemoveVisualMesh(EEquipmentSlot Slot);

	/**
	 * Get ability system component
	 */
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	/**
	 * Get attribute set
	 */
	UHarmoniaAttributeSet* GetAttributeSet() const;

	/**
	 * Get owner character mesh
	 */
	USkeletalMeshComponent* GetOwnerMesh() const;

	/**
	 * Replication callback
	 */
	UFUNCTION()
	void OnRep_EquippedItems();

	/**
	 * Server RPC for equipping
	 */
	UFUNCTION(Server, Reliable)
	void ServerEquipItem(const FHarmoniaID& EquipmentId, EEquipmentSlot Slot);

	/**
	 * Server RPC for unequipping
	 */
	UFUNCTION(Server, Reliable)
	void ServerUnequipItem(EEquipmentSlot Slot);

private:
	// Cached ability system component
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// Cached attribute set
	UPROPERTY()
	TWeakObjectPtr<UHarmoniaAttributeSet> CachedAttributeSet;

	// Spawned equipment meshes
	UPROPERTY()
	TArray<USkeletalMeshComponent*> EquipmentMeshes;
};
