// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapComponent.generated.h"

class UHarmoniaMapDataAsset;
class UHarmoniaMapSubsystem;
class UHarmoniaFogOfWarRenderer;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocationDiscovered, const FMapLocationData&, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegionExplored, const FExploredRegion&, Region);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPingCreated, const FMapPingData&, Ping);

/**
 * Component attached to player character to track map exploration and discoveries
 */
UCLASS(Blueprintable, ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaMapComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Current map data asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	TObjectPtr<UHarmoniaMapDataAsset> CurrentMapData;

	// Explored regions (replicated to owner only)
	UPROPERTY(ReplicatedUsing = OnRep_ExploredRegions, BlueprintReadOnly, Category = "Map|Exploration")
	TArray<FExploredRegion> ExploredRegions;

	// Discovered locations (replicated to owner only)
	UPROPERTY(ReplicatedUsing = OnRep_DiscoveredLocations, BlueprintReadOnly, Category = "Map|Locations")
	TArray<FMapLocationData> DiscoveredLocations;

	// Active pings created by this player (replicated to all clients for team coordination)
	// NOTE: Each player's pings are stored in their own MapComponent.
	// To display all team pings, iterate through all player characters' MapComponents.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Map|Pings")
	TArray<FMapPingData> ActivePings;

	// Player bookmarks (replicated to owner only)
	UPROPERTY(ReplicatedUsing = OnRep_Bookmarks, BlueprintReadOnly, Category = "Map|Bookmarks")
	TArray<FMapBookmark> Bookmarks;

	// Exploration radius around the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Exploration")
	float ExplorationRadius = 1000.0f;

	// How often to update exploration (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Exploration")
	float ExplorationUpdateInterval = 1.0f;

	// Whether to automatically explore around the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Exploration")
	bool bAutoExplore = true;

	// Whether to show all locations (for debugging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Debug")
	bool bRevealAllLocations = false;

	// Whether to reveal the entire map (for debugging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Debug")
	bool bRevealEntireMap = false;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnLocationDiscovered OnLocationDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnRegionExplored OnRegionExplored;

	UPROPERTY(BlueprintAssignable, Category = "Map|Events")
	FOnPingCreated OnPingCreated;

	// Check if a location is explored
	UFUNCTION(BlueprintCallable, Category = "Map|Exploration")
	bool IsLocationExplored(const FVector& WorldLocation) const;

	// Get exploration alpha (0-1) at a location
	UFUNCTION(BlueprintCallable, Category = "Map|Exploration")
	float GetExplorationAlpha(const FVector& WorldLocation) const;

	// Manually add an explored region
	UFUNCTION(BlueprintCallable, Category = "Map|Exploration")
	void AddExploredRegion(const FVector& Center, float Radius);

	// Discover a location
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	void DiscoverLocation(const FMapLocationData& Location);

	// Check if a location is discovered
	UFUNCTION(BlueprintCallable, Category = "Map|Locations")
	bool IsLocationDiscovered(const FVector& WorldLocation, float Threshold = 100.0f) const;

	// Create a ping at world location
	UFUNCTION(BlueprintCallable, Category = "Map|Pings")
	void CreatePing(const FVector& WorldLocation, float Lifetime = 5.0f, FGameplayTag PingTag = FGameplayTag());

	// Remove a ping
	UFUNCTION(BlueprintCallable, Category = "Map|Pings")
	void RemovePing(int32 PingIndex);

	// Clear all pings
	UFUNCTION(BlueprintCallable, Category = "Map|Pings")
	void ClearAllPings();

	// Get all active pings
	UFUNCTION(BlueprintCallable, Category = "Map|Pings")
	TArray<FMapPingData> GetActivePings() const { return ActivePings; }

	// Add a bookmark
	UFUNCTION(BlueprintCallable, Category = "Map|Bookmarks")
	void AddBookmark(const FText& Name, const FVector& Location, const FLinearColor& Color = FLinearColor::White);

	// Remove a bookmark
	UFUNCTION(BlueprintCallable, Category = "Map|Bookmarks")
	void RemoveBookmark(int32 BookmarkIndex);

	// Get all bookmarks
	UFUNCTION(BlueprintCallable, Category = "Map|Bookmarks")
	TArray<FMapBookmark> GetBookmarks() const { return Bookmarks; }

	// Get player world position
	UFUNCTION(BlueprintCallable, Category = "Map")
	FVector GetPlayerWorldPosition() const;

	// Get player world rotation (yaw)
	UFUNCTION(BlueprintCallable, Category = "Map")
	float GetPlayerWorldRotation() const;

	// Convert world position to map UV
	UFUNCTION(BlueprintCallable, Category = "Map")
	FVector2D WorldToMapUV(const FVector& WorldPosition) const;

	// Convert map UV to world position
	UFUNCTION(BlueprintCallable, Category = "Map")
	FVector MapUVToWorld(const FVector2D& UV, float WorldZ = 0.0f) const;

	// Save exploration data
	UFUNCTION(BlueprintCallable, Category = "Map|Save")
	void SaveExplorationData();

	// Load exploration data
	UFUNCTION(BlueprintCallable, Category = "Map|Save")
	void LoadExplorationData();

protected:
	// Timer for exploration updates
	float ExplorationUpdateTimer = 0.0f;

	// Replication notify for explored regions
	UFUNCTION()
	void OnRep_ExploredRegions();

	// Replication notify for discovered locations
	UFUNCTION()
	void OnRep_DiscoveredLocations();

	// Update exploration around player
	void UpdateExploration();

	// Check for nearby undiscovered locations
	void CheckNearbyLocations();

	// Update active pings (remove expired ones)
	void UpdatePings(float DeltaTime);

	// Server RPC for creating pings
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCreatePing(const FVector& WorldLocation, float Lifetime, FGameplayTag PingTag);

	// Server RPC for adding explored regions
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddExploredRegion(const FVector& Center, float Radius);

	// Server RPC for discovering locations
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDiscoverLocation(const FMapLocationData& Location);

	// Server RPC for adding bookmarks
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddBookmark(const FMapBookmark& Bookmark);

	// Server RPC for removing bookmarks
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveBookmark(int32 BookmarkIndex);

	// Replication notify for bookmarks
	UFUNCTION()
	void OnRep_Bookmarks();

	// Multicast RPC for broadcasting ping creation (pings are shared with all players)
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnPingCreated(const FMapPingData& Ping);

public:
	// Get fog of war renderer
	UFUNCTION(BlueprintPure, Category = "Map|Fog of War")
	UHarmoniaFogOfWarRenderer* GetFogOfWarRenderer() const { return FogOfWarRenderer; }

private:
	// Cached subsystem reference
	UPROPERTY()
	TObjectPtr<UHarmoniaMapSubsystem> MapSubsystem;

	// Fog of war renderer
	UPROPERTY(Transient)
	TObjectPtr<UHarmoniaFogOfWarRenderer> FogOfWarRenderer;

	// Get or cache map subsystem
	UHarmoniaMapSubsystem* GetMapSubsystem();

	// Initialize fog of war renderer
	void InitializeFogOfWarRenderer();

	// Rate limiting for ping creation
	float LastPingTime = 0.0f;
	TArray<float> PingTimestamps;
};
