// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaShopSystemDefinitions.h"
#include "HarmoniaShopSubsystem.generated.h"

class UHarmoniaShopConfigDataAsset;

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnItemPurchased, APlayerController*, Player, FName, ShopID, FGameplayTag, ItemTag, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnItemSold, APlayerController*, Player, FName, ShopID, FGameplayTag, ItemTag, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCompleted, const FTradeOffer&, Trade, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopOpened, FName, ShopID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopClosed, FName, ShopID);

/**
 * Harmonia Shop Subsystem
 *
 * Central management for shops, trading, and economic transactions.
 *
 * Features:
 * - NPC shop management
 * - Dynamic pricing
 * - Player-to-player trading
 * - Transaction history
 * - Stock and restock management
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================================
	// Configuration
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	void SetConfigDataAsset(UHarmoniaShopConfigDataAsset* InConfig);

	// ============================================================================
	// Shop Management
	// ============================================================================

	/** Register a shop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	void RegisterShop(const FShopDefinition& Shop);

	/** Unregister a shop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	void UnregisterShop(FName ShopID);

	/** Get shop by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	bool GetShop(FName ShopID, FShopDefinition& OutShop) const;

	/** Get all shops */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FShopDefinition> GetAllShops() const;

	/** Get shops by type */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FShopDefinition> GetShopsByType(EHarmoniaShopType Type) const;

	/** Open a shop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	void OpenShop(FName ShopID);

	/** Close a shop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	void CloseShop(FName ShopID);

	// ============================================================================
	// Shop Items
	// ============================================================================

	/** Get items in a shop */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FShopItemData> GetShopItems(FName ShopID) const;

	/** Get available items (that player can buy) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FShopItemData> GetAvailableItems(APlayerController* Player, FName ShopID) const;

	/** Check item availability */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	EHarmoniaShopItemAvailability GetItemAvailability(APlayerController* Player, FName ShopID, FName ShopItemID) const;

	/** Get final buy price (with modifiers) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FHarmoniaCurrencyCost> GetFinalBuyPrice(APlayerController* Player, FName ShopID, FName ShopItemID, int32 Quantity = 1) const;

	/** Get final sell price (with modifiers) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FHarmoniaCurrencyCost> GetFinalSellPrice(APlayerController* Player, FName ShopID, FGameplayTag ItemTag, int32 Quantity = 1) const;

	// ============================================================================
	// Purchase/Sell
	// ============================================================================

	/** Purchase an item from a shop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool PurchaseItem(APlayerController* Player, FName ShopID, FName ShopItemID, int32 Quantity = 1);

	/** Sell an item to a shop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool SellItem(APlayerController* Player, FName ShopID, FGameplayTag ItemTag, int32 Quantity = 1);

	/** Check if player can afford */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	bool CanAfford(APlayerController* Player, const TArray<FHarmoniaCurrencyCost>& Cost) const;

	// ============================================================================
	// Player Trading
	// ============================================================================

	/** Initiate a trade with another player */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	FGuid InitiateTrade(APlayerController* Initiator, APlayerController* Target);

	/** Add item to trade offer */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool AddItemToTrade(FGuid TradeID, APlayerController* Player, FGameplayTag ItemTag);

	/** Remove item from trade offer */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool RemoveItemFromTrade(FGuid TradeID, APlayerController* Player, FGameplayTag ItemTag);

	/** Add currency to trade offer */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool AddCurrencyToTrade(FGuid TradeID, APlayerController* Player, const FHarmoniaCurrencyCost& Currency);

	/** Confirm trade */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool ConfirmTrade(FGuid TradeID, APlayerController* Player);

	/** Cancel trade */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool CancelTrade(FGuid TradeID, APlayerController* Player);

	/** Get active trade */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	bool GetActiveTrade(FGuid TradeID, FTradeOffer& OutTrade) const;

	// ============================================================================
	// Currency Management
	// ============================================================================

	/** Get player's currency amount */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	int64 GetPlayerCurrency(APlayerController* Player, EHarmoniaCurrencyType Type) const;

	/** Add currency to player */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	void AddPlayerCurrency(APlayerController* Player, EHarmoniaCurrencyType Type, int64 Amount);

	/** Remove currency from player */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Shop")
	bool RemovePlayerCurrency(APlayerController* Player, EHarmoniaCurrencyType Type, int64 Amount);

	// ============================================================================
	// Transaction History
	// ============================================================================

	/** Get transaction history for player */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Shop")
	TArray<FTransactionRecord> GetTransactionHistory(APlayerController* Player, int32 MaxRecords = 100) const;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Shop")
	FOnItemPurchased OnItemPurchased;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Shop")
	FOnItemSold OnItemSold;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Shop")
	FOnTradeCompleted OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Shop")
	FOnShopOpened OnShopOpened;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Shop")
	FOnShopClosed OnShopClosed;

protected:
	/** Process restock for a shop */
	void ProcessRestock(FName ShopID);

	/** Execute trade */
	bool ExecuteTrade(FTradeOffer& Trade);

	/** Record transaction */
	void RecordTransaction(const FTransactionRecord& Record);

	/** Get player ID */
	uint32 GetPlayerID(APlayerController* Player) const;

private:
	/** Registered shops */
	UPROPERTY()
	TMap<FName, FShopDefinition> Shops;

	/** Active trades */
	TMap<FGuid, FTradeOffer> ActiveTrades;

	/** Transaction history (PlayerID -> Records) */
	TMap<uint32, TArray<FTransactionRecord>> TransactionHistory;

	/** Player currencies (PlayerID -> CurrencyType -> Amount) */
	TMap<uint32, TMap<EHarmoniaCurrencyType, int64>> PlayerCurrencies;

	/** Player purchase counts (PlayerID -> ShopItemID -> Count) */
	TMap<uint32, TMap<FName, int32>> PlayerPurchaseCounts;

	/** Config data asset */
	UPROPERTY()
	TObjectPtr<UHarmoniaShopConfigDataAsset> ConfigAsset;

	/** Restock timer handles */
	TMap<FName, FTimerHandle> RestockTimers;
};
