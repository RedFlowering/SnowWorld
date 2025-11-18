// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Definitions/HarmoniaDeathPenaltyDefinitions.h"
#include "HarmoniaDeathPenaltyLibrary.generated.h"

class UHarmoniaDeathPenaltyComponent;
class UHarmoniaCurrencyManagerComponent;
class AHarmoniaMemoryEchoActor;

/**
 * Blueprint function library for death penalty and currency systems
 * Provides convenient static functions for common operations
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDeathPenaltyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//~ Component Access Helpers
	/** Get death penalty component from actor (player character) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Death Penalty", meta = (DefaultToSelf = "Actor"))
	static UHarmoniaDeathPenaltyComponent* GetDeathPenaltyComponent(AActor* Actor);

	/** Get currency manager component from actor (player character) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Currency", meta = (DefaultToSelf = "Actor"))
	static UHarmoniaCurrencyManagerComponent* GetCurrencyManagerComponent(AActor* Actor);

	//~ Death Penalty Operations
	/** Handle player death at specified location */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death Penalty", meta = (DefaultToSelf = "PlayerCharacter"))
	static void HandlePlayerDeath(AActor* PlayerCharacter, const FVector& DeathLocation);

	/** Attempt to recover currencies from nearby memory echo */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death Penalty", meta = (DefaultToSelf = "PlayerCharacter"))
	static bool RecoverCurrencies(AActor* PlayerCharacter);

	/** Check if player is in ethereal (weakened) state */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Death Penalty", meta = (DefaultToSelf = "PlayerCharacter"))
	static bool IsPlayerInEtherealState(AActor* PlayerCharacter);

	/** Get distance to player's memory echo */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Death Penalty", meta = (DefaultToSelf = "PlayerCharacter"))
	static float GetDistanceToMemoryEcho(AActor* PlayerCharacter);

	/** Reset all death penalties (call at safe zones/rest points) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death Penalty", meta = (DefaultToSelf = "PlayerCharacter"))
	static void ResetDeathPenalties(AActor* PlayerCharacter);

	//~ Currency Operations
	/** Add currency to player */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Currency", meta = (DefaultToSelf = "PlayerCharacter"))
	static bool AddCurrency(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Remove currency from player */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Currency", meta = (DefaultToSelf = "PlayerCharacter"))
	static bool RemoveCurrency(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Get currency amount */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Currency", meta = (DefaultToSelf = "PlayerCharacter"))
	static int32 GetCurrencyAmount(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType);

	/** Check if player has enough currency */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Currency", meta = (DefaultToSelf = "PlayerCharacter"))
	static bool HasCurrency(AActor* PlayerCharacter, EHarmoniaCurrencyType CurrencyType, int32 Amount);

	//~ Utility Functions
	/** Get localized currency name */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Currency")
	static FText GetCurrencyName(EHarmoniaCurrencyType CurrencyType);

	/** Get currency type from string (for parsing) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Currency")
	static EHarmoniaCurrencyType GetCurrencyTypeFromString(const FString& TypeString);

	/** Format currency amount for display */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Currency")
	static FText FormatCurrencyAmount(EHarmoniaCurrencyType CurrencyType, int32 Amount);

	/** Find nearest memory echo to a location */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death Penalty", meta = (WorldContext = "WorldContextObject"))
	static AHarmoniaMemoryEchoActor* FindNearestMemoryEcho(UObject* WorldContextObject, const FVector& Location, float MaxDistance = 10000.0f);

	/** Get all memory echoes in world */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death Penalty", meta = (WorldContext = "WorldContextObject"))
	static TArray<AHarmoniaMemoryEchoActor*> GetAllMemoryEchoes(UObject* WorldContextObject);
};
