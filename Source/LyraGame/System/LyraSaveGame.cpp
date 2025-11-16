// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSaveGame.h"
#include "Misc/DateTime.h"

ULyraSaveGame::ULyraSaveGame()
	: SaveSlotName(TEXT("DefaultSave"))
	, SaveVersion(1)
	, TotalPlayTime(0.0f)
{
	LastSaveTimestamp = FDateTime::Now();
}

void ULyraSaveGame::SetPlayerData(const FString& InSteamID, const FLyraPlayerSaveData& InPlayerData)
{
	PlayerDataMap.Add(InSteamID, InPlayerData);
}

bool ULyraSaveGame::GetPlayerData(const FString& InSteamID, FLyraPlayerSaveData& OutPlayerData) const
{
	if (const FLyraPlayerSaveData* FoundData = PlayerDataMap.Find(InSteamID))
	{
		OutPlayerData = *FoundData;
		return true;
	}
	return false;
}

bool ULyraSaveGame::HasPlayerData(const FString& InSteamID) const
{
	return PlayerDataMap.Contains(InSteamID);
}

void ULyraSaveGame::AddBuilding(const FLyraSavedBuildingInstance& Building)
{
	WorldData.PlacedBuildings.Add(Building);
}

void ULyraSaveGame::ClearBuildings()
{
	WorldData.PlacedBuildings.Empty();
}
