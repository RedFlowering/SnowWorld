// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaRechargeableItemComponent.h
 * @brief Rechargeable recovery item management component
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaRecoveryItemDefinitions.h"
#include "HarmoniaRechargeableItemComponent.generated.h"

class APlayerController;
class UHarmoniaCheckpointSubsystem;

/**
 * Rechargeable Item Events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemUsed, EHarmoniaRecoveryItemType, ItemType, int32, RemainingCharges, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemRecharged, EHarmoniaRecoveryItemType, ItemType, int32, CurrentCharges, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChargesChanged, EHarmoniaRecoveryItemType, ItemType, int32, NewCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUpgraded, EHarmoniaRecoveryItemType, ItemType, int32, NewMaxCharges);

/**
 * @class UHarmoniaRechargeableItemComponent
 * @brief Component for managing rechargeable recovery items
 * 
 * Generic component usable for all recovery item types.
 *
 * Key Features:
 * - Item charge count management
 * - Auto recharge at checkpoints
 * - Item usage validation
 * - Item upgrade (increase max charges)
 * - Network replication
 * - Save/Load system integration
 *
 * Extensibility:
 * - Can be used for any rechargeable item beyond recovery items
 * - Can be extended for dungeon entry tokens, revive tokens, etc.
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaRechargeableItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaRechargeableItemComponent();

	// UActorComponent interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// End of UActorComponent interface

	// ============================================================================
	// Item Registration
	// ============================================================================

	/**
	 * Register recovery item (initialization)
	 * @param ItemType Item type
	 * @param Config Item configuration
	 * @return Registration success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterRecoveryItem(EHarmoniaRecoveryItemType ItemType, const FHarmoniaRecoveryItemConfig& Config);

	/**
	 * Register resonance shard (per frequency)
	 * @param Frequency Resonance frequency
	 * @param VariantConfig Shard variant configuration
	 * @return Registration success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool RegisterResonanceShard(EHarmoniaResonanceFrequency Frequency, const FHarmoniaResonanceShardVariant& VariantConfig);

	/**
	 * Unregister item
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void UnregisterRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	// ============================================================================
	// Item Usage
	// ============================================================================

	/**
	 * Use recovery item (consumes 1 charge)
	 * @param ItemType Item type to use
	 * @return Usage success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseRecoveryItem(EHarmoniaRecoveryItemType ItemType);

	/**
	 * Use resonance shard (per frequency)
	 * @param Frequency Resonance frequency of shard to use
	 * @return Usage success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UseResonanceShard(EHarmoniaResonanceFrequency Frequency);

	/**
	 * Check if item can be used
	 * @param ItemType Item type to check
	 * @param OutReason Reason for unavailability (on failure)
	 * @return Whether item can be used
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool CanUseRecoveryItem(EHarmoniaRecoveryItemType ItemType, FText& OutReason) const;

	/**
	 * Get remaining charges
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetRemainingCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * Get maximum charges
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	int32 GetMaxCharges(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * Get item configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetRecoveryItemConfig(EHarmoniaRecoveryItemType ItemType, FHarmoniaRecoveryItemConfig& OutConfig) const;

	/**
	 * Get resonance shard variant configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	bool GetResonanceShardVariant(EHarmoniaResonanceFrequency Frequency, FHarmoniaResonanceShardVariant& OutVariant) const;

	// ============================================================================
	// Item Recharge
	// ============================================================================

	/**
	 * Recharge item
	 * @param ItemType Item type to recharge
	 * @param Amount Amount to recharge (-1 for full recharge)
	 * @return Number of charges added
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeItem(EHarmoniaRecoveryItemType ItemType, int32 Amount = -1);

	/**
	 * Recharge all items (resonance at checkpoint, etc.)
	 * @return Total number recharged
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeAllItems();

	/**
	 * Recharge only rechargeable items
	 * @return Total number recharged
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	int32 RechargeRechargeableItems();

	// ============================================================================
	// Item Upgrade
	// ============================================================================

	/**
	 * Upgrade item (increase max charges)
	 * @param ItemType Item type to upgrade
	 * @param AdditionalMaxCharges Additional max charges
	 * @return Upgrade success status
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	bool UpgradeItemMaxCharges(EHarmoniaRecoveryItemType ItemType, int32 AdditionalMaxCharges);

	// ============================================================================
	// Checkpoint Integration
	// ============================================================================

	/**
	 * Handle checkpoint resonance event
	 */
	UFUNCTION()
	void OnCheckpointResonanceCompleted(FName CheckpointID, const FHarmoniaResonanceResult& Result);

	// ============================================================================
	// Data Persistence
	// ============================================================================

	/**
	 * Get item states for save data
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Recovery Item")
	TArray<FHarmoniaRecoveryItemState> GetItemStatesForSave() const;

	/**
	 * Load item states
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Recovery Item")
	void LoadItemStates(const TArray<FHarmoniaRecoveryItemState>& LoadedStates);

	// ============================================================================
	// Events
	// ============================================================================

	/** Item used event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUsed OnItemUsed;

	/** Item recharged event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemRecharged OnItemRecharged;

	/** Charges changed event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnChargesChanged OnChargesChanged;

	/** Item upgraded event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Recovery Item|Events")
	FOnItemUpgraded OnItemUpgraded;

protected:
	/**
	 * Check if item is registered
	 */
	bool IsItemRegistered(EHarmoniaRecoveryItemType ItemType) const;

	/**
	 * Set charges (internal, includes replication)
	 */
	void SetCharges(EHarmoniaRecoveryItemType ItemType, int32 NewCharges);

	/**
	 * Initialize checkpoint subsystem integration
	 */
	void InitializeCheckpointIntegration();

private:
	/** Registered recovery item configurations */
	UPROPERTY(Transient)
	TMap<EHarmoniaRecoveryItemType, FHarmoniaRecoveryItemConfig> RegisteredItems;

	/** Registered resonance shard variants (per frequency) */
	UPROPERTY(Transient)
	TMap<EHarmoniaResonanceFrequency, FHarmoniaResonanceShardVariant> ResonanceShardVariants;

	/** Current item states (network replicated) */
	UPROPERTY(ReplicatedUsing=OnRep_ItemStates)
	TArray<FHarmoniaRecoveryItemState> ItemStates;

	UFUNCTION()
	void OnRep_ItemStates();

	/** Cached checkpoint subsystem */
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaCheckpointSubsystem> CheckpointSubsystem;
};
