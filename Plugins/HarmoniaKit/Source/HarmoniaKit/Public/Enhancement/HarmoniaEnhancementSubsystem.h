// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaEnhancementSystemDefinitions.h"
#include "HarmoniaEnhancementSubsystem.generated.h"

class UHarmoniaEnhancementConfigDataAsset;

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnhancementAttempted, APlayerController*, Player, const FHarmoniaEnhancementSessionResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnchantmentApplied, APlayerController*, Player, FGuid, ItemID, FName, EnchantmentID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnhancementItemDestroyed, APlayerController*, Player, FGuid, ItemID);

/**
 * Harmonia Enhancement Subsystem
 *
 * Manages equipment enhancement and enchantment.
 *
 * Features:
 * - Level-based enhancement with success rates
 * - Failure penalties (downgrade, destruction)
 * - Protection items
 * - Enchantment/socket system
 * - Pity system for guaranteed success
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaEnhancementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================================
	// Configuration
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	void SetConfigDataAsset(UHarmoniaEnhancementConfigDataAsset* InConfig);

	// ============================================================================
	// Enhancement Levels
	// ============================================================================

	/** Register enhancement level data */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	void RegisterEnhancementLevel(const FHarmoniaEnhancementLevelConfig& Level);

	/** Get enhancement level data */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	bool GetEnhancementLevel(int32 Level, FHarmoniaEnhancementLevelConfig& OutLevel) const;

	/** Get max enhancement level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	int32 GetMaxEnhancementLevel() const;

	// ============================================================================
	// Enchantments
	// ============================================================================

	/** Register an enchantment */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	void RegisterEnchantment(const FEnchantmentDefinition& Enchantment);

	/** Get enchantment */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	bool GetEnchantment(FName EnchantmentID, FEnchantmentDefinition& OutEnchantment) const;

	/** Get all enchantments */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	TArray<FEnchantmentDefinition> GetAllEnchantments() const;

	/** Get enchantments by slot */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	TArray<FEnchantmentDefinition> GetEnchantmentsBySlot(EEnchantSlot Slot) const;

	// ============================================================================
	// Item Enhancement State
	// ============================================================================

	/** Get item enhancement state */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	FEnhancedItemData GetItemState(FGuid ItemID) const;

	/** Set item enhancement state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	void SetItemState(FGuid ItemID, const FEnhancedItemData& State);

	/** Initialize new item state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	FEnhancedItemData InitializeItemState(int32 InitialSockets = 0);

	// ============================================================================
	// Enhancement Operations
	// ============================================================================

	/** Check if player can enhance item */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	bool CanEnhance(APlayerController* Player, FGuid ItemID, FText& OutReason) const;

	/** Attempt to enhance item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	FHarmoniaEnhancementSessionResult AttemptEnhance(APlayerController* Player, FGuid ItemID, bool bUseProtection = false);

	/** Get success rate for enhancement */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	float GetSuccessRate(FGuid ItemID, float BonusRate = 0.0f) const;

	/** Get required materials for next enhancement */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	TArray<FEnhancementMaterial> GetRequiredMaterials(FGuid ItemID) const;

	/** Get currency cost for next enhancement */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	int64 GetEnhancementCost(FGuid ItemID) const;

	// ============================================================================
	// Enchantment Operations
	// ============================================================================

	/** Apply enchantment to item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	bool ApplyEnchantment(APlayerController* Player, FGuid ItemID, FName EnchantmentID, EEnchantSlot Slot);

	/** Remove enchantment from item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	bool RemoveEnchantment(APlayerController* Player, FGuid ItemID, EEnchantSlot Slot);

	/** Get applied enchantments */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	TMap<EEnchantSlot, FName> GetAppliedEnchantments(FGuid ItemID) const;

	/** Check if enchantment is compatible */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	bool IsEnchantmentCompatible(FGuid ItemID, FName EnchantmentID, EEnchantSlot Slot) const;

	// ============================================================================
	// Socket Operations
	// ============================================================================

	/** Add socket to item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	bool AddSocket(APlayerController* Player, FGuid ItemID);

	/** Insert gem into socket */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	bool InsertGem(APlayerController* Player, FGuid ItemID, int32 SocketIndex, FGameplayTag GemTag);

	/** Remove gem from socket */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	bool RemoveGem(APlayerController* Player, FGuid ItemID, int32 SocketIndex);

	// ============================================================================
	// Protection
	// ============================================================================

	/** Apply protection to item */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Enhancement")
	bool ApplyProtection(APlayerController* Player, FGuid ItemID, EProtectionType ProtectionType);

	/** Get protection count */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	int32 GetProtectionCount(FGuid ItemID) const;

	// ============================================================================
	// Pity System
	// ============================================================================

	/** Get pity counter */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	int32 GetPityCounter(FGuid ItemID) const;

	/** Get pity threshold (failures needed for guaranteed success) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	int32 GetPityThreshold(int32 EnhancementLevel) const;

	/** Check if pity is active */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	bool IsPityActive(FGuid ItemID) const;

	// ============================================================================
	// Stat Calculation
	// ============================================================================

	/** Get stat multiplier for enhancement level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	float GetStatMultiplier(int32 EnhancementLevel) const;

	/** Get total stat bonuses from enchantments */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Enhancement")
	TMap<FGameplayTag, float> GetEnchantmentBonuses(FGuid ItemID) const;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Enhancement")
	FOnEnhancementAttempted OnEnhancementAttempted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Enhancement")
	FOnEnchantmentApplied OnEnchantmentApplied;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Enhancement")
	FOnEnhancementItemDestroyed OnItemDestroyed;

protected:
	/** Consume enhancement materials */
	bool ConsumeMaterials(APlayerController* Player, const TArray<FEnhancementMaterial>& Materials);

	/** Consume currency */
	bool ConsumeCurrency(APlayerController* Player, int64 Amount);

	/** Roll enhancement result */
	EEnhancementResult RollEnhancementResult(const FHarmoniaEnhancementLevelConfig& Level, bool bPityActive, bool bUseProtection);

private:
	/** Enhancement level data */
	UPROPERTY()
	TMap<int32, FHarmoniaEnhancementLevelConfig> EnhancementLevels;

	/** Registered enchantments */
	UPROPERTY()
	TMap<FName, FEnchantmentDefinition> Enchantments;

	/** Item enhancement states */
	TMap<FGuid, FEnhancedItemData> ItemStates;

	/** Config data asset */
	UPROPERTY()
	TObjectPtr<UHarmoniaEnhancementConfigDataAsset> ConfigAsset;
};
