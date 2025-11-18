// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/HarmoniaMapSubsystem.h"

void UHarmoniaMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaMapSubsystem initialized"));
}

void UHarmoniaMapSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaMapSubsystem deinitialized"));
}

void UHarmoniaMapSubsystem::RegisterMapData(UHarmoniaMapDataAsset* MapData)
{
	if (MapData && !RegisteredMaps.Contains(MapData))
	{
		RegisteredMaps.Add(MapData);
		UE_LOG(LogTemp, Log, TEXT("Registered map data: %s"), *MapData->GetName());

		// Set as current if no current map
		if (!CurrentMapData)
		{
			SetCurrentMapData(MapData);
		}
	}
}

void UHarmoniaMapSubsystem::SetCurrentMapData(UHarmoniaMapDataAsset* MapData)
{
	if (MapData)
	{
		CurrentMapData = MapData;
		UE_LOG(LogTemp, Log, TEXT("Set current map data: %s"), *MapData->GetName());
	}
}

void UHarmoniaMapSubsystem::AddLocation(const FMapLocationData& Location)
{
	// Check if location already exists at this position
	for (FMapLocationData& ExistingLocation : GlobalLocations)
	{
		if (FVector::Dist(ExistingLocation.WorldPosition, Location.WorldPosition) < 100.0f)
		{
			// Update existing location
			ExistingLocation = Location;
			return;
		}
	}

	// Add new location
	GlobalLocations.Add(Location);
	UE_LOG(LogTemp, Log, TEXT("Added location: %s at %s"), *Location.LocationName.ToString(), *Location.WorldPosition.ToString());
}

void UHarmoniaMapSubsystem::RemoveLocation(const FVector& WorldPosition, float Threshold)
{
	for (int32 i = GlobalLocations.Num() - 1; i >= 0; --i)
	{
		if (FVector::Dist(GlobalLocations[i].WorldPosition, WorldPosition) < Threshold)
		{
			GlobalLocations.RemoveAt(i);
			return;
		}
	}
}

TArray<FMapLocationData> UHarmoniaMapSubsystem::GetLocationsInRadius(const FVector& Center, float Radius) const
{
	TArray<FMapLocationData> Result;

	for (const FMapLocationData& Location : GlobalLocations)
	{
		if (FVector::Dist(Location.WorldPosition, Center) <= Radius)
		{
			Result.Add(Location);
		}
	}

	return Result;
}

TArray<FMapLocationData> UHarmoniaMapSubsystem::GetLocationsByTag(FGameplayTag Tag) const
{
	TArray<FMapLocationData> Result;

	if (!Tag.IsValid())
	{
		return Result;
	}

	for (const FMapLocationData& Location : GlobalLocations)
	{
		if (Location.LocationTag.MatchesTagExact(Tag))
		{
			Result.Add(Location);
		}
	}

	return Result;
}

bool UHarmoniaMapSubsystem::FindLocationByName(const FText& Name, FMapLocationData& OutLocation) const
{
	for (const FMapLocationData& Location : GlobalLocations)
	{
		if (Location.LocationName.EqualTo(Name))
		{
			OutLocation = Location;
			return true;
		}
	}
	return false;
}

void UHarmoniaMapSubsystem::UpdateLocation(const FVector& WorldPosition, const FMapLocationData& NewData)
{
	for (FMapLocationData& Location : GlobalLocations)
	{
		if (FVector::Dist(Location.WorldPosition, WorldPosition) < 100.0f)
		{
			Location = NewData;
			return;
		}
	}
}

void UHarmoniaMapSubsystem::ClearAllLocations()
{
	GlobalLocations.Empty();
	UE_LOG(LogTemp, Log, TEXT("Cleared all locations"));
}
