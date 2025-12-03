// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonManager.generated.h"

class UDungeonDataAsset;

/**
 * @struct FDungeonInstanceInfo
 * @brief Runtime dungeon instance information
 */
USTRUCT(BlueprintType)
struct FDungeonInstanceInfo
{
	GENERATED_BODY()

	/** Instance unique ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid InstanceID;

	/** Dungeon data asset */
	UPROPERTY(BlueprintReadOnly)
	const UDungeonDataAsset* DungeonData = nullptr;

	/** Difficulty level */
	UPROPERTY(BlueprintReadOnly)
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** Creation timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreationTime;

	/** List of participating player IDs */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PlayerIDs;

	/** Instance state */
	UPROPERTY(BlueprintReadOnly)
	EDungeonState State = EDungeonState::NotStarted;
};

/**
 * @struct FDungeonRankingArray
 * @brief Dungeon ranking array wrapper (for use as TMap value)
 */
USTRUCT(BlueprintType)
struct FDungeonRankingArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FDungeonRankingEntry> Rankings;
};

/**
 * @class UHarmoniaDungeonManager
 * @brief Dungeon manager subsystem for game-wide dungeon system management
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDungeonManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ Dungeon Data Management

	/** Register dungeon data */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void RegisterDungeon(UDungeonDataAsset* DungeonData);

	/** Get dungeon data by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	UDungeonDataAsset* GetDungeonData(FName DungeonID) const;

	/** Get all registered dungeons */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetAllDungeons() const;

	/** Get dungeons by type */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetDungeonsByType(EDungeonType DungeonType) const;

	//~ Instance Management

	/** Create dungeon instance */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	FGuid CreateDungeonInstance(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** Destroy dungeon instance */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DestroyDungeonInstance(FGuid InstanceID);

	/** Get instance info */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	FDungeonInstanceInfo GetInstanceInfo(FGuid InstanceID) const;

	/** Get active instance list */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonInstanceInfo> GetActiveInstances() const;

	/** Add player to instance */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool AddPlayerToInstance(FGuid InstanceID, const FString& PlayerID);

	/** Remove player from instance */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool RemovePlayerFromInstance(FGuid InstanceID, const FString& PlayerID);

	//~ Matchmaking

	/** Start dungeon matchmaking */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void StartMatchmaking(const FString& PlayerID, FName DungeonID, EDungeonDifficulty Difficulty);

	/** Cancel matchmaking */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void CancelMatchmaking(const FString& PlayerID);

	//~ Global Rankings

	/** Get global rankings */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetGlobalRankings(FName DungeonID, int32 TopCount = 100) const;

	/** Submit ranking entry */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void SubmitGlobalRanking(FName DungeonID, const FDungeonRankingEntry& Entry);

	/** Get season rankings */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetSeasonRankings(FName DungeonID, int32 SeasonID, int32 TopCount = 100) const;

	//~ Event Dungeons

	/** Activate event dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void ActivateEventDungeon(FName DungeonID, FDateTime StartTime, FDateTime EndTime);

	/** Deactivate event dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DeactivateEventDungeon(FName DungeonID);

	/** Get active event dungeons list */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FName> GetActiveEventDungeons() const;

	//~ Statistics

	/** Get dungeon clear count */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	int32 GetDungeonClearCount(FName DungeonID) const;

	/** Get average clear time */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetAverageClearTime(FName DungeonID) const;

	/** Get clear rate */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetClearRate(FName DungeonID) const;

protected:
	/** Registered dungeons map */
	UPROPERTY()
	TMap<FName, UDungeonDataAsset*> RegisteredDungeons;

	/** Active instances map */
	UPROPERTY()
	TMap<FGuid, FDungeonInstanceInfo> ActiveInstances;

	/** Global rankings by dungeon */
	UPROPERTY()
	TMap<FName, FDungeonRankingArray> GlobalRankings;

	/** Matchmaking queue */
	UPROPERTY()
	TMap<FString, FGuid> MatchmakingQueue;

	/** Active event dungeons */
	UPROPERTY()
	TMap<FName, FDateTime> ActiveEventDungeons;

	/** Dungeon statistics */
	UPROPERTY()
	TMap<FName, int32> DungeonClearCounts;

	UPROPERTY()
	TMap<FName, float> TotalClearTimes;

	UPROPERTY()
	TMap<FName, int32> DungeonAttemptCounts;

private:
	/** Cleanup expired instances */
	void CleanupExpiredInstances();

	/** Process matchmaking queue */
	void ProcessMatchmaking();
};
