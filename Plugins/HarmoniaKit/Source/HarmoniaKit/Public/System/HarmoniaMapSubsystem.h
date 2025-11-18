// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapSubsystem.generated.h"

/**
 * Game instance subsystem for managing global map data
 *
 * NETWORK NOTE: GameInstanceSubsystem is NOT replicated.
 * Server and each client have their own independent instance.
 * GlobalLocations are local to each game instance and not synchronized across the network.
 * For shared/synchronized locations, use UHarmoniaMapComponent's DiscoveredLocations instead.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaMapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Register a map data asset
	UFUNCTION(BlueprintCallable, Category = "Map")
	void RegisterMapData(UHarmoniaMapDataAsset* MapData);

	// Get current active map data
	UFUNCTION(BlueprintCallable, Category = "Map")
	UHarmoniaMapDataAsset* GetCurrentMapData() const { return CurrentMapData; }

	// Set current active map
	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetCurrentMapData(UHarmoniaMapDataAsset* MapData);

	// Add a global location/POI
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	void AddLocation(const FMapLocationData& Location);

	// Remove a location
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	void RemoveLocation(const FVector& WorldPosition, float Threshold = 100.0f);

	// Get all locations
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	TArray<FMapLocationData> GetAllLocations() const { return GlobalLocations; }

	// Get locations within a radius
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	TArray<FMapLocationData> GetLocationsInRadius(const FVector& Center, float Radius) const;

	// Get locations by tag
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	TArray<FMapLocationData> GetLocationsByTag(FGameplayTag Tag) const;

	// Find location by name
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	bool FindLocationByName(const FText& Name, FMapLocationData& OutLocation) const;

	// Update a location
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	void UpdateLocation(const FVector& WorldPosition, const FMapLocationData& NewData);

	// Clear all locations
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	void ClearAllLocations();

protected:
	// Currently active map data
	UPROPERTY()
	TObjectPtr<UHarmoniaMapDataAsset> CurrentMapData;

	// Global locations (POIs, quest markers, etc.)
	UPROPERTY()
	TArray<FMapLocationData> GlobalLocations;

	// Registered map data assets
	UPROPERTY()
	TArray<TObjectPtr<UHarmoniaMapDataAsset>> RegisteredMaps;
};
