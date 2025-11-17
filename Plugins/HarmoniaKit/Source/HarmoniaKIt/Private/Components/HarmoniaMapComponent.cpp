// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaMapComponent.h"
#include "System/HarmoniaMapSubsystem.h"
#include "System/HarmoniaFogOfWarRenderer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
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

		// Initialize fog of war renderer (client only)
		if (GetOwnerRole() != ROLE_Authority || GetNetMode() == NM_Standalone)
		{
			InitializeFogOfWarRenderer();
		}
	}
}

void UHarmoniaMapComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaMapComponent, ExploredRegions);
	DOREPLIFETIME(UHarmoniaMapComponent, DiscoveredLocations);
	DOREPLIFETIME(UHarmoniaMapComponent, ActivePings);
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

			// Broadcast event
			MulticastOnRegionExplored(NewRegion);
		}

		// Update fog of war renderer
		if (FogOfWarRenderer)
		{
			FogOfWarRenderer->UpdateFogOfWar(ExploredRegions);
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

		// Broadcast event
		MulticastOnLocationDiscovered(NewLocation);
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
	if (ActivePings.IsValidIndex(PingIndex))
	{
		ActivePings.RemoveAt(PingIndex);
	}
}

void UHarmoniaMapComponent::ClearAllPings()
{
	ActivePings.Empty();
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
	// TODO: Implement save system integration
	// This would integrate with your game's save system
	UE_LOG(LogTemp, Log, TEXT("SaveExplorationData called - implement save system integration"));
}

void UHarmoniaMapComponent::LoadExplorationData()
{
	// TODO: Implement save system integration
	// This would load from your game's save system
	UE_LOG(LogTemp, Log, TEXT("LoadExplorationData called - implement save system integration"));
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

void UHarmoniaMapComponent::ServerAddExploredRegion_Implementation(const FVector& Center, float Radius)
{
	AddExploredRegion(Center, Radius);
}

void UHarmoniaMapComponent::ServerDiscoverLocation_Implementation(const FMapLocationData& Location)
{
	DiscoverLocation(Location);
}

void UHarmoniaMapComponent::MulticastOnPingCreated_Implementation(const FMapPingData& Ping)
{
	OnPingCreated.Broadcast(Ping);
}

void UHarmoniaMapComponent::MulticastOnLocationDiscovered_Implementation(const FMapLocationData& Location)
{
	OnLocationDiscovered.Broadcast(Location);
}

void UHarmoniaMapComponent::MulticastOnRegionExplored_Implementation(const FExploredRegion& Region)
{
	OnRegionExplored.Broadcast(Region);
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
