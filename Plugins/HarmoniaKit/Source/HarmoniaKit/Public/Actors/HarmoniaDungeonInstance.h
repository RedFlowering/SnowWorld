// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonInstance.generated.h"

class UDungeonDataAsset;

/**
 * ?˜ì „ ì²´í¬?¬ì¸??
 */
USTRUCT(BlueprintType)
struct FDungeonCheckpoint
{
	GENERATED_BODY()

	/** ì²´í¬?¬ì¸???„ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector Location;

	/** ì²´í¬?¬ì¸???Œì „ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FRotator Rotation;

	/** ì²´í¬?¬ì¸???´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FText CheckpointName;
};

/**
 * ?˜ì „ ?¤í° ?¬ì¸??
 */
USTRUCT(BlueprintType)
struct FDungeonSpawnPoint
{
	GENERATED_BODY()

	/** ?¤í° ?„ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FTransform SpawnTransform;

	/** ?¤í°??ëª¬ìŠ¤???´ë˜??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<AActor> MonsterClass;

	/** ?¤í° ?¨ì´ë¸?ë²ˆí˜¸ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 WaveNumber = 1;

	/** ?œì„±???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	bool bIsActive = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonInstanceStarted, AHarmoniaDungeonInstance*, Instance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonInstanceCompleted, AHarmoniaDungeonInstance*, Instance, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerExitedDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonWaveCompleted, int32, WaveNumber);

/**
 * ?˜ì „ ?¸ìŠ¤?´ìŠ¤ ?¡í„°
 * ?¤ì œ ?˜ì „ ?ˆë²¨??ë°°ì¹˜?˜ì–´ ?˜ì „??ê´€ë¦¬í•˜???¡í„°
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

	//~ ?¸ë¦¬ê²Œì´??
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

	//~ ?˜ì „ ?¤ì •

	/** ?˜ì „ ?°ì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	const UDungeonDataAsset* DungeonData;

	/** ?„ì¬ ?œì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** ?…ì¥ ì§€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform EntranceTransform;

	/** ì¶œêµ¬ ì§€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform ExitTransform;

	/** ì²´í¬?¬ì¸??ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonCheckpoint> Checkpoints;

	/** ?¤í° ?¬ì¸??ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonSpawnPoint> SpawnPoints;

	//~ ?˜ì „ ?œì–´

	/** ?˜ì „ ?œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** ?˜ì „ ì¢…ë£Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void EndDungeon(bool bSuccess);

	/** ?Œë ˆ?´ì–´ ?…ì¥ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerEnter(APlayerController* Player);

	/** ?Œë ˆ?´ì–´ ?´ì¥ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerExit(APlayerController* Player);

	/** ?„ì¬ ?¨ì´ë¸??œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartWave(int32 WaveNumber);

	/** ?¤ìŒ ?¨ì´ë¸Œë¡œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void AdvanceToNextWave();

	//~ ?íƒœ ì¡°íšŒ

	/** ?˜ì „ ì§„í–‰ ì¤‘ì¸ì§€ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsActive() const { return bIsActive; }

	/** ?„ì¬ ?¨ì´ë¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** ì°¸ê? ?Œë ˆ?´ì–´ ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetPlayerCount() const { return ActivePlayers.Num(); }

	/** ?ì¡´ ëª¬ìŠ¤????*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetAliveMonsterCount() const;

	//~ ëª¬ìŠ¤??ê´€ë¦?

	/** ëª¬ìŠ¤???¤í° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	AActor* SpawnMonster(const FDungeonSpawnPoint& SpawnPoint);

	/** ëª¬ìŠ¤???±ë¡ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void RegisterMonster(AActor* Monster);

	/** ëª¬ìŠ¤???œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void UnregisterMonster(AActor* Monster);

	/** ëª¨ë“  ëª¬ìŠ¤???œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ClearAllMonsters();

	//~ ë¬´í•œ ?˜ì „??

	/** ?¤ìŒ ì¸??ì„± (ë¬´í•œ ?˜ì „?? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void GenerateNextFloor();

	/** ?„ì¬ ì¸??•ë³´ ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	FInfiniteDungeonFloor GetCurrentFloorInfo() const;

protected:
	/** ?˜ì „ ?œì„±???íƒœ */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	bool bIsActive;

	/** ?„ì¬ ?¨ì´ë¸?*/
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentWave;

	/** ?„ì¬ ì¸?(ë¬´í•œ ?˜ì „?? */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentFloor;

	/** ì°¸ê? ?Œë ˆ?´ì–´ ëª©ë¡ */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<APlayerController*> ActivePlayers;

	/** ?¤í°??ëª¬ìŠ¤??ëª©ë¡ */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<AActor*> SpawnedMonsters;

	/** ?¸ìŠ¤?´ìŠ¤ ID */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	FGuid InstanceID;

private:
	/** ëª¬ìŠ¤???¤í° ì²˜ë¦¬ */
	void ProcessMonsterSpawns(int32 WaveNumber);

	/** ?¨ì´ë¸??„ë£Œ ì²´í¬ */
	void CheckWaveCompletion();

	/** ?„ë£Œ ì¡°ê±´ ì²´í¬ */
	void CheckCompletionConditions();

	/** ?€?´ë¨¸ ?¸ë“¤ */
	FTimerHandle WaveCheckTimerHandle;
};
