// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaTerrainOwnershipManager.h"
#include "GameFramework/PlayerState.h"
#include "Teams/LyraTeamAgentInterface.h"

void UHarmoniaTerrainOwnershipManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("UHarmoniaTerrainOwnershipManager initialized"));
}

void UHarmoniaTerrainOwnershipManager::Deinitialize()
{
	OwnershipZones.Empty();

	Super::Deinitialize();
}

// ============================================================================
// Zone Management
// ============================================================================

int32 UHarmoniaTerrainOwnershipManager::RegisterOwnershipZone(const FTerrainOwnershipZone& Zone)
{
	int32 ZoneIndex = OwnershipZones.Add(Zone);

	UE_LOG(LogTemp, Log, TEXT("Registered ownership zone '%s' at index %d (Center: %s, Radius: %.1f)"),
		*Zone.ZoneName, ZoneIndex, *Zone.Center.ToString(), Zone.Radius);

	return ZoneIndex;
}

void UHarmoniaTerrainOwnershipManager::UnregisterOwnershipZone(int32 ZoneIndex)
{
	if (OwnershipZones.IsValidIndex(ZoneIndex))
	{
		FTerrainOwnershipZone& Zone = OwnershipZones[ZoneIndex];
		UE_LOG(LogTemp, Log, TEXT("Unregistered ownership zone '%s' at index %d"), *Zone.ZoneName, ZoneIndex);

		OwnershipZones.RemoveAt(ZoneIndex);
	}
}

const FTerrainOwnershipZone* UHarmoniaTerrainOwnershipManager::GetZoneAtLocation(const FVector& Location) const
{
	// Return the smallest zone that contains the location (most specific)
	const FTerrainOwnershipZone* BestZone = nullptr;
	float SmallestRadius = FLT_MAX;

	for (const FTerrainOwnershipZone& Zone : OwnershipZones)
	{
		float Distance = FVector::Dist(Zone.Center, Location);
		if (Distance <= Zone.Radius && Zone.Radius < SmallestRadius)
		{
			BestZone = &Zone;
			SmallestRadius = Zone.Radius;
		}
	}

	return BestZone;
}

TArray<int32> UHarmoniaTerrainOwnershipManager::GetZonesOwnedByPlayer(const FString& PlayerID) const
{
	TArray<int32> Result;

	for (int32 i = 0; i < OwnershipZones.Num(); ++i)
	{
		if (OwnershipZones[i].OwnerPlayerID == PlayerID)
		{
			Result.Add(i);
		}
	}

	return Result;
}

TArray<int32> UHarmoniaTerrainOwnershipManager::GetZonesOwnedByTeam(int32 TeamID) const
{
	TArray<int32> Result;

	if (TeamID == INDEX_NONE)
	{
		return Result;
	}

	for (int32 i = 0; i < OwnershipZones.Num(); ++i)
	{
		if (OwnershipZones[i].OwnerTeamID == TeamID)
		{
			Result.Add(i);
		}
	}

	return Result;
}

// ============================================================================
// Permission Checks
// ============================================================================

bool UHarmoniaTerrainOwnershipManager::HasBuildingPermission(AActor* Actor, const FVector& Location) const
{
	if (!Actor)
	{
		return false;
	}

	FString PlayerID = GetPlayerIDFromActor(Actor);
	int32 TeamID = GetTeamIDFromActor(Actor);

	return HasBuildingPermissionByID(PlayerID, TeamID, Location);
}

bool UHarmoniaTerrainOwnershipManager::HasBuildingPermissionByID(const FString& PlayerID, int32 TeamID, const FVector& Location) const
{
	const FTerrainOwnershipZone* Zone = GetZoneAtLocation(Location);

	// No zone found - check if building is allowed in unclaimed areas
	if (!Zone)
	{
		return bAllowBuildingInUnclaimedAreas;
	}

	// Protected zone - no building allowed
	if (Zone->bProtectedZone)
	{
		return false;
	}

	// Public building zone - anyone can build
	if (Zone->bAllowPublicBuilding)
	{
		return true;
	}

	// Check player ownership
	if (!Zone->OwnerPlayerID.IsEmpty() && Zone->OwnerPlayerID == PlayerID)
	{
		return true;
	}

	// Check team ownership
	if (Zone->OwnerTeamID != INDEX_NONE && Zone->OwnerTeamID == TeamID)
	{
		return true;
	}

	// Not owned by this player or team
	return false;
}

bool UHarmoniaTerrainOwnershipManager::IsProtectedZone(const FVector& Location) const
{
	const FTerrainOwnershipZone* Zone = GetZoneAtLocation(Location);
	return Zone && Zone->bProtectedZone;
}

bool UHarmoniaTerrainOwnershipManager::IsPublicBuildingArea(const FVector& Location) const
{
	const FTerrainOwnershipZone* Zone = GetZoneAtLocation(Location);
	return Zone && Zone->bAllowPublicBuilding;
}

// ============================================================================
// Zone Creation Helpers
// ============================================================================

int32 UHarmoniaTerrainOwnershipManager::CreateProtectedZone(const FVector& Center, float Radius, const FString& ZoneName)
{
	FTerrainOwnershipZone Zone;
	Zone.Center = Center;
	Zone.Radius = Radius;
	Zone.ZoneName = ZoneName;
	Zone.bProtectedZone = true;
	Zone.bAllowPublicBuilding = false;

	return RegisterOwnershipZone(Zone);
}

int32 UHarmoniaTerrainOwnershipManager::CreatePublicBuildingZone(const FVector& Center, float Radius, const FString& ZoneName)
{
	FTerrainOwnershipZone Zone;
	Zone.Center = Center;
	Zone.Radius = Radius;
	Zone.ZoneName = ZoneName;
	Zone.bProtectedZone = false;
	Zone.bAllowPublicBuilding = true;

	return RegisterOwnershipZone(Zone);
}

int32 UHarmoniaTerrainOwnershipManager::CreatePlayerZone(const FVector& Center, float Radius, const FString& PlayerID, const FString& ZoneName)
{
	FTerrainOwnershipZone Zone;
	Zone.Center = Center;
	Zone.Radius = Radius;
	Zone.OwnerPlayerID = PlayerID;
	Zone.OwnerTeamID = INDEX_NONE;
	Zone.ZoneName = ZoneName;
	Zone.bProtectedZone = false;
	Zone.bAllowPublicBuilding = false;

	return RegisterOwnershipZone(Zone);
}

int32 UHarmoniaTerrainOwnershipManager::CreateTeamZone(const FVector& Center, float Radius, int32 TeamID, const FString& ZoneName)
{
	FTerrainOwnershipZone Zone;
	Zone.Center = Center;
	Zone.Radius = Radius;
	Zone.OwnerPlayerID = FString();
	Zone.OwnerTeamID = TeamID;
	Zone.ZoneName = ZoneName;
	Zone.bProtectedZone = false;
	Zone.bAllowPublicBuilding = false;

	return RegisterOwnershipZone(Zone);
}

// ============================================================================
// Configuration
// ============================================================================

void UHarmoniaTerrainOwnershipManager::SetAllowBuildingInUnclaimedAreas(bool bAllow)
{
	bAllowBuildingInUnclaimedAreas = bAllow;

	UE_LOG(LogTemp, Log, TEXT("Building in unclaimed areas: %s"), bAllow ? TEXT("Enabled") : TEXT("Disabled"));
}

// ============================================================================
// Internal Helpers
// ============================================================================

FString UHarmoniaTerrainOwnershipManager::GetPlayerIDFromActor(AActor* Actor) const
{
	if (!Actor)
	{
		return FString();
	}

	// Try to get PlayerState
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn)
	{
		APlayerState* PlayerState = Pawn->GetPlayerState();
		if (PlayerState)
		{
			return PlayerState->GetUniqueId().ToString();
		}
	}

	// Try to get from controller
	AController* Controller = Cast<AController>(Actor);
	if (Controller)
	{
		APlayerState* PlayerState = Controller->PlayerState;
		if (PlayerState)
		{
			return PlayerState->GetUniqueId().ToString();
		}
	}

	return FString();
}

int32 UHarmoniaTerrainOwnershipManager::GetTeamIDFromActor(AActor* Actor) const
{
	if (!Actor)
	{
		return INDEX_NONE;
	}

	// Check if actor implements ILyraTeamAgentInterface
	if (ILyraTeamAgentInterface* TeamAgent = Cast<ILyraTeamAgentInterface>(Actor))
	{
		FGenericTeamId TeamId = TeamAgent->GetGenericTeamId();
		return GenericTeamIdToInteger(TeamId);
	}

	// Try to get from pawn's controller
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn && Pawn->GetController())
	{
		if (ILyraTeamAgentInterface* TeamAgent = Cast<ILyraTeamAgentInterface>(Pawn->GetController()))
		{
			FGenericTeamId TeamId = TeamAgent->GetGenericTeamId();
			return GenericTeamIdToInteger(TeamId);
		}
	}

	return INDEX_NONE;
}
