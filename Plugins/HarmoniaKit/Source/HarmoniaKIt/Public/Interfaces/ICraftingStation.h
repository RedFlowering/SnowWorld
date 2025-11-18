// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "ICraftingStation.generated.h"

/**
 * Interface for crafting station actors
 * Implement this interface on any actor that should function as a crafting station
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UICraftingStation : public UInterface
{
	GENERATED_BODY()
};

class HARMONIAKIT_API IICraftingStation
{
	GENERATED_BODY()

public:
	/**
	 * Get the type of this crafting station
	 * Used for server-side validation to prevent clients from spoofing station types
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting|Station")
	ECraftingStationType GetStationType() const;

	/**
	 * Get the station tags for custom stations
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting|Station")
	FGameplayTagContainer GetStationTags() const;

	/**
	 * Check if this station is available for use
	 * Can be used to implement fuel systems, ownership checks, etc.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting|Station")
	bool IsAvailableForUse(AActor* User) const;

	/**
	 * Called when a player starts using this station
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting|Station")
	void OnStationUsageStarted(AActor* User);

	/**
	 * Called when a player stops using this station
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Crafting|Station")
	void OnStationUsageEnded(AActor* User);
};
