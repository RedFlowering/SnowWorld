// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaItemSystemDefinitions.h"
#include "HarmoniaShopSystemDefinitions.generated.h"

/**
 * Currency type - unified currency enum for all systems (Shop, Death Penalty, etc.)
 */
UENUM(BlueprintType)
enum class EHarmoniaCurrencyType : uint8
{
	None			UMETA(DisplayName = "None"),			// ?ÜÏùå
	
	// Economy currencies
	Gold			UMETA(DisplayName = "Gold"),			// Í∏∞Î≥∏ Í≥®Îìú
	Premium			UMETA(DisplayName = "Premium"),			// ?ÑÎ¶¨ÎØ∏ÏóÑ ?¨Ìôî (Ï∫êÏãú)
	Honor			UMETA(DisplayName = "Honor"),			// Î™ÖÏòà ?¨Ïù∏??
	Arena			UMETA(DisplayName = "Arena"),			// ?ÑÎ†à???¨Ïù∏??
	Guild			UMETA(DisplayName = "Guild"),			// Í∏∏Îìú ?¨Ïù∏??
	Event			UMETA(DisplayName = "Event"),			// ?¥Î≤§???¨Ìôî
	Reputation		UMETA(DisplayName = "Reputation"),		// ?âÌåê ?¨Ïù∏??
	
	// Death Penalty currencies
	MemoryEssence		UMETA(DisplayName = "Memory Essence"),	// Í∏∞Ïñµ???ïÏàò
	SoulCrystals		UMETA(DisplayName = "Soul Crystals"),	// ?ÅÌòº ?òÏ†ï
	ForgottenKnowledge	UMETA(DisplayName = "Forgotten Knowledge"),	// ?äÌ?Ïß?ÏßÄ??
	TimeFragments		UMETA(DisplayName = "Time Fragments"),	// ?úÍ∞Ñ ?åÌé∏
	
	Custom			UMETA(DisplayName = "Custom"),			// Ïª§Ïä§?Ä (?úÍ∑∏Î°?ÏßÄ??
	MAX				UMETA(Hidden)
};

// Legacy alias for backward compatibility
using ECurrencyType = EHarmoniaCurrencyType;

/**
 * Shop type
 */
UENUM(BlueprintType)
enum class EHarmoniaShopType : uint8
{
	General			UMETA(DisplayName = "General"),			// ?°Ìôî??
	Weapon			UMETA(DisplayName = "Weapon"),			// Î¨¥Í∏∞?ÅÏ†ê
	Armor			UMETA(DisplayName = "Armor"),			// Î∞©Ïñ¥Íµ¨ÏÉÅ??
	Consumable		UMETA(DisplayName = "Consumable"),		// ?åÎπÑ??
	Material		UMETA(DisplayName = "Material"),		// ?¨Î£å?ÅÏ†ê
	Premium			UMETA(DisplayName = "Premium"),			// ?ÑÎ¶¨ÎØ∏ÏóÑ ??
	Guild			UMETA(DisplayName = "Guild"),			// Í∏∏Îìú ?ÅÏ†ê
	Event			UMETA(DisplayName = "Event"),			// ?¥Î≤§???ÅÏ†ê
	Auction			UMETA(DisplayName = "Auction"),			// Í≤ΩÎß§??
	MAX				UMETA(Hidden)
};

/**
 * Shop item availability
 */
UENUM(BlueprintType)
enum class EHarmoniaShopItemAvailability : uint8
{
	Available			UMETA(DisplayName = "Available"),
	SoldOut				UMETA(DisplayName = "Sold Out"),
	LevelLocked			UMETA(DisplayName = "Level Locked"),
	ReputationLocked	UMETA(DisplayName = "Reputation Locked"),
	QuestLocked			UMETA(DisplayName = "Quest Locked"),
	TimeLocked			UMETA(DisplayName = "Time Locked"),
	PurchaseLimitReached	UMETA(DisplayName = "Purchase Limit Reached"),
	NotUnlocked			UMETA(DisplayName = "Not Unlocked"),
	NotEnoughCurrency	UMETA(DisplayName = "Not Enough Currency"),
	MAX					UMETA(Hidden)
};

/**
 * Transaction type
 */
UENUM(BlueprintType)
enum class ETransactionType : uint8
{
	Buy				UMETA(DisplayName = "Buy"),				// Íµ¨Îß§
	Sell			UMETA(DisplayName = "Sell"),			// ?êÎß§
	Trade			UMETA(DisplayName = "Trade"),			// ÍµêÌôò
	Refund			UMETA(DisplayName = "Refund"),			// ?òÎ∂à
	MAX				UMETA(Hidden)
};

/**
 * Transaction result
 */
UENUM(BlueprintType)
enum class ETransactionResult : uint8
{
	Success			UMETA(DisplayName = "Success"),
	InsufficientFunds	UMETA(DisplayName = "Insufficient Funds"),
	InsufficientStock	UMETA(DisplayName = "Insufficient Stock"),
	InventoryFull	UMETA(DisplayName = "Inventory Full"),
	ItemNotFound	UMETA(DisplayName = "Item Not Found"),
	ShopNotFound	UMETA(DisplayName = "Shop Not Found"),
	Cancelled		UMETA(DisplayName = "Cancelled"),
	Failed			UMETA(DisplayName = "Failed"),
	MAX				UMETA(Hidden)
};

/**
 * Currency cost structure
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaCurrencyCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	EHarmoniaCurrencyType CurrencyType = EHarmoniaCurrencyType::Gold;

	// Alias for compatibility - returns CurrencyType
	EHarmoniaCurrencyType GetType() const { return CurrencyType; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	int64 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
	FGameplayTag CustomCurrencyTag;

	FHarmoniaCurrencyCost() = default;
	FHarmoniaCurrencyCost(EHarmoniaCurrencyType InType, int64 InAmount)
		: CurrencyType(InType), Amount(InAmount) {}
};

/**
 * Currency definition
 */
USTRUCT(BlueprintType)
struct FHarmoniaCurrencyData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	EHarmoniaCurrencyType Type = EHarmoniaCurrencyType::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	TSoftObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	int64 MaxAmount = 999999999;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	FGameplayTag CurrencyTag;
};

/**
 * Shop item data
 */
USTRUCT(BlueprintType)
struct FHarmoniaShopItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName ShopItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FHarmoniaID ItemId = FHarmoniaID();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FGameplayTag ItemTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pricing")
	EHarmoniaCurrencyType Currency = EHarmoniaCurrencyType::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pricing")
	int64 BasePrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pricing")
	float BuyPriceModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pricing")
	float SellPriceModifier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pricing")
	TArray<FHarmoniaCurrencyCost> BuyPrice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pricing")
	TArray<FHarmoniaCurrencyCost> SellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stock")
	int32 Stock = -1;  // -1 = unlimited

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stock")
	int32 MaxStock = -1;  // -1 = unlimited, used for restock

	UPROPERTY(BlueprintReadWrite, Category = "Stock")
	int32 CurrentStock = -1;  // Runtime current stock

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stock")
	bool bRestockable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stock")
	float RestockIntervalSeconds = 3600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stock")
	float RestockTime = 3600.0f;  // Alias for RestockIntervalSeconds

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 RequiredPlayerLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 PurchaseLimitPerPlayer = -1;  // -1 = unlimited

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sale")
	bool bIsOnSale = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sale", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SaleDiscount = 0.0f;
};

/**
 * Shop definition
 */
USTRUCT(BlueprintType)
struct FHarmoniaShopData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FName ShopID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	EHarmoniaShopType ShopType = EHarmoniaShopType::General;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FText Description = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TSoftObjectPtr<UTexture2D> ShopIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	FGameplayTagContainer ShopTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Modifiers")
	float GlobalBuyModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Modifiers")
	float GlobalSellModifier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Modifiers")
	float BuyPriceModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Modifiers")
	float SellPriceModifier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Items")
	TArray<FHarmoniaShopItemData> Items;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Requirements")
	int32 RequiredPlayerLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|Requirements")
	FGameplayTagContainer RequiredTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|State")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop|State")
	bool bCanSellItems = true;
};

/**
 * Trade offer item
 */
USTRUCT(BlueprintType)
struct FTradeOfferItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	FHarmoniaID ItemId = FHarmoniaID();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	EHarmoniaCurrencyType Currency = EHarmoniaCurrencyType::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	int64 CurrencyAmount = 0;
};

/**
 * Player-to-player trade offer
 */
USTRUCT(BlueprintType)
struct FTradeOffer
{
	GENERATED_BODY()

	FTradeOffer()
	{
		TradeID = FGuid::NewGuid();
	}

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	FGuid TradeID;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	FGuid InitiatorPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	FGuid TargetPlayerId;

	// Player controller weak references for runtime use
	TWeakObjectPtr<APlayerController> Initiator;
	TWeakObjectPtr<APlayerController> Target;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	TArray<FTradeOfferItem> InitiatorItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trade")
	TArray<FTradeOfferItem> TargetItems;

	// Currency arrays for trade
	TArray<FHarmoniaCurrencyCost> InitiatorCurrency;
	TArray<FHarmoniaCurrencyCost> TargetCurrency;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	bool bInitiatorConfirmed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	bool bTargetConfirmed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Trade")
	float ExpirationTime = 0.0f;
};

/**
 * Transaction record
 */
USTRUCT(BlueprintType)
struct FTransactionRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	FGuid TransactionId;

	// Player unique ID (runtime only, not exposed to blueprint)
	int32 PlayerID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	FName ShopID;  // Shop identifier

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	FGameplayTag ItemTag;  // Item tag for transaction

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	ETransactionType Type = ETransactionType::Buy;

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	ETransactionResult Result = ETransactionResult::Success;

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	FHarmoniaID ItemId = FHarmoniaID();

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	EHarmoniaCurrencyType CurrencyType = EHarmoniaCurrencyType::Gold;

	// Multi-currency support (runtime only)
	TArray<FHarmoniaCurrencyCost> Currency;

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	int64 Amount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	bool bWasPurchase = true;  // true = buy, false = sell

	UPROPERTY(BlueprintReadOnly, Category = "Transaction")
	FDateTime Timestamp;
};

// ============================================================================
// Type aliases for consistent naming with subsystems
// ============================================================================

using FHarmoniaShopItem = FHarmoniaShopItemData;
using FHarmoniaShopDefinition = FHarmoniaShopData;
using FHarmoniaTradeOffer = FTradeOffer;
using FHarmoniaTransactionRecord = FTransactionRecord;
