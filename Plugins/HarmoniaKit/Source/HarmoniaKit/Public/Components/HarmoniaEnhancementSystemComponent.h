// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaEnhancementSystemDefinitions.h"
#include "Engine/DataTable.h"
#include "HarmoniaEnhancementSystemComponent.generated.h"

class UHarmoniaInventoryComponent;
class UAbilitySystemComponent;

/**
 * Enhancement System Component
 * Manages all item enhancement features:
 * - Equipment enhancement (+1, +2, ... +15)
 * - Socket/Gem system
 * - Reforge system (stat rerolling)
 * - Transcendence system
 * - Transmog system (appearance override)
 * - Repair system (durability management)
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaEnhancementSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaEnhancementSystemComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ============================================================================
	// Initialization
	// ============================================================================

	/** Initialize with inventory component */
	UFUNCTION(BlueprintCallable, Category = "Enhancement")
	void Initialize(UHarmoniaInventoryComponent* InInventoryComponent);

	// ============================================================================
	// Enhancement System (+1, +2, ...)
	// ============================================================================

	/** Get enhancement data for item */
	UFUNCTION(BlueprintPure, Category = "Enhancement")
	bool GetEnhancementData(FGuid ItemGUID, FEnhancedItemData& OutData) const;

	/** Can enhance item to next level? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement")
	bool CanEnhanceItem(FGuid ItemGUID, int32 TargetLevel, FString& OutReason) const;

	/** Enhance item */
	UFUNCTION(BlueprintCallable, Category = "Enhancement", meta = (AutoCreateRefTerm = "bUseProtection"))
	bool EnhanceItem(FGuid ItemGUID, int32 TargetLevel, bool bUseProtection = false);

	/** Get enhancement level config */
	UFUNCTION(BlueprintPure, Category = "Enhancement")
	bool GetEnhancementLevelConfig(int32 Level, FEnhancementLevelConfig& OutConfig) const;

	/** Get total stat multiplier for item (base * enhancement * transcendence) */
	UFUNCTION(BlueprintPure, Category = "Enhancement")
	float GetTotalStatMultiplier(const FEnhancedItemData& ItemData) const;

	/** Get all stat modifiers for enhanced item */
	UFUNCTION(BlueprintCallable, Category = "Enhancement")
	void GetEnhancedStatModifiers(const FEnhancedItemData& ItemData, TArray<FEquipmentStatModifier>& OutModifiers) const;

	// ============================================================================
	// Socket/Gem System
	// ============================================================================

	/** Add socket to item */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Sockets")
	bool AddSocket(FGuid ItemGUID, EGemSocketType SocketType, bool bUnlocked = true);

	/** Unlock socket */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Sockets")
	bool UnlockSocket(FGuid ItemGUID, int32 SocketIndex);

	/** Can insert gem into socket? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Sockets")
	bool CanInsertGem(FGuid ItemGUID, int32 SocketIndex, FHarmoniaID GemId, FString& OutReason) const;

	/** Insert gem into socket */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Sockets")
	bool InsertGem(FGuid ItemGUID, int32 SocketIndex, FHarmoniaID GemId);

	/** Remove gem from socket */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Sockets")
	bool RemoveGem(FGuid ItemGUID, int32 SocketIndex, bool bDestroyGem = false);

	/** Get gem data */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Sockets")
	bool GetGemData(FHarmoniaID GemId, FGemData& OutData) const;

	/** Get all gems in item */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Sockets")
	void GetInsertedGems(FGuid ItemGUID, TArray<FGemData>& OutGems) const;

	// ============================================================================
	// Reforge System (Stat Rerolling)
	// ============================================================================

	/** Can reforge item? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Reforge")
	bool CanReforgeItem(FGuid ItemGUID, FString& OutReason) const;

	/** Reforge item (reroll stats) */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Reforge")
	bool ReforgeItem(FGuid ItemGUID, bool bLockCurrentStats = false);

	/** Get reforge config for item grade */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Reforge")
	bool GetReforgeConfig(EItemGrade ItemGrade, FReforgeConfig& OutConfig) const;

	/** Lock reforge stat (prevent reroll on this stat) */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Reforge")
	bool LockReforgeStat(FGuid ItemGUID, int32 StatIndex, bool bLocked = true);

	// ============================================================================
	// Transcendence System (Tier Upgrade)
	// ============================================================================

	/** Can transcend item? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Transcendence")
	bool CanTranscendItem(FGuid ItemGUID, ETranscendenceTier TargetTier, FString& OutReason) const;

	/** Transcend item to next tier */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Transcendence")
	bool TranscendItem(FGuid ItemGUID, ETranscendenceTier TargetTier);

	/** Get transcendence config */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Transcendence")
	bool GetTranscendenceConfig(ETranscendenceTier Tier, FTranscendenceConfig& OutConfig) const;

	// ============================================================================
	// Transmog System (Appearance Override)
	// ============================================================================

	/** Can apply transmog? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Transmog")
	bool CanApplyTransmog(FGuid TargetItemGUID, FHarmoniaID AppearanceItemId, FString& OutReason) const;

	/** Apply transmog (override appearance) */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Transmog")
	bool ApplyTransmog(FGuid TargetItemGUID, FHarmoniaID AppearanceItemId);

	/** Remove transmog */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Transmog")
	bool RemoveTransmog(FGuid ItemGUID);

	/** Get transmog appearance data */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Transmog")
	bool GetTransmogData(FGuid ItemGUID, FTransmogData& OutData) const;

	// ============================================================================
	// Repair System (Durability Management)
	// ============================================================================

	/** Damage item durability */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	void DamageItemDurability(FGuid ItemGUID, float DamageAmount);

	/** Can repair item? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	bool CanRepairItem(FGuid ItemGUID, FString& OutReason) const;

	/** Repair item at repair station */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	bool RepairItem(FGuid ItemGUID, bool bFullRepair = true, float RepairAmount = 0.0f);

	/** Repair item using repair kit */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	bool RepairItemWithKit(FGuid ItemGUID, FHarmoniaID RepairKitId);

	/** Repair item at specific repair station */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	bool RepairItemAtStation(FGuid ItemGUID, AActor* RepairStation, bool bFullRepair = true);

	/** Get repair cost */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Repair")
	int32 GetRepairCost(FGuid ItemGUID, bool bFullRepair = true, float RepairAmount = 0.0f) const;

	/** Get repair cost at specific station */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	int32 GetRepairCostAtStation(FGuid ItemGUID, AActor* RepairStation, bool bFullRepair = true) const;

	/** Get repair config */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Repair")
	bool GetRepairConfig(EItemGrade ItemGrade, FRepairConfig& OutConfig) const;

	/** Get repair kit data */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Repair")
	bool GetRepairKitData(FHarmoniaID RepairKitId, FRepairKitData& OutData) const;

	/** Can use repair kit on item? */
	UFUNCTION(BlueprintCallable, Category = "Enhancement|Repair")
	bool CanUseRepairKit(FGuid ItemGUID, FHarmoniaID RepairKitId, FString& OutReason) const;

	/** Get durability penalty multiplier (0.0 - 1.0) based on current durability */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Repair")
	float GetDurabilityPenaltyMultiplier(FGuid ItemGUID) const;

	/** Check if item is damaged enough to show warnings */
	UFUNCTION(BlueprintPure, Category = "Enhancement|Repair")
	bool ShouldShowDurabilityWarning(FGuid ItemGUID, float WarningThreshold = 0.25f) const;

	// ============================================================================
	// Item Management
	// ============================================================================

	/** Create enhanced item instance */
	UFUNCTION(BlueprintCallable, Category = "Enhancement")
	FGuid CreateEnhancedItem(FHarmoniaID ItemId, int32 InitialEnhancementLevel = 0);

	/** Destroy enhanced item */
	UFUNCTION(BlueprintCallable, Category = "Enhancement")
	void DestroyEnhancedItem(FGuid ItemGUID, bool bWasEnhancementFailure = false);

	/** Check if item exists */
	UFUNCTION(BlueprintPure, Category = "Enhancement")
	bool ItemExists(FGuid ItemGUID) const;

	/** Get item base data */
	bool GetItemBaseData(FHarmoniaID ItemId, FEquipmentData& OutData) const;

	// ============================================================================
	// Events
	// ============================================================================

	/** Enhancement started */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnEnhancementStarted OnEnhancementStarted;

	/** Enhancement completed */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnEnhancementCompleted OnEnhancementCompleted;

	/** Gem inserted */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnGemInserted OnGemInserted;

	/** Gem removed */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnGemRemoved OnGemRemoved;

	/** Item reforged */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnItemReforged OnItemReforged;

	/** Item transcended */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnItemTranscended OnItemTranscended;

	/** Transmog applied */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnTransmogApplied OnTransmogApplied;

	/** Transmog removed */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnTransmogRemoved OnTransmogRemoved;

	/** Item repaired */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnItemRepaired OnItemRepaired;

	/** Item destroyed */
	UPROPERTY(BlueprintAssignable, Category = "Enhancement|Events")
	FOnItemDestroyed OnItemDestroyed;

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/** Enhancement level config data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> EnhancementLevelConfigTable = nullptr;

	/** Gem data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> GemDataTable = nullptr;

	/** Reforge config data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> ReforgeConfigTable = nullptr;

	/** Transcendence config data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> TranscendenceConfigTable = nullptr;

	/** Repair config data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> RepairConfigTable = nullptr;

	/** Repair kit data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> RepairKitDataTable = nullptr;

	/** Equipment data table (base item data) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	TObjectPtr<UDataTable> EquipmentDataTable = nullptr;

	/** Maximum enhancement level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	int32 MaxEnhancementLevel = 15;

	/** Use protection stone to prevent item destruction? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	bool bAllowProtectionStones = true;

	/** Protection stone item ID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config", meta = (EditCondition = "bAllowProtectionStones"))
	FHarmoniaID ProtectionStoneItemId;

	/** Transmog cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	int32 TransmogCost = 1000;

	/** Durability loss on death */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config")
	float DurabilityLossOnDeath = 10.0f;

	/** Enable durability-based performance penalty? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config|Durability")
	bool bEnableDurabilityPenalty = true;

	/** Durability threshold below which penalty starts (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config|Durability", meta = (EditCondition = "bEnableDurabilityPenalty", ClampMin = "0.0", ClampMax = "1.0"))
	float DurabilityPenaltyThreshold = 0.5f; // 50%

	/** Maximum stat penalty at 0 durability (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config|Durability", meta = (EditCondition = "bEnableDurabilityPenalty", ClampMin = "0.0", ClampMax = "1.0"))
	float MaxDurabilityPenalty = 0.5f; // 50% stat reduction at 0 durability

	/** Auto repair enabled? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config|Auto Repair")
	bool bAutoRepairEnabled = false;

	/** Auto repair threshold (repair when durability drops below this %) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config|Auto Repair", meta = (EditCondition = "bAutoRepairEnabled", ClampMin = "0.0", ClampMax = "1.0"))
	float AutoRepairThreshold = 0.25f; // 25%

	/** Auto repair uses repair kits first? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhancement|Config|Auto Repair", meta = (EditCondition = "bAutoRepairEnabled"))
	bool bAutoRepairPreferKits = true;

	// ============================================================================
	// State
	// ============================================================================

	/** Enhanced items registry */
	UPROPERTY(Replicated, SaveGame)
	TArray<FEnhancedItemData> EnhancedItems;

	/** Current enhancement session (if active) */
	UPROPERTY()
	FEnhancementSession CurrentSession;

	/** Inventory component reference */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent = nullptr;

	/** Ability system component reference (for applying effects) */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	// ============================================================================
	// Internal Methods
	// ============================================================================

	/** Roll enhancement result */
	EEnhancementResult RollEnhancementResult(const FEnhancementLevelConfig& Config, bool bUseProtection) const;

	/** Apply enhancement to item */
	void ApplyEnhancement(FEnhancedItemData& ItemData, int32 NewLevel);

	/** Roll reforge stats */
	TArray<FReforgeStatEntry> RollReforgeStats(const FReforgeConfig& Config) const;

	/** Consume materials from inventory */
	bool ConsumeMaterials(const TMap<FHarmoniaID, int32>& Materials);

	/** Consume currency from player */
	bool ConsumeCurrency(int32 Amount);

	/** Has materials in inventory? */
	bool HasMaterials(const TMap<FHarmoniaID, int32>& Materials) const;

	/** Has currency? */
	bool HasCurrency(int32 Amount) const;

	/** Apply stat modifiers to ability system */
	void ApplyStatModifiers(const TArray<FEquipmentStatModifier>& Modifiers);

	/** Remove stat modifiers from ability system */
	void RemoveStatModifiers(const TArray<FEquipmentStatModifier>& Modifiers);

	// ============================================================================
	// Replication
	// ============================================================================

	/** Called when enhanced items change */
	UFUNCTION()
	void OnRep_EnhancedItems();

	// ============================================================================
	// Server RPCs
	// ============================================================================

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEnhanceItem(FGuid ItemGUID, int32 TargetLevel, bool bUseProtection);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInsertGem(FGuid ItemGUID, int32 SocketIndex, FHarmoniaID GemId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveGem(FGuid ItemGUID, int32 SocketIndex, bool bDestroyGem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReforgeItem(FGuid ItemGUID);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTranscendItem(FGuid ItemGUID, ETranscendenceTier TargetTier);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerApplyTransmog(FGuid TargetItemGUID, FHarmoniaID AppearanceItemId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveTransmog(FGuid ItemGUID);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRepairItem(FGuid ItemGUID, bool bFullRepair, float RepairAmount);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRepairItemWithKit(FGuid ItemGUID, FHarmoniaID RepairKitId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRepairItemAtStation(FGuid ItemGUID, AActor* RepairStation, bool bFullRepair);

	// ============================================================================
	// Helper Methods
	// ============================================================================

	/** Try auto repair for item */
	void TryAutoRepair(FGuid ItemGUID);

	/** Apply durability penalty to stat modifiers */
	void ApplyDurabilityPenalty(FGuid ItemGUID, TArray<FEquipmentStatModifier>& InOutModifiers) const;

	/** Find enhanced item by GUID */
	FEnhancedItemData* FindEnhancedItem(FGuid ItemGUID);

	/** Find enhanced item by GUID (const) */
	const FEnhancedItemData* FindEnhancedItem(FGuid ItemGUID) const;
};
