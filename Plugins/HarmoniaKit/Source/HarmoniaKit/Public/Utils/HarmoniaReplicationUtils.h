// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaReplicationUtils.generated.h"

/**
 * Utility functions for network replication
 * Provides common patterns and helpers for multiplayer functionality
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaReplicationUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Check if this is the server
	 * @param WorldContextObject - Object with world context
	 * @return True if running on server
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication", meta = (WorldContext = "WorldContextObject"))
	static bool IsServer(const UObject* WorldContextObject);

	/**
	 * Check if this is a client
	 * @param WorldContextObject - Object with world context
	 * @return True if running on client
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication", meta = (WorldContext = "WorldContextObject"))
	static bool IsClient(const UObject* WorldContextObject);

	/**
	 * Check if this is a standalone game (not multiplayer)
	 * @param WorldContextObject - Object with world context
	 * @return True if standalone
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication", meta = (WorldContext = "WorldContextObject"))
	static bool IsStandalone(const UObject* WorldContextObject);

	/**
	 * Check if this is the locally controlled pawn
	 * @param Pawn - The pawn to check
	 * @return True if locally controlled
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication")
	static bool IsLocallyControlled(const APawn* Pawn);

	/**
	 * Check if this actor has authority (server)
	 * @param Actor - The actor to check
	 * @return True if has authority
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication")
	static bool HasAuthority(const AActor* Actor);

	/**
	 * Check if this actor is owned by the local player
	 * @param Actor - The actor to check
	 * @return True if owned by local player
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication")
	static bool IsOwnedByLocalPlayer(const AActor* Actor);

	/**
	 * Get the role of an actor (Authority, AutonomousProxy, SimulatedProxy)
	 * @param Actor - The actor to check
	 * @return The role as a string
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Replication")
	static FString GetActorRoleString(const AActor* Actor);

	/**
	 * Check if actor should replicate to a specific player
	 * Useful for conditional replication logic
	 * @param Actor - The actor
	 * @param PlayerController - The player controller
	 * @return True if should replicate
	 */
	static bool ShouldReplicateToPlayer(const AActor* Actor, const APlayerController* PlayerController);

	/**
	 * Helper for rate-limited RPC calls
	 * @param LastCallTime - Reference to last call time
	 * @param WorldTime - Current world time
	 * @param MinInterval - Minimum interval between calls
	 * @return True if call is allowed
	 */
	static bool IsRPCRateLimited(float& LastCallTime, float WorldTime, float MinInterval = 0.1f);

	/**
	 * Validate actor for RPC (common anti-cheat check)
	 * @param Actor - The actor to validate
	 * @param bRequireAuthority - Whether authority is required
	 * @return True if actor is valid for RPC
	 */
	static bool ValidateActorForRPC(const AActor* Actor, bool bRequireAuthority = true);

	/**
	 * Common replication condition helper
	 * Use in GetLifetimeReplicatedProps
	 */
	template<typename TOwner>
	static void SetupStandardReplication(TArray<FLifetimeProperty>& OutLifetimeProps)
	{
		// This is a template for custom setup
		// Subclasses should call their own version
	}

	/**
	 * Add replicated property with condition
	 * Reduces boilerplate in GetLifetimeReplicatedProps
	 */
	static void AddReplicatedProperty(
		TArray<FLifetimeProperty>& OutLifetimeProps,
		const UClass* InClass,
		const FProperty* Property,
		ELifetimeCondition Condition = COND_None);
};
