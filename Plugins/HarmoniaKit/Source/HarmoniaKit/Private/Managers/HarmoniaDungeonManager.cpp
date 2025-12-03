// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#include "Managers/HarmoniaDungeonManager.h"
#include "Engine/World.h"

void UHarmoniaDungeonManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("HarmoniaDungeonManager initialized"));
}

void UHarmoniaDungeonManager::Deinitialize()
{
	// Clean up all instances
	ActiveInstances.Empty();
	MatchmakingQueue.Empty();

	Super::Deinitialize();
}

void UHarmoniaDungeonManager::RegisterDungeon(UDungeonDataAsset* DungeonData)
{
	if (!DungeonData)
	{
		return;
	}

	RegisteredDungeons.Add(DungeonData->DungeonID, DungeonData);
	UE_LOG(LogTemp, Log, TEXT("Dungeon registered: %s"), *DungeonData->DungeonID.ToString());
}

UDungeonDataAsset* UHarmoniaDungeonManager::GetDungeonData(FName DungeonID) const
{
	if (const UDungeonDataAsset* const* FoundDungeon = RegisteredDungeons.Find(DungeonID))
	{
		return const_cast<UDungeonDataAsset*>(*FoundDungeon);
	}

	return nullptr;
}

TArray<UDungeonDataAsset*> UHarmoniaDungeonManager::GetAllDungeons() const
{
	TArray<UDungeonDataAsset*> Result;
	RegisteredDungeons.GenerateValueArray(Result);
	return Result;
}

TArray<UDungeonDataAsset*> UHarmoniaDungeonManager::GetDungeonsByType(EDungeonType DungeonType) const
{
	TArray<UDungeonDataAsset*> Result;

	for (const auto& Pair : RegisteredDungeons)
	{
		if (Pair.Value && Pair.Value->DungeonType == DungeonType)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

FGuid UHarmoniaDungeonManager::CreateDungeonInstance(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty)
{
	if (!DungeonData)
	{
		return FGuid();
	}

	FDungeonInstanceInfo NewInstance;
	NewInstance.InstanceID = FGuid::NewGuid();
	NewInstance.DungeonData = DungeonData;
	NewInstance.Difficulty = Difficulty;
	NewInstance.CreationTime = FDateTime::Now();
	NewInstance.State = EDungeonState::NotStarted;

	ActiveInstances.Add(NewInstance.InstanceID, NewInstance);

	UE_LOG(LogTemp, Log, TEXT("Dungeon instance created: %s (ID: %s)"), 
		*DungeonData->DungeonID.ToString(), *NewInstance.InstanceID.ToString());

	return NewInstance.InstanceID;
}

void UHarmoniaDungeonManager::DestroyDungeonInstance(FGuid InstanceID)
{
	if (ActiveInstances.Remove(InstanceID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Dungeon instance destroyed: %s"), *InstanceID.ToString());
	}
}

FDungeonInstanceInfo UHarmoniaDungeonManager::GetInstanceInfo(FGuid InstanceID) const
{
	if (const FDungeonInstanceInfo* Found = ActiveInstances.Find(InstanceID))
	{
		return *Found;
	}

	return FDungeonInstanceInfo();
}

TArray<FDungeonInstanceInfo> UHarmoniaDungeonManager::GetActiveInstances() const
{
	TArray<FDungeonInstanceInfo> Result;
	ActiveInstances.GenerateValueArray(Result);
	return Result;
}

bool UHarmoniaDungeonManager::AddPlayerToInstance(FGuid InstanceID, const FString& PlayerID)
{
	if (FDungeonInstanceInfo* Instance = ActiveInstances.Find(InstanceID))
	{
		if (!Instance->PlayerIDs.Contains(PlayerID))
		{
			Instance->PlayerIDs.Add(PlayerID);
			return true;
		}
	}

	return false;
}

bool UHarmoniaDungeonManager::RemovePlayerFromInstance(FGuid InstanceID, const FString& PlayerID)
{
	if (FDungeonInstanceInfo* Instance = ActiveInstances.Find(InstanceID))
	{
		return Instance->PlayerIDs.Remove(PlayerID) > 0;
	}

	return false;
}

void UHarmoniaDungeonManager::StartMatchmaking(const FString& PlayerID, FName DungeonID, EDungeonDifficulty Difficulty)
{
	UDungeonDataAsset* DungeonData = GetDungeonData(DungeonID);
	if (!DungeonData)
	{
		return;
	}

	// Simple matchmaking: create instance immediately
	FGuid InstanceID = CreateDungeonInstance(DungeonData, Difficulty);
	AddPlayerToInstance(InstanceID, PlayerID);
	MatchmakingQueue.Add(PlayerID, InstanceID);

	// In production, add to queue and process matching periodically
}

void UHarmoniaDungeonManager::CancelMatchmaking(const FString& PlayerID)
{
	if (FGuid* InstanceID = MatchmakingQueue.Find(PlayerID))
	{
		RemovePlayerFromInstance(*InstanceID, PlayerID);
		MatchmakingQueue.Remove(PlayerID);
	}
}

TArray<FDungeonRankingEntry> UHarmoniaDungeonManager::GetGlobalRankings(FName DungeonID, int32 TopCount) const
{
	TArray<FDungeonRankingEntry> Result;

	if (const FDungeonRankingArray* RankingArray = GlobalRankings.Find(DungeonID))
	{
		const TArray<FDungeonRankingEntry>& Rankings = RankingArray->Rankings;
		int32 Count = FMath::Min(TopCount, Rankings.Num());
		for (int32 i = 0; i < Count; i++)
		{
			Result.Add(Rankings[i]);
		}
	}

	return Result;
}

void UHarmoniaDungeonManager::SubmitGlobalRanking(FName DungeonID, const FDungeonRankingEntry& Entry)
{
	FDungeonRankingArray& RankingArray = GlobalRankings.FindOrAdd(DungeonID);
	RankingArray.Rankings.Add(Entry);

	// Sort by score (descending)
	RankingArray.Rankings.Sort([](const FDungeonRankingEntry& A, const FDungeonRankingEntry& B)
	{
		return A.Score > B.Score;
	});

	// Keep only top 1000
	if (RankingArray.Rankings.Num() > 1000)
	{
		RankingArray.Rankings.SetNum(1000);
	}

	// Update statistics
	int32& ClearCount = DungeonClearCounts.FindOrAdd(DungeonID);
	ClearCount++;

	float& TotalTime = TotalClearTimes.FindOrAdd(DungeonID);
	TotalTime += Entry.ClearTime;
}

TArray<FDungeonRankingEntry> UHarmoniaDungeonManager::GetSeasonRankings(FName DungeonID, int32 SeasonID, int32 TopCount) const
{
	TArray<FDungeonRankingEntry> Result;

	if (const FDungeonRankingArray* RankingArray = GlobalRankings.Find(DungeonID))
	{
		// Filter by season
		for (const FDungeonRankingEntry& Entry : RankingArray->Rankings)
		{
			if (Entry.SeasonID == SeasonID)
			{
				Result.Add(Entry);
				if (Result.Num() >= TopCount)
				{
					break;
				}
			}
		}
	}

	return Result;
}

void UHarmoniaDungeonManager::ActivateEventDungeon(FName DungeonID, FDateTime StartTime, FDateTime EndTime)
{
	ActiveEventDungeons.Add(DungeonID, EndTime);
	UE_LOG(LogTemp, Log, TEXT("Event dungeon activated: %s"), *DungeonID.ToString());
}

void UHarmoniaDungeonManager::DeactivateEventDungeon(FName DungeonID)
{
	ActiveEventDungeons.Remove(DungeonID);
	UE_LOG(LogTemp, Log, TEXT("Event dungeon deactivated: %s"), *DungeonID.ToString());
}

TArray<FName> UHarmoniaDungeonManager::GetActiveEventDungeons() const
{
	TArray<FName> Result;
	FDateTime Now = FDateTime::Now();

	for (const auto& Pair : ActiveEventDungeons)
	{
		if (Pair.Value > Now)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

int32 UHarmoniaDungeonManager::GetDungeonClearCount(FName DungeonID) const
{
	if (const int32* Count = DungeonClearCounts.Find(DungeonID))
	{
		return *Count;
	}

	return 0;
}

float UHarmoniaDungeonManager::GetAverageClearTime(FName DungeonID) const
{
	const int32 ClearCount = GetDungeonClearCount(DungeonID);
	if (ClearCount == 0)
	{
		return 0.0f;
	}

	if (const float* TotalTime = TotalClearTimes.Find(DungeonID))
	{
		return *TotalTime / ClearCount;
	}

	return 0.0f;
}

float UHarmoniaDungeonManager::GetClearRate(FName DungeonID) const
{
	const int32 ClearCount = GetDungeonClearCount(DungeonID);
	
	if (const int32* AttemptCount = DungeonAttemptCounts.Find(DungeonID))
	{
		if (*AttemptCount > 0)
		{
			return (float)ClearCount / (float)(*AttemptCount);
		}
	}

	return 0.0f;
}

void UHarmoniaDungeonManager::CleanupExpiredInstances()
{
	FDateTime Now = FDateTime::Now();
	TArray<FGuid> ToRemove;

	for (const auto& Pair : ActiveInstances)
	{
		// Remove instances older than 24 hours
		FTimespan Elapsed = Now - Pair.Value.CreationTime;
		if (Elapsed.GetTotalHours() > 24.0)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FGuid& ID : ToRemove)
	{
		DestroyDungeonInstance(ID);
	}
}

void UHarmoniaDungeonManager::ProcessMatchmaking()
{
	// Implement matchmaking logic
	// Group players requesting the same dungeon and difficulty to create instances
}
