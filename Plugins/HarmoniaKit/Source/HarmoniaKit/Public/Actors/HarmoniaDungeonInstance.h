// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonInstance.generated.h"

class UDungeonDataAsset;

/**
 * @struct FDungeonCheckpoint
 * @brief Dungeon checkpoint data
 */
USTRUCT(BlueprintType)
struct FDungeonCheckpoint
{
	GENERATED_BODY()

	/** Checkpoint location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector Location;

	/** Checkpoint rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FRotator Rotation;

	/** Checkpoint name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FText CheckpointName;
};

/**
 * @struct FDungeonSpawnPoint
 * @brief Dungeon spawn point data
 */
USTRUCT(BlueprintType)
struct FDungeonSpawnPoint
{
	GENERATED_BODY()

	/** Spawn transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FTransform SpawnTransform;

	/** Monster class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<AActor> MonsterClass;

	/** Wave number for this spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 WaveNumber = 1;

	/** Whether spawn is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	bool bIsActive = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonInstanceStarted, AHarmoniaDungeonInstance*, Instance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonInstanceCompleted, AHarmoniaDungeonInstance*, Instance, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerExitedDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonWaveCompleted, int32, WaveNumber);

/**
 * @class AHarmoniaDungeonInstance
 * @brief Dungeon instance actor that manages actual dungeon gameplay
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaDungeonInstance : public AActor
{
	GENERATED_BODY()
	
public:	
	AHarmoniaDungeonInstance();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	//~ Delegates
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonInstanceStarted OnDungeonInstanceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonInstanceCompleted OnDungeonInstanceCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnPlayerEnteredDungeon OnPlayerEnteredDungeon;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnPlayerExitedDungeon OnPlayerExitedDungeon;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonWaveCompleted OnWaveCompleted;

	//~ Dungeon Settings

	/** Dungeon data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	const UDungeonDataAsset* DungeonData;

	/** Current difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** Entrance location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform EntranceTransform;

	/** Exit location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform ExitTransform;

	/** Checkpoint list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonCheckpoint> Checkpoints;

	/** Spawn point list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonSpawnPoint> SpawnPoints;

	//~ Dungeon Control

	/** Start the dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** End the dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void EndDungeon(bool bSuccess);

	/** Handle player entering */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerEnter(APlayerController* Player);

	/** Handle player exiting */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerExit(APlayerController* Player);

	/** Start a specific wave */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartWave(int32 WaveNumber);

	/** Advance to next wave */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void AdvanceToNextWave();

	//~ State Queries

	/** Check if dungeon is active */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsActive() const { return bIsActive; }

	/** Get current wave number */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** Get participating player count */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetPlayerCount() const { return ActivePlayers.Num(); }

	/** Get alive monster count */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetAliveMonsterCount() const;

	//~ Monster Management

	/** Spawn a monster */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	AActor* SpawnMonster(const FDungeonSpawnPoint& SpawnPoint);

	/** Register a monster */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void RegisterMonster(AActor* Monster);

	/** Unregister a monster */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void UnregisterMonster(AActor* Monster);

	/** Clear all monsters */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ClearAllMonsters();

	//~ Infinite Dungeon Mode

	/** Generate next floor (for infinite dungeon) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void GenerateNextFloor();

	/** Get current floor info */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	FInfiniteDungeonFloor GetCurrentFloorInfo() const;

protected:
	/** Whether dungeon is active */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	bool bIsActive;

	/** Current wave number */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentWave;

	/** Current floor (for infinite dungeon) */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentFloor;

	/** Active players list */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<APlayerController*> ActivePlayers;

	/** Spawned monsters list */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<AActor*> SpawnedMonsters;

	/** Instance unique ID */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	FGuid InstanceID;

private:
	/** Process monster spawns for wave */
	void ProcessMonsterSpawns(int32 WaveNumber);

	/** Check wave completion */
	void CheckWaveCompletion();

	/** Check completion conditions */
	void CheckCompletionConditions();

	/** Timer handle for wave checks */
	FTimerHandle WaveCheckTimerHandle;
};
