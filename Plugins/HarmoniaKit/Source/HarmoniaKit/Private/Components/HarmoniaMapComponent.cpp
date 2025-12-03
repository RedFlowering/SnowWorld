// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaMapComponent.h"
#include "System/HarmoniaMapSubsystem.h"
#include "System/HarmoniaFogOfWarRenderer.h"
#include "System/HarmoniaSaveGameSubsystem.h"
#include "System/HarmoniaSaveGame.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UHarmoniaMapComponent::UHarmoniaMapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);

	ExplorationRadius = 1000.0f;
	ExplorationUpdateInterval = 1.0f;
	bAutoExplore = true;
	bRevealAllLocations = false;
	bRevealEntireMap = false;
}

void UHarmoniaMapComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get map subsystem
	MapSubsystem = GetMapSubsystem();

	// Load exploration data if available
	LoadExplorationData();

	// Initialize from map data asset
	if (CurrentMapData)
	{
		ExplorationRadius = CurrentMapData->ExplorationRadius;
		ExplorationUpdateInterval = CurrentMapData->ExplorationUpdateInterval;

		// Initialize fog of war renderer - only for the owning client or standalone
		// FogOfWarRenderer is purely visual and should only exist on the local player's machine
		if (GetOwnerRole() == ROLE_AutonomousProxy || GetNetMode() == NM_Standalone)
		{
			InitializeFogOfWarRenderer();
		}
	}
}

void UHarmoniaMapComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Only replicate exploration data to the owning client to save bandwidth
	// Each player only needs to see their own explored regions and discovered locations
	DOREPLIFETIME_CONDITION(UHarmoniaMapComponent, ExploredRegions, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHarmoniaMapComponent, DiscoveredLocations, COND_OwnerOnly);

	// Pings are shared with all clients (for team coordination)
	DOREPLIFETIME(UHarmoniaMapComponent, ActivePings);

	// Bookmarks are owner only
	DOREPLIFETIME_CONDITION(UHarmoniaMapComponent, Bookmarks, COND_OwnerOnly);
}

void UHarmoniaMapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only update on server or in standalone
	if (GetOwnerRole() == ROLE_Authority)
	{
		// Update exploration
		if (bAutoExplore)
		{
			ExplorationUpdateTimer += DeltaTime;
			if (ExplorationUpdateTimer >= ExplorationUpdateInterval)
			{
				ExplorationUpdateTimer = 0.0f;
				UpdateExploration();
				CheckNearbyLocations();
			}
		}

		// Update pings
		UpdatePings(DeltaTime);
	}
}

void UHarmoniaMapComponent::OnRep_ExploredRegions()
{
	// Update fog of war renderer when explored regions are replicated
	// This only happens on the owning client (due to COND_OwnerOnly)
	if (FogOfWarRenderer)
	{
		FogOfWarRenderer->UpdateFogOfWar(ExploredRegions);
	}

	// Broadcast event for the newly added region (last in array)
	if (ExploredRegions.Num() > 0)
	{
		OnRegionExplored.Broadcast(ExploredRegions.Last());
	}
}

void UHarmoniaMapComponent::OnRep_DiscoveredLocations()
{
	// Broadcast event for the newly discovered location (last in array)
	if (DiscoveredLocations.Num() > 0)
	{
		OnLocationDiscovered.Broadcast(DiscoveredLocations.Last());
	}
}

void UHarmoniaMapComponent::OnRep_Bookmarks()
{
	// Optional: Broadcast event if UI needs to update immediately
	// OnBookmarksChanged.Broadcast();
}

bool UHarmoniaMapComponent::IsLocationExplored(const FVector& WorldLocation) const
{
	if (bRevealEntireMap)
	{
		return true;
	}

	for (const FExploredRegion& Region : ExploredRegions)
	{
		if (Region.ContainsPoint(WorldLocation))
		{
			return true;
		}
	}

	return false;
}

float UHarmoniaMapComponent::GetExplorationAlpha(const FVector& WorldLocation) const
{
	if (bRevealEntireMap)
	{
		return 1.0f;
	}

	// Find the closest explored region and calculate fade
	float ClosestDistanceRatio = FLT_MAX;

	for (const FExploredRegion& Region : ExploredRegions)
	{
		float Distance = FVector::Dist(Region.Center, WorldLocation);
		float DistanceRatio = Distance / Region.Radius;

		if (DistanceRatio < ClosestDistanceRatio)
		{
			ClosestDistanceRatio = DistanceRatio;
		}
	}

	if (ClosestDistanceRatio <= 1.0f)
	{
		return 1.0f; // Fully explored
	}
	else if (ClosestDistanceRatio <= 1.5f)
	{
		// Fade out from 1.0 to 0.0 over distance
		return FMath::Clamp(1.0f - ((ClosestDistanceRatio - 1.0f) / 0.5f), 0.0f, 1.0f);
	}

	return 0.0f; // Not explored
}

void UHarmoniaMapComponent::AddExploredRegion(const FVector& Center, float Radius)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		// Check if we already have a region nearby
		bool bMerged = false;
		for (FExploredRegion& ExistingRegion : ExploredRegions)
		{
			float Distance = FVector::Dist(ExistingRegion.Center, Center);
			if (Distance < (ExistingRegion.Radius + Radius) * 0.5f)
			{
				// Merge regions by expanding the existing one
				FVector NewCenter = (ExistingRegion.Center + Center) * 0.5f;
				float NewRadius = FMath::Max(ExistingRegion.Radius, Radius) + Distance * 0.5f;

				ExistingRegion.Center = NewCenter;
				ExistingRegion.Radius = NewRadius;
				bMerged = true;
				break;
			}
		}

		if (!bMerged)
		{
			// Add new region
			FExploredRegion NewRegion(Center, Radius);
			NewRegion.ExploredTime = GetWorld()->GetTimeSeconds();
			ExploredRegions.Add(NewRegion);
			// Event will be broadcast via OnRep_ExploredRegions on the owning client
		}
	}
	else
	{
		// Client -> Server
		ServerAddExploredRegion(Center, Radius);
	}
}

void UHarmoniaMapComponent::DiscoverLocation(const FMapLocationData& Location)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		// Check if already discovered
		for (const FMapLocationData& DiscoveredLoc : DiscoveredLocations)
		{
			if (FVector::Dist(DiscoveredLoc.WorldPosition, Location.WorldPosition) < 100.0f)
			{
				return; // Already discovered
			}
		}

		// Add to discovered locations
		FMapLocationData NewLocation = Location;
		NewLocation.bDiscovered = true;
		DiscoveredLocations.Add(NewLocation);
		// Event will be broadcast via OnRep_DiscoveredLocations on the owning client
	}
	else
	{
		// Client -> Server
		ServerDiscoverLocation(Location);
	}
}

bool UHarmoniaMapComponent::IsLocationDiscovered(const FVector& WorldLocation, float Threshold) const
{
	if (bRevealAllLocations)
	{
		return true;
	}

	for (const FMapLocationData& Location : DiscoveredLocations)
	{
		if (FVector::Dist(Location.WorldPosition, WorldLocation) < Threshold)
		{
			return true;
		}
	}

	return false;
}

void UHarmoniaMapComponent::CreatePing(const FVector& WorldLocation, float Lifetime, FGameplayTag PingTag)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		// Get player info
		FString PlayerName = TEXT("Player");
		int32 PlayerId = -1;

		APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (OwnerPawn)
		{
			APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
			if (PC && PC->PlayerState)
			{
				PlayerName = PC->PlayerState->GetPlayerName();
				PlayerId = PC->PlayerState->GetPlayerId();
			}
		}

		// Create ping data
		FMapPingData NewPing(WorldLocation, PlayerName, PlayerId);
		NewPing.Lifetime = Lifetime;
		NewPing.PingTag = PingTag;
		NewPing.CreationTime = GetWorld()->GetTimeSeconds();

		// Limit number of active pings
		int32 MaxPings = CurrentMapData ? CurrentMapData->MaxActivePings : 10;
		if (ActivePings.Num() >= MaxPings)
		{
			// Remove oldest ping
			ActivePings.RemoveAt(0);
		}

		ActivePings.Add(NewPing);

		// Broadcast event
		MulticastOnPingCreated(NewPing);
	}
	else
	{
		// Client -> Server
		ServerCreatePing(WorldLocation, Lifetime, PingTag);
	}
}

void UHarmoniaMapComponent::RemovePing(int32 PingIndex)
{
	// Server-only execution (ActivePings is replicated)
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (ActivePings.IsValidIndex(PingIndex))
	{
		ActivePings.RemoveAt(PingIndex);
	}
}

void UHarmoniaMapComponent::ClearAllPings()
{
	// Server-only execution (ActivePings is replicated)
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	ActivePings.Empty();
}

void UHarmoniaMapComponent::AddBookmark(const FText& Name, const FVector& Location, const FLinearColor& Color)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FMapBookmark NewBookmark;
		NewBookmark.BookmarkName = Name;
		NewBookmark.WorldPosition = Location;
		NewBookmark.Color = Color;
		NewBookmark.bVisible = true;

		Bookmarks.Add(NewBookmark);
		// OnRep_Bookmarks will be called on client
	}
	else
	{
		FMapBookmark NewBookmark;
		NewBookmark.BookmarkName = Name;
		NewBookmark.WorldPosition = Location;
		NewBookmark.Color = Color;
		NewBookmark.bVisible = true;
		
		ServerAddBookmark(NewBookmark);
	}
}

void UHarmoniaMapComponent::RemoveBookmark(int32 BookmarkIndex)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Bookmarks.IsValidIndex(BookmarkIndex))
		{
			Bookmarks.RemoveAt(BookmarkIndex);
		}
	}
	else
	{
		ServerRemoveBookmark(BookmarkIndex);
	}
}

FVector UHarmoniaMapComponent::GetPlayerWorldPosition() const
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		return Owner->GetActorLocation();
	}
	return FVector::ZeroVector;
}

float UHarmoniaMapComponent::GetPlayerWorldRotation() const
{
	AActor* Owner = GetOwner();
	if (Owner)
	{
		return Owner->GetActorRotation().Yaw;
	}
	return 0.0f;
}

FVector2D UHarmoniaMapComponent::WorldToMapUV(const FVector& WorldPosition) const
{
	if (CurrentMapData)
	{
		return CurrentMapData->CapturedMapData.WorldToMapUV(WorldPosition);
	}
	return FVector2D::ZeroVector;
}

FVector UHarmoniaMapComponent::MapUVToWorld(const FVector2D& UV, float WorldZ) const
{
	if (CurrentMapData)
	{
		return CurrentMapData->CapturedMapData.MapUVToWorld(UV, WorldZ);
	}
	return FVector::ZeroVector;
}

void UHarmoniaMapComponent::SaveExplorationData()
{
	// Save exploration data - integrate with save system
	UE_LOG(LogTemp, Log, TEXT("SaveExplorationData called (%d regions, %d locations)"),
		ExploredRegions.Num(), DiscoveredLocations.Num());

	// Integrate with HarmoniaSaveGameSubsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaSaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHarmoniaSaveGameSubsystem>();
	if (!SaveSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveExplorationData: HarmoniaSaveGameSubsystem not found"));
		return;
	}

	// Get current save game object
	UHarmoniaSaveGame* SaveGame = SaveSubsystem->GetCurrentSaveGame();
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveExplorationData: No active save game"));
		return;
	}

	// Get player's Steam ID
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	// Get or create player data
	FString SteamID = TEXT("LocalPlayer"); // Default for local testing
	// In multiplayer, you would get the actual Steam ID here

	FHarmoniaPlayerSaveData PlayerData;
	if (!SaveGame->GetPlayerData(SteamID, PlayerData))
	{
		// Create new player data if it doesn't exist
		PlayerData.SteamID = SteamID;
		PlayerData.PlayerName = PC->PlayerState ? PC->PlayerState->GetPlayerName() : TEXT("Player");
	}

	// Update exploration data
	PlayerData.ExploredRegions = ExploredRegions;
	PlayerData.DiscoveredLocations = DiscoveredLocations;

	// Save back to save game
	SaveGame->SetPlayerData(SteamID, PlayerData);

	UE_LOG(LogTemp, Log, TEXT("SaveExplorationData: Saved %d regions and %d locations for player %s"),
		ExploredRegions.Num(), DiscoveredLocations.Num(), *SteamID);
}

void UHarmoniaMapComponent::LoadExplorationData()
{
	// Load exploration data - integrate with save system
	UE_LOG(LogTemp, Log, TEXT("LoadExplorationData called"));

	// Integrate with HarmoniaSaveGameSubsystem
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaSaveGameSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHarmoniaSaveGameSubsystem>();
	if (!SaveSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadExplorationData: HarmoniaSaveGameSubsystem not found"));
		return;
	}

	// Get current save game object
	UHarmoniaSaveGame* SaveGame = SaveSubsystem->GetCurrentSaveGame();
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadExplorationData: No active save game"));
		return;
	}

	// Get player's Steam ID
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		return;
	}

	// Get player data
	FString SteamID = TEXT("LocalPlayer"); // Default for local testing
	// In multiplayer, you would get the actual Steam ID here

	FHarmoniaPlayerSaveData PlayerData;
	if (SaveGame->GetPlayerData(SteamID, PlayerData))
	{
		// Load exploration data
		ExploredRegions = PlayerData.ExploredRegions;
		DiscoveredLocations = PlayerData.DiscoveredLocations;

		// Trigger replication callbacks to update fog of war and UI
		OnRep_ExploredRegions();
		OnRep_DiscoveredLocations();

		UE_LOG(LogTemp, Log, TEXT("LoadExplorationData: Loaded %d regions and %d locations for player %s"),
			ExploredRegions.Num(), DiscoveredLocations.Num(), *SteamID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadExplorationData: No saved data found for player %s"), *SteamID);
	}
}

void UHarmoniaMapComponent::UpdateExploration()
{
	FVector PlayerPosition = GetPlayerWorldPosition();
	AddExploredRegion(PlayerPosition, ExplorationRadius);
}

void UHarmoniaMapComponent::CheckNearbyLocations()
{
	if (!CurrentMapData || !MapSubsystem)
	{
		return;
	}

	FVector PlayerPosition = GetPlayerWorldPosition();

	// Check static locations from map data
	for (const FMapLocationData& Location : CurrentMapData->StaticLocations)
	{
		if (!Location.bDiscovered && !IsLocationDiscovered(Location.WorldPosition))
		{
			// Check if player is close enough to discover
			float DiscoveryDistance = ExplorationRadius * 0.5f; // Discover at half the exploration radius
			if (FVector::Dist(PlayerPosition, Location.WorldPosition) < DiscoveryDistance)
			{
				DiscoverLocation(Location);
			}
		}
	}

	// Check global locations from subsystem
	TArray<FMapLocationData> GlobalLocations = MapSubsystem->GetAllLocations();
	for (const FMapLocationData& Location : GlobalLocations)
	{
		if (!Location.bDiscovered && !IsLocationDiscovered(Location.WorldPosition))
		{
			float DiscoveryDistance = ExplorationRadius * 0.5f;
			if (FVector::Dist(PlayerPosition, Location.WorldPosition) < DiscoveryDistance)
			{
				DiscoverLocation(Location);
			}
		}
	}
}

void UHarmoniaMapComponent::UpdatePings(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Remove expired pings
	for (int32 i = ActivePings.Num() - 1; i >= 0; --i)
	{
		if (ActivePings[i].IsExpired(CurrentTime))
		{
			ActivePings.RemoveAt(i);
		}
	}
}

void UHarmoniaMapComponent::ServerCreatePing_Implementation(const FVector& WorldLocation, float Lifetime, FGameplayTag PingTag)
{
	CreatePing(WorldLocation, Lifetime, PingTag);
}

bool UHarmoniaMapComponent::ServerCreatePing_Validate(const FVector& WorldLocation, float Lifetime, FGameplayTag PingTag)
{
	// Anti-cheat: Validate lifetime
	if (Lifetime < 0.0f || Lifetime > 60.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerCreatePing: Invalid lifetime %.1f"), Lifetime);
		return false;
	}

	// Rate limiting to prevent ping spam
	const float MinTimeBetweenPings = 0.5f; // Minimum 0.5 seconds between pings
	const int32 MaxPingsPerMinute = 30; // Maximum 30 pings per minute

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Check time since last ping
	if (CurrentTime - LastPingTime < MinTimeBetweenPings)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerCreatePing: Ping spam detected (too frequent)"));
		return false;
	}

	// Check ping count in the last minute
	PingTimestamps.RemoveAll([CurrentTime](float Timestamp) {
		return (CurrentTime - Timestamp) > 60.0f; // Remove timestamps older than 1 minute
	});

	if (PingTimestamps.Num() >= MaxPingsPerMinute)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerCreatePing: Ping spam detected (too many pings)"));
		return false;
	}

	// Update rate limiting trackers
	LastPingTime = CurrentTime;
	PingTimestamps.Add(CurrentTime);

	return true;
}

void UHarmoniaMapComponent::ServerAddExploredRegion_Implementation(const FVector& Center, float Radius)
{
	AddExploredRegion(Center, Radius);
}

bool UHarmoniaMapComponent::ServerAddExploredRegion_Validate(const FVector& Center, float Radius)
{
	// Anti-cheat: Validate radius is reasonable
	if (Radius < 0.0f || Radius > 10000.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerAddExploredRegion: Invalid radius %.1f"), Radius);
		return false;
	}

	// Validate player is near the region center
	FVector PlayerPos = GetPlayerWorldPosition();
	float Distance = FVector::Dist(PlayerPos, Center);
	if (Distance > Radius * 2.0f) // Allow 2x buffer for network lag
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerAddExploredRegion: Too far from region (%.1f > %.1f)"), Distance, Radius * 2.0f);
		return false;
	}

	return true;
}

void UHarmoniaMapComponent::ServerDiscoverLocation_Implementation(const FMapLocationData& Location)
{
	DiscoverLocation(Location);
}

bool UHarmoniaMapComponent::ServerDiscoverLocation_Validate(const FMapLocationData& Location)
{
	// Anti-cheat: Validate player is near the location
	FVector PlayerPos = GetPlayerWorldPosition();
	float Distance = FVector::Dist(PlayerPos, Location.WorldPosition);

	const float DiscoveryDistance = ExplorationRadius * 0.5f;
	if (Distance > DiscoveryDistance * 2.0f) // Allow 2x buffer for network lag
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerDiscoverLocation: Too far from location (%.1f > %.1f)"),
			Distance, DiscoveryDistance * 2.0f);
		return false;
	}

	return true;
}

void UHarmoniaMapComponent::ServerAddBookmark_Implementation(const FMapBookmark& Bookmark)
{
	// Add directly to bookmarks array
	if (Bookmarks.Num() < 100) // Hard limit
	{
		Bookmarks.Add(Bookmark);
	}
}

bool UHarmoniaMapComponent::ServerAddBookmark_Validate(const FMapBookmark& Bookmark)
{
	// Basic validation
	if (Bookmark.BookmarkName.IsEmpty())
	{
		return false;
	}
	return true;
}

void UHarmoniaMapComponent::ServerRemoveBookmark_Implementation(int32 BookmarkIndex)
{
	RemoveBookmark(BookmarkIndex);
}

bool UHarmoniaMapComponent::ServerRemoveBookmark_Validate(int32 BookmarkIndex)
{
	return true;
}

void UHarmoniaMapComponent::MulticastOnPingCreated_Implementation(const FMapPingData& Ping)
{
	OnPingCreated.Broadcast(Ping);
}

UHarmoniaMapSubsystem* UHarmoniaMapComponent::GetMapSubsystem()
{
	if (!MapSubsystem)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameInstance* GameInstance = World->GetGameInstance();
			if (GameInstance)
			{
				MapSubsystem = GameInstance->GetSubsystem<UHarmoniaMapSubsystem>();
			}
		}
	}
	return MapSubsystem;
}

void UHarmoniaMapComponent::InitializeFogOfWarRenderer()
{
	if (!CurrentMapData || FogOfWarRenderer)
	{
		return;
	}

	// Create fog of war renderer
	FogOfWarRenderer = NewObject<UHarmoniaFogOfWarRenderer>(this);
	if (FogOfWarRenderer)
	{
		// Initialize with map bounds
		FogOfWarRenderer->Initialize(
			CurrentMapData->CapturedMapData.WorldBounds,
			FIntPoint(1024, 1024)
		);

		// Initial update with current explored regions
		FogOfWarRenderer->UpdateFogOfWar(ExploredRegions);

		UE_LOG(LogTemp, Log, TEXT("Fog of War renderer initialized"));
	}
}
