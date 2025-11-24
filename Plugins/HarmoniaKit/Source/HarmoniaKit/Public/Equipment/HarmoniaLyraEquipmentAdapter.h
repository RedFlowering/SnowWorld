// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "HarmoniaLyraEquipmentAdapter.generated.h"

class ULyraEquipmentManagerComponent;
class UHarmoniaEquipmentComponent;
class ULyraInventoryManagerComponent;
class UHarmoniaInventoryComponent;

/**
 * Adapter class to bridge Harmonia Equipment System with Lyra Equipment System
 * Allows gradual migration from Harmonia to Lyra while maintaining compatibility
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaLyraEquipmentAdapter : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Convert Harmonia equipment slot to Lyra equipment system
	 * This is a helper for interoperability
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment|Adapter")
	static FString ConvertEquipmentSlotToString(EEquipmentSlot Slot);

	/**
	 * Sync Harmonia equipment to Lyra equipment manager
	 * Call this to keep both systems in sync during migration
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment|Adapter")
	static bool SyncHarmoniaToLyra(UHarmoniaEquipmentComponent* HarmoniaEquipment, ULyraEquipmentManagerComponent* LyraEquipment);

	/**
	 * Sync Lyra equipment to Harmonia equipment manager
	 * Call this to keep both systems in sync during migration
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment|Adapter")
	static bool SyncLyraToHarmonia(ULyraEquipmentManagerComponent* LyraEquipment, UHarmoniaEquipmentComponent* HarmoniaEquipment);

	/**
	 * Check if actor has both equipment systems
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment|Adapter")
	static bool HasBothEquipmentSystems(AActor* Actor);

	/**
	 * Get or create Lyra equipment manager on actor
	 * Helper for migration - creates Lyra component if it doesn't exist
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Equipment|Adapter")
	static ULyraEquipmentManagerComponent* GetOrCreateLyraEquipmentManager(AActor* Actor);
};
