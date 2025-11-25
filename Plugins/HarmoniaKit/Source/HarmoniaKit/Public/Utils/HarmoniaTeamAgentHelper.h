// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Definitions/HarmoniaTeamSystemDefinitions.h"
#include "HarmoniaTeamAgentHelper.generated.h"

/**
 * Helper class for IHarmoniaTeamAgentInterface implementations
 *
 * Provides default implementations for common team agent functionality.
 * Use these static methods in your IHarmoniaTeamAgentInterface _Implementation methods
 * to get standard behavior without duplicating code.
 *
 * Example usage in implementing class:
 *
 * FHarmoniaTeamIdentification AMyActor::GetTeamID_Implementation() const
 * {
 *     return MyTeamID;
 * }
 *
 * EHarmoniaTeamRelationship AMyActor::GetRelationshipWith_Implementation(AActor* OtherActor) const
 * {
 *     return UHarmoniaTeamAgentHelper::GetRelationshipWith(this, OtherActor);
 * }
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaTeamAgentHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Default implementation for GetRelationshipWith
	 * Checks if the other actor implements IHarmoniaTeamAgentInterface and compares team IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static EHarmoniaTeamRelationship GetRelationshipWith(const UObject* TeamAgent, AActor* OtherActor);

	/**
	 * Default implementation for CanAttackActor
	 * Returns true if the relationship is Enemy
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static bool CanAttackActor(const UObject* TeamAgent, AActor* OtherActor);

	/**
	 * Default implementation for ShouldHelpActor
	 * Returns true if the relationship is Ally
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static bool ShouldHelpActor(const UObject* TeamAgent, AActor* OtherActor);

	/**
	 * Default implementation for IsSameTeamAs
	 * Returns true if both actors have the same team ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static bool IsSameTeamAs(const UObject* TeamAgent, AActor* OtherActor);

	/**
	 * Default implementation for IsAllyWith
	 * Returns true if the relationship is Ally
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static bool IsAllyWith(const UObject* TeamAgent, AActor* OtherActor);

	/**
	 * Default implementation for IsEnemyWith
	 * Returns true if the relationship is Enemy
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static bool IsEnemyWith(const UObject* TeamAgent, AActor* OtherActor);

	/**
	 * Get team ID from an actor that implements IHarmoniaTeamAgentInterface
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Team|Helper")
	static FHarmoniaTeamIdentification GetTeamIDFromActor(AActor* Actor);

	/**
	 * Check if an actor implements IHarmoniaTeamAgentInterface
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Team|Helper")
	static bool DoesActorImplementTeamInterface(AActor* Actor);
};
