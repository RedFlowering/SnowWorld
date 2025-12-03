// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

/**
 * @file HarmoniaCheatLibrary.h
 * @brief Blueprint function library for cheat manager access
 * @author Harmonia Team
 * 
 * Provides easy Blueprint access to the cheat manager system.
 * 
 * @warning This library is not compiled in Shipping builds and the cheat manager
 *          does not exist, so all functions will silently fail.
 */

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaCheatLibrary.generated.h"

/**
 * @class UHarmoniaCheatLibrary
 * @brief Blueprint function library for cheat manager access
 * 
 * Provides easy Blueprint access to the cheat manager system.
 * Not available in Shipping builds.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Gets the cheat manager for the current player
	 * @param WorldContextObject World context
	 * @return Cheat manager (nullptr if unavailable)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static class UHarmoniaCheatManager* GetHarmoniaCheatManager(const UObject* WorldContextObject);

	/**
	 * @brief Checks if cheat manager is created and available
	 * @param WorldContextObject World context
	 * @return True if cheat manager is available
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatManagerAvailable(const UObject* WorldContextObject);

	/**
	 * @brief Checks if cheats are currently enabled (requires Development build and -cheat command line option)
	 * @param WorldContextObject World context
	 * @return True if cheats are enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatsEnabled(const UObject* WorldContextObject);

	// ==================== Quick Cheat Functions ====================

	/**
	 * @brief Quick cheat: Fully heals the player
	 * @param WorldContextObject World context
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickHeal(const UObject* WorldContextObject);

	/**
	 * @brief Quick cheat: Toggles invincibility mode
	 * @param WorldContextObject World context
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleInvincible(const UObject* WorldContextObject);

	/**
	 * @brief Quick cheat: Grants gold (default 10000)
	 * @param WorldContextObject World context
	 * @param Amount Amount of gold to grant
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickGiveGold(const UObject* WorldContextObject, int32 Amount = 10000);

	/**
	 * @brief Quick cheat: Toggles fly mode
	 * @param WorldContextObject World context
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleFly(const UObject* WorldContextObject);

	/**
	 * @brief Quick cheat: Sets movement speed to 2x
	 * @param WorldContextObject World context
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickDoubleSpeed(const UObject* WorldContextObject);

	/**
	 * @brief Quick cheat: Resets all cheats
	 * @param WorldContextObject World context
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickResetCheats(const UObject* WorldContextObject);
};
