// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaSaveGame.h"
#include "Misc/DateTime.h"

UHarmoniaSaveGame::UHarmoniaSaveGame()
	: SaveSlotName(TEXT("DefaultSave"))
	, SaveVersion(2)
	, TotalPlayTime(0.0f)
{
	LastSaveTimestamp = FDateTime::Now();
}

void UHarmoniaSaveGame::SetPlayerData(const FString& InSteamID, const FHarmoniaPlayerSaveData& InPlayerData)
{
	PlayerDataMap.Add(InSteamID, InPlayerData);
}

bool UHarmoniaSaveGame::GetPlayerData(const FString& InSteamID, FHarmoniaPlayerSaveData& OutPlayerData) const
{
	if (const FHarmoniaPlayerSaveData* FoundData = PlayerDataMap.Find(InSteamID))
	{
		OutPlayerData = *FoundData;
		return true;
	}
	return false;
}

bool UHarmoniaSaveGame::HasPlayerData(const FString& InSteamID) const
{
	return PlayerDataMap.Contains(InSteamID);
}

void UHarmoniaSaveGame::AddBuilding(const FHarmoniaSavedBuildingInstance& Building)
{
	WorldData.PlacedBuildings.Add(Building);
}

void UHarmoniaSaveGame::ClearBuildings()
{
	WorldData.PlacedBuildings.Empty();
}
