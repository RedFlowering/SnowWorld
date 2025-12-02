// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaShopSystemDefinitions.h"
#include "HarmoniaShopConfigDataAsset.generated.h"

/**
 * Data asset for shop system configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaShopConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Default Shops
	// ============================================================================

	/** Default shops to register on startup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shops")
	TArray<FHarmoniaShopData> DefaultShops;

	// ============================================================================
	// Pricing Settings
	// ============================================================================

	/** Default sell price for items not in shop inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	int64 DefaultSellPrice = 1;

	/** Global buy price multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	float GlobalBuyPriceMultiplier = 1.0f;

	/** Global sell price multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pricing")
	float GlobalSellPriceMultiplier = 1.0f;

	// ============================================================================
	// Trading Settings
	// ============================================================================

	/** Trade expiration time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
	float TradeExpirationTime = 300.0f;

	/** Maximum items per trade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
	int32 MaxItemsPerTrade = 20;

	/** Whether to allow trades between players on different teams */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading")
	bool bAllowCrossTeamTrades = true;

	// ============================================================================
	// History Settings
	// ============================================================================

	/** Maximum transaction history entries per player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	int32 MaxTransactionHistory = 1000;

	// ============================================================================
	// Currency Settings
	// ============================================================================

	/** Starting gold for new players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	int64 StartingGold = 100;

	/** Starting premium currency for new players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	int64 StartingPremium = 0;
};
