// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HarmoniaTerrainOwnershipManager.generated.h"

/**
 * Zone ownership data
 * Represents a territory that can be owned by a player or team
 */
USTRUCT(BlueprintType)
struct FTerrainOwnershipZone
{
	GENERATED_BODY()

	/** Center location of the zone */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	FVector Center = FVector::ZeroVector;

	/** Radius of the zone */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	float Radius = 1000.0f;

	/** Owner player ID (empty if owned by team) */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	FString OwnerPlayerID;

	/** Owner team ID (INDEX_NONE if owned by individual player) */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	int32 OwnerTeamID = INDEX_NONE;

	/** Zone name/description */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	FString ZoneName;

	/** Whether this is a protected zone (no building allowed) */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	bool bProtectedZone = false;

	/** Whether building is allowed in this zone by non-owners */
	UPROPERTY(BlueprintReadWrite, Category = "Ownership")
	bool bAllowPublicBuilding = false;
};

/**
 * UHarmoniaTerrainOwnershipManager
 *
 * World subsystem for managing terrain ownership and building permissions
 * Features:
 * - Zone-based ownership (individual or team)
 * - Building permission validation
 * - Protected zones (cities, dungeons, etc.)
 * - Public building areas
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaTerrainOwnershipManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~UWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of UWorldSubsystem interface

	// ============================================================================
	// Zone Management
	// ============================================================================

	/**
	 * Register a new ownership zone
	 * @param Zone - Zone to register
	 * @return Zone index if successful, INDEX_NONE otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	int32 RegisterOwnershipZone(const FTerrainOwnershipZone& Zone);

	/**
	 * Unregister an ownership zone
	 * @param ZoneIndex - Index of zone to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	void UnregisterOwnershipZone(int32 ZoneIndex);

	/**
	 * Get zone at location
	 * @param Location - World location to check
	 * @param OutZone - Zone data at location
	 * @return True if zone found, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	bool GetZoneAtLocation(const FVector& Location, FTerrainOwnershipZone& OutZone) const;

	/**
	 * Get all zones owned by a player
	 * @param PlayerID - Player ID to check
	 * @return Array of zone indices
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	TArray<int32> GetZonesOwnedByPlayer(const FString& PlayerID) const;

	/**
	 * Get all zones owned by a team
	 * @param TeamID - Team ID to check
	 * @return Array of zone indices
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	TArray<int32> GetZonesOwnedByTeam(int32 TeamID) const;

	// ============================================================================
	// Permission Checks
	// ============================================================================

	/**
	 * Check if an actor has building permission at a location
	 * Takes into account:
	 * - Zone ownership (player or team)
	 * - Protected zones
	 * - Public building areas
	 * - Team membership
	 *
	 * @param Actor - Actor requesting permission
	 * @param Location - World location to check
	 * @return True if actor can build at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	bool HasBuildingPermission(AActor* Actor, const FVector& Location) const;

	/**
	 * Check if a player has permission at a location
	 * @param PlayerID - Player ID to check
	 * @param TeamID - Team ID of the player (INDEX_NONE if no team)
	 * @param Location - World location to check
	 * @return True if player can build at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	bool HasBuildingPermissionByID(const FString& PlayerID, int32 TeamID, const FVector& Location) const;

	/**
	 * Check if location is in a protected zone
	 * @param Location - World location to check
	 * @return True if location is in a protected zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	bool IsProtectedZone(const FVector& Location) const;

	/**
	 * Check if location is in a public building area
	 * @param Location - World location to check
	 * @return True if anyone can build at this location
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	bool IsPublicBuildingArea(const FVector& Location) const;

	// ============================================================================
	// Zone Creation Helpers
	// ============================================================================

	/**
	 * Create a protected zone (no building allowed)
	 * Useful for cities, dungeons, quest areas, etc.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	int32 CreateProtectedZone(const FVector& Center, float Radius, const FString& ZoneName);

	/**
	 * Create a public building zone (anyone can build)
	 * Useful for community areas, shared territories, etc.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	int32 CreatePublicBuildingZone(const FVector& Center, float Radius, const FString& ZoneName);

	/**
	 * Create a player-owned zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	int32 CreatePlayerZone(const FVector& Center, float Radius, const FString& PlayerID, const FString& ZoneName);

	/**
	 * Create a team-owned zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	int32 CreateTeamZone(const FVector& Center, float Radius, int32 TeamID, const FString& ZoneName);

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Set whether building is allowed in unclaimed areas
	 * Default: true
	 */
	UFUNCTION(BlueprintCallable, Category = "Terrain|Ownership")
	void SetAllowBuildingInUnclaimedAreas(bool bAllow);

	/**
	 * Get whether building is allowed in unclaimed areas
	 */
	UFUNCTION(BlueprintPure, Category = "Terrain|Ownership")
	bool GetAllowBuildingInUnclaimedAreas() const { return bAllowBuildingInUnclaimedAreas; }

protected:
	/** All registered ownership zones */
	UPROPERTY()
	TArray<FTerrainOwnershipZone> OwnershipZones;

	/** Whether building is allowed in unclaimed areas */
	UPROPERTY()
	bool bAllowBuildingInUnclaimedAreas = true;

	/**
	 * Get player ID from actor
	 */
	FString GetPlayerIDFromActor(AActor* Actor) const;

	/**
	 * Get team ID from actor
	 */
	int32 GetTeamIDFromActor(AActor* Actor) const;
};
