// Copyright 2025 Snow Game Studio.

#include "Economy/HarmoniaShopSubsystem.h"
#include "Economy/HarmoniaShopConfigDataAsset.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UHarmoniaShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaShopSubsystem::Deinitialize()
{
	// Clear all restock timers
	if (UWorld* World = GetWorld())
	{
		for (auto& Pair : RestockTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	RestockTimers.Empty();

	Super::Deinitialize();
}

void UHarmoniaShopSubsystem::SetConfigDataAsset(UHarmoniaShopConfigDataAsset* InConfig)
{
	ConfigAsset = InConfig;

	if (ConfigAsset)
	{
		for (const FShopDefinition& Shop : ConfigAsset->DefaultShops)
		{
			RegisterShop(Shop);
		}
	}
}

void UHarmoniaShopSubsystem::RegisterShop(const FShopDefinition& Shop)
{
	if (Shop.ShopID.IsNone())
	{
		return;
	}

	Shops.Add(Shop.ShopID, Shop);
}

void UHarmoniaShopSubsystem::UnregisterShop(FName ShopID)
{
	Shops.Remove(ShopID);

	// Clear restock timer
	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* Timer = RestockTimers.Find(ShopID))
		{
			World->GetTimerManager().ClearTimer(*Timer);
			RestockTimers.Remove(ShopID);
		}
	}
}

bool UHarmoniaShopSubsystem::GetShop(FName ShopID, FShopDefinition& OutShop) const
{
	if (const FShopDefinition* Found = Shops.Find(ShopID))
	{
		OutShop = *Found;
		return true;
	}
	return false;
}

TArray<FShopDefinition> UHarmoniaShopSubsystem::GetAllShops() const
{
	TArray<FShopDefinition> Result;
	Shops.GenerateValueArray(Result);
	return Result;
}

TArray<FShopDefinition> UHarmoniaShopSubsystem::GetShopsByType(EHarmoniaShopType Type) const
{
	TArray<FShopDefinition> Result;
	for (const auto& Pair : Shops)
	{
		if (Pair.Value.ShopType == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

void UHarmoniaShopSubsystem::OpenShop(FName ShopID)
{
	if (FShopDefinition* Shop = Shops.Find(ShopID))
	{
		Shop->bIsOpen = true;
		OnShopOpened.Broadcast(ShopID);
	}
}

void UHarmoniaShopSubsystem::CloseShop(FName ShopID)
{
	if (FShopDefinition* Shop = Shops.Find(ShopID))
	{
		Shop->bIsOpen = false;
		OnShopClosed.Broadcast(ShopID);
	}
}

TArray<FShopItemData> UHarmoniaShopSubsystem::GetShopItems(FName ShopID) const
{
	if (const FShopDefinition* Shop = Shops.Find(ShopID))
	{
		return Shop->Items;
	}
	return TArray<FShopItemData>();
}

TArray<FShopItemData> UHarmoniaShopSubsystem::GetAvailableItems(APlayerController* Player, FName ShopID) const
{
	TArray<FShopItemData> Result;

	const FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop || !Player)
	{
		return Result;
	}

	for (const FShopItemData& Item : Shop->Items)
	{
		EHarmoniaShopItemAvailability Availability = GetItemAvailability(Player, ShopID, Item.ShopItemID);
		if (Availability == EHarmoniaShopItemAvailability::Available ||
			Availability == EHarmoniaShopItemAvailability::NotEnoughCurrency)
		{
			Result.Add(Item);
		}
	}

	return Result;
}

EHarmoniaShopItemAvailability UHarmoniaShopSubsystem::GetItemAvailability(APlayerController* Player, FName ShopID, FName ShopItemID) const
{
	const FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop || !Player)
	{
		return EHarmoniaShopItemAvailability::NotUnlocked;
	}

	const FShopItemData* Item = nullptr;
	for (const FShopItemData& ShopItem : Shop->Items)
	{
		if (ShopItem.ShopItemID == ShopItemID)
		{
			Item = &ShopItem;
			break;
		}
	}

	if (!Item)
	{
		return EHarmoniaShopItemAvailability::NotUnlocked;
	}

	// Check stock
	if (Item->MaxStock >= 0 && Item->CurrentStock <= 0)
	{
		return EHarmoniaShopItemAvailability::SoldOut;
	}

	// Check purchase limit
	uint32 PlayerID = GetPlayerID(Player);
	if (Item->PurchaseLimitPerPlayer >= 0)
	{
		if (const TMap<FName, int32>* Counts = PlayerPurchaseCounts.Find(PlayerID))
		{
			if (const int32* Count = Counts->Find(ShopItemID))
			{
				if (*Count >= Item->PurchaseLimitPerPlayer)
				{
					return EHarmoniaShopItemAvailability::SoldOut;
				}
			}
		}
	}

	// TODO: Check level requirement
	// TODO: Check reputation requirement
	// TODO: Check quest requirement

	// Check currency
	TArray<FHarmoniaCurrencyCost> FinalPrice = GetFinalBuyPrice(Player, ShopID, ShopItemID, 1);
	if (!CanAfford(Player, FinalPrice))
	{
		return EHarmoniaShopItemAvailability::NotEnoughCurrency;
	}

	return EHarmoniaShopItemAvailability::Available;
}

TArray<FHarmoniaCurrencyCost> UHarmoniaShopSubsystem::GetFinalBuyPrice(APlayerController* Player, FName ShopID, FName ShopItemID, int32 Quantity) const
{
	TArray<FHarmoniaCurrencyCost> Result;

	const FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop)
	{
		return Result;
	}

	const FShopItemData* Item = nullptr;
	for (const FShopItemData& ShopItem : Shop->Items)
	{
		if (ShopItem.ShopItemID == ShopItemID)
		{
			Item = &ShopItem;
			break;
		}
	}

	if (!Item)
	{
		return Result;
	}

	for (const FHarmoniaCurrencyCost& BaseCost : Item->BuyPrice)
	{
		FHarmoniaCurrencyCost FinalCost = BaseCost;
		FinalCost.Amount = static_cast<int64>(FinalCost.Amount * Quantity * Shop->BuyPriceModifier);

		// Apply sale discount
		if (Item->bIsOnSale)
		{
			FinalCost.Amount = static_cast<int64>(FinalCost.Amount * (1.0f - Item->SaleDiscount));
		}

		Result.Add(FinalCost);
	}

	return Result;
}

TArray<FHarmoniaCurrencyCost> UHarmoniaShopSubsystem::GetFinalSellPrice(APlayerController* Player, FName ShopID, FGameplayTag ItemTag, int32 Quantity) const
{
	TArray<FHarmoniaCurrencyCost> Result;

	const FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop)
	{
		return Result;
	}

	// Find matching item for sell price reference
	for (const FShopItemData& Item : Shop->Items)
	{
		if (Item.ItemTag == ItemTag)
		{
			for (const FHarmoniaCurrencyCost& BaseCost : Item.SellPrice)
			{
				FHarmoniaCurrencyCost FinalCost = BaseCost;
				FinalCost.Amount = static_cast<int64>(FinalCost.Amount * Quantity * Shop->SellPriceModifier);
				Result.Add(FinalCost);
			}
			return Result;
		}
	}

	// Default sell price if not in shop inventory
	if (ConfigAsset)
	{
		FHarmoniaCurrencyCost DefaultCost(EHarmoniaCurrencyType::Gold, ConfigAsset->DefaultSellPrice * Quantity);
		Result.Add(DefaultCost);
	}

	return Result;
}

bool UHarmoniaShopSubsystem::PurchaseItem(APlayerController* Player, FName ShopID, FName ShopItemID, int32 Quantity)
{
	if (!Player || Quantity <= 0)
	{
		return false;
	}

	// Check availability
	EHarmoniaShopItemAvailability Availability = GetItemAvailability(Player, ShopID, ShopItemID);
	if (Availability != EHarmoniaShopItemAvailability::Available)
	{
		return false;
	}

	FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop)
	{
		return false;
	}

	FShopItemData* Item = nullptr;
	for (FShopItemData& ShopItem : Shop->Items)
	{
		if (ShopItem.ShopItemID == ShopItemID)
		{
			Item = &ShopItem;
			break;
		}
	}

	if (!Item)
	{
		return false;
	}

	// Check stock
	if (Item->MaxStock >= 0 && Item->CurrentStock < Quantity)
	{
		return false;
	}

	// Get final price
	TArray<FHarmoniaCurrencyCost> FinalPrice = GetFinalBuyPrice(Player, ShopID, ShopItemID, Quantity);

	// Remove currency
	for (const FHarmoniaCurrencyCost& Cost : FinalPrice)
	{
		if (!RemovePlayerCurrency(Player, Cost.CurrencyType, Cost.Amount))
		{
			return false;
		}
	}

	// Update stock
	if (Item->MaxStock >= 0)
	{
		Item->CurrentStock -= Quantity;
	}

	// Update purchase count
	uint32 PlayerID = GetPlayerID(Player);
	TMap<FName, int32>& Counts = PlayerPurchaseCounts.FindOrAdd(PlayerID);
	int32& Count = Counts.FindOrAdd(ShopItemID);
	Count += Quantity;

	// TODO: Grant item to player inventory

	// Record transaction
	FTransactionRecord Record;
	Record.PlayerID = PlayerID;
	Record.ShopID = ShopID;
	Record.ItemTag = Item->ItemTag;
	Record.Quantity = Quantity;
	Record.Currency = FinalPrice;
	Record.bWasPurchase = true;
	RecordTransaction(Record);

	OnItemPurchased.Broadcast(Player, ShopID, Item->ItemTag, Quantity);
	return true;
}

bool UHarmoniaShopSubsystem::SellItem(APlayerController* Player, FName ShopID, FGameplayTag ItemTag, int32 Quantity)
{
	if (!Player || Quantity <= 0)
	{
		return false;
	}

	FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop || !Shop->bCanSellItems)
	{
		return false;
	}

	// TODO: Check if player has the item in inventory
	// TODO: Remove item from player inventory

	// Get sell price
	TArray<FHarmoniaCurrencyCost> SellPrice = GetFinalSellPrice(Player, ShopID, ItemTag, Quantity);

	// Add currency
	for (const FHarmoniaCurrencyCost& Currency : SellPrice)
	{
		AddPlayerCurrency(Player, Currency.CurrencyType, Currency.Amount);
	}

	// Record transaction
	uint32 PlayerID = GetPlayerID(Player);
	FTransactionRecord Record;
	Record.PlayerID = PlayerID;
	Record.ShopID = ShopID;
	Record.ItemTag = ItemTag;
	Record.Quantity = Quantity;
	Record.Currency = SellPrice;
	Record.bWasPurchase = false;
	RecordTransaction(Record);

	OnItemSold.Broadcast(Player, ShopID, ItemTag, Quantity);
	return true;
}

bool UHarmoniaShopSubsystem::CanAfford(APlayerController* Player, const TArray<FHarmoniaCurrencyCost>& Cost) const
{
	if (!Player)
	{
		return false;
	}

	for (const FHarmoniaCurrencyCost& Currency : Cost)
	{
		if (GetPlayerCurrency(Player, Currency.CurrencyType) < Currency.Amount)
		{
			return false;
		}
	}

	return true;
}

FGuid UHarmoniaShopSubsystem::InitiateTrade(APlayerController* Initiator, APlayerController* Target)
{
	if (!Initiator || !Target || Initiator == Target)
	{
		return FGuid();
	}

	FTradeOffer Trade;
	Trade.Initiator = Initiator;
	Trade.Target = Target;

	ActiveTrades.Add(Trade.TradeID, Trade);
	return Trade.TradeID;
}

bool UHarmoniaShopSubsystem::AddItemToTrade(FGuid TradeID, APlayerController* Player, FGameplayTag ItemTag)
{
	FTradeOffer* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Player)
	{
		return false;
	}

	// Reset confirmations
	Trade->bInitiatorConfirmed = false;
	Trade->bTargetConfirmed = false;

	if (Trade->Initiator.Get() == Player)
	{
		FTradeOfferItem TradeItem;
		// TradeItem.ItemTag = ItemTag; // Set item tag if FTradeOfferItem has it
		Trade->InitiatorItems.Add(TradeItem);
		return true;
	}
	else if (Trade->Target.Get() == Player)
	{
		FTradeOfferItem TradeItem;
		// TradeItem.ItemTag = ItemTag; // Set item tag if FTradeOfferItem has it
		Trade->TargetItems.Add(TradeItem);
		return true;
	}

	return false;
}

bool UHarmoniaShopSubsystem::RemoveItemFromTrade(FGuid TradeID, APlayerController* Player, FGameplayTag ItemTag)
{
	FTradeOffer* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Player)
	{
		return false;
	}

	// Reset confirmations
	Trade->bInitiatorConfirmed = false;
	Trade->bTargetConfirmed = false;

	if (Trade->Initiator.Get() == Player)
	{
		// Remove first item matching criteria (simplified)
		if (Trade->InitiatorItems.Num() > 0)
		{
			Trade->InitiatorItems.RemoveAt(0);
			return true;
		}
		return false;
	}
	else if (Trade->Target.Get() == Player)
	{
		// Remove first item matching criteria (simplified)
		if (Trade->TargetItems.Num() > 0)
		{
			Trade->TargetItems.RemoveAt(0);
			return true;
		}
		return false;
	}

	return false;
}

bool UHarmoniaShopSubsystem::AddCurrencyToTrade(FGuid TradeID, APlayerController* Player, const FHarmoniaCurrencyCost& Currency)
{
	FTradeOffer* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Player)
	{
		return false;
	}

	// Reset confirmations
	Trade->bInitiatorConfirmed = false;
	Trade->bTargetConfirmed = false;

	if (Trade->Initiator.Get() == Player)
	{
		Trade->InitiatorCurrency.Add(Currency);
		return true;
	}
	else if (Trade->Target.Get() == Player)
	{
		Trade->TargetCurrency.Add(Currency);
		return true;
	}

	return false;
}

bool UHarmoniaShopSubsystem::ConfirmTrade(FGuid TradeID, APlayerController* Player)
{
	FTradeOffer* Trade = ActiveTrades.Find(TradeID);
	if (!Trade || !Player)
	{
		return false;
	}

	if (Trade->Initiator.Get() == Player)
	{
		Trade->bInitiatorConfirmed = true;
	}
	else if (Trade->Target.Get() == Player)
	{
		Trade->bTargetConfirmed = true;
	}
	else
	{
		return false;
	}

	// Execute if both confirmed
	if (Trade->bInitiatorConfirmed && Trade->bTargetConfirmed)
	{
		bool bSuccess = ExecuteTrade(*Trade);
		OnTradeCompleted.Broadcast(*Trade, bSuccess);
		ActiveTrades.Remove(TradeID);
		return bSuccess;
	}

	return true;
}

bool UHarmoniaShopSubsystem::CancelTrade(FGuid TradeID, APlayerController* Player)
{
	FTradeOffer* Trade = ActiveTrades.Find(TradeID);
	if (!Trade)
	{
		return false;
	}

	if (Trade->Initiator.Get() == Player || Trade->Target.Get() == Player)
	{
		OnTradeCompleted.Broadcast(*Trade, false);
		ActiveTrades.Remove(TradeID);
		return true;
	}

	return false;
}

bool UHarmoniaShopSubsystem::GetActiveTrade(FGuid TradeID, FTradeOffer& OutTrade) const
{
	if (const FTradeOffer* Found = ActiveTrades.Find(TradeID))
	{
		OutTrade = *Found;
		return true;
	}
	return false;
}

int64 UHarmoniaShopSubsystem::GetPlayerCurrency(APlayerController* Player, EHarmoniaCurrencyType Type) const
{
	if (!Player)
	{
		return 0;
	}

	uint32 PlayerID = GetPlayerID(Player);
	if (const TMap<EHarmoniaCurrencyType, int64>* Currencies = PlayerCurrencies.Find(PlayerID))
	{
		if (const int64* Amount = Currencies->Find(Type))
		{
			return *Amount;
		}
	}

	return 0;
}

void UHarmoniaShopSubsystem::AddPlayerCurrency(APlayerController* Player, EHarmoniaCurrencyType Type, int64 Amount)
{
	if (!Player || Amount <= 0)
	{
		return;
	}

	uint32 PlayerID = GetPlayerID(Player);
	TMap<EHarmoniaCurrencyType, int64>& Currencies = PlayerCurrencies.FindOrAdd(PlayerID);
	int64& CurrentAmount = Currencies.FindOrAdd(Type);
	CurrentAmount += Amount;
}

bool UHarmoniaShopSubsystem::RemovePlayerCurrency(APlayerController* Player, EHarmoniaCurrencyType Type, int64 Amount)
{
	if (!Player || Amount <= 0)
	{
		return false;
	}

	uint32 PlayerID = GetPlayerID(Player);
	TMap<EHarmoniaCurrencyType, int64>* Currencies = PlayerCurrencies.Find(PlayerID);
	if (!Currencies)
	{
		return false;
	}

	int64* CurrentAmount = Currencies->Find(Type);
	if (!CurrentAmount || *CurrentAmount < Amount)
	{
		return false;
	}

	*CurrentAmount -= Amount;
	return true;
}

TArray<FTransactionRecord> UHarmoniaShopSubsystem::GetTransactionHistory(APlayerController* Player, int32 MaxRecords) const
{
	if (!Player)
	{
		return TArray<FTransactionRecord>();
	}

	uint32 PlayerID = GetPlayerID(Player);
	if (const TArray<FTransactionRecord>* Records = TransactionHistory.Find(PlayerID))
	{
		if (Records->Num() <= MaxRecords)
		{
			return *Records;
		}

		// Return most recent records
		TArray<FTransactionRecord> Result;
		int32 StartIndex = Records->Num() - MaxRecords;
		for (int32 i = StartIndex; i < Records->Num(); ++i)
		{
			Result.Add((*Records)[i]);
		}
		return Result;
	}

	return TArray<FTransactionRecord>();
}

void UHarmoniaShopSubsystem::ProcessRestock(FName ShopID)
{
	FShopDefinition* Shop = Shops.Find(ShopID);
	if (!Shop)
	{
		return;
	}

	for (FShopItemData& Item : Shop->Items)
	{
		if (Item.MaxStock >= 0 && Item.RestockTime > 0.0f)
		{
			Item.CurrentStock = Item.MaxStock;
		}
	}
}

bool UHarmoniaShopSubsystem::ExecuteTrade(FTradeOffer& Trade)
{
	APlayerController* Initiator = Trade.Initiator.Get();
	APlayerController* Target = Trade.Target.Get();

	if (!Initiator || !Target)
	{
		return false;
	}

	// Validate initiator has items and currency
	for (const FHarmoniaCurrencyCost& Currency : Trade.InitiatorCurrency)
	{
		if (GetPlayerCurrency(Initiator, Currency.CurrencyType) < Currency.Amount)
		{
			return false;
		}
	}

	// Validate target has items and currency
	for (const FHarmoniaCurrencyCost& Currency : Trade.TargetCurrency)
	{
		if (GetPlayerCurrency(Target, Currency.CurrencyType) < Currency.Amount)
		{
			return false;
		}
	}

	// TODO: Validate items in inventory

	// Execute currency transfer
	for (const FHarmoniaCurrencyCost& Currency : Trade.InitiatorCurrency)
	{
		RemovePlayerCurrency(Initiator, Currency.CurrencyType, Currency.Amount);
		AddPlayerCurrency(Target, Currency.CurrencyType, Currency.Amount);
	}

	for (const FHarmoniaCurrencyCost& Currency : Trade.TargetCurrency)
	{
		RemovePlayerCurrency(Target, Currency.CurrencyType, Currency.Amount);
		AddPlayerCurrency(Initiator, Currency.CurrencyType, Currency.Amount);
	}

	// TODO: Execute item transfer

	return true;
}

void UHarmoniaShopSubsystem::RecordTransaction(const FTransactionRecord& Record)
{
	TArray<FTransactionRecord>& Records = TransactionHistory.FindOrAdd(Record.PlayerID);
	Records.Add(Record);

	// Limit history size
	const int32 MaxHistory = ConfigAsset ? ConfigAsset->MaxTransactionHistory : 1000;
	while (Records.Num() > MaxHistory)
	{
		Records.RemoveAt(0);
	}
}

uint32 UHarmoniaShopSubsystem::GetPlayerID(APlayerController* Player) const
{
	if (!Player)
	{
		return 0;
	}
	return Player->GetUniqueID();
}
