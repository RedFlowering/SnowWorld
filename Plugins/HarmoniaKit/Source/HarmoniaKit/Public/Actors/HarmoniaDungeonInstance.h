// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonInstance.generated.h"

class UDungeonDataAsset;

/**
 * ?�전 체크?�인??
 */
USTRUCT(BlueprintType)
struct FDungeonCheckpoint
{
	GENERATED_BODY()

	/** 체크?�인???�치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector Location;

	/** 체크?�인???�전 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FRotator Rotation;

	/** 체크?�인???�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FText CheckpointName;
};

/**
 * ?�전 ?�폰 ?�인??
 */
USTRUCT(BlueprintType)
struct FDungeonSpawnPoint
{
	GENERATED_BODY()

	/** ?�폰 ?�치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FTransform SpawnTransform;

	/** ?�폰??몬스???�래??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<AActor> MonsterClass;

	/** ?�폰 ?�이�?번호 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 WaveNumber = 1;

	/** ?�성???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	bool bIsActive = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonInstanceStarted, AHarmoniaDungeonInstance*, Instance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonInstanceCompleted, AHarmoniaDungeonInstance*, Instance, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerExitedDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonWaveCompleted, int32, WaveNumber);

/**
 * ?�전 ?�스?�스 ?�터
 * ?�제 ?�전 ?�벨??배치?�어 ?�전??관리하???�터
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

	//~ ?�리게이??
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

	//~ ?�전 ?�정

	/** ?�전 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	const UDungeonDataAsset* DungeonData;

	/** ?�재 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** ?�장 지??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform EntranceTransform;

	/** 출구 지??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform ExitTransform;

	/** 체크?�인??목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonCheckpoint> Checkpoints;

	/** ?�폰 ?�인??목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonSpawnPoint> SpawnPoints;

	//~ ?�전 ?�어

	/** ?�전 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** ?�전 종료 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void EndDungeon(bool bSuccess);

	/** ?�레?�어 ?�장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerEnter(APlayerController* Player);

	/** ?�레?�어 ?�장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerExit(APlayerController* Player);

	/** ?�재 ?�이�??�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartWave(int32 WaveNumber);

	/** ?�음 ?�이브로 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void AdvanceToNextWave();

	//~ ?�태 조회

	/** ?�전 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsActive() const { return bIsActive; }

	/** ?�재 ?�이�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** 참�? ?�레?�어 ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetPlayerCount() const { return ActivePlayers.Num(); }

	/** ?�존 몬스????*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetAliveMonsterCount() const;

	//~ 몬스??관�?

	/** 몬스???�폰 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	AActor* SpawnMonster(const FDungeonSpawnPoint& SpawnPoint);

	/** 몬스???�록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void RegisterMonster(AActor* Monster);

	/** 몬스???�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void UnregisterMonster(AActor* Monster);

	/** 모든 몬스???�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ClearAllMonsters();

	//~ 무한 ?�전??

	/** ?�음 �??�성 (무한 ?�전?? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void GenerateNextFloor();

	/** ?�재 �??�보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	FInfiniteDungeonFloor GetCurrentFloorInfo() const;

protected:
	/** ?�전 ?�성???�태 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	bool bIsActive;

	/** ?�재 ?�이�?*/
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentWave;

	/** ?�재 �?(무한 ?�전?? */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentFloor;

	/** 참�? ?�레?�어 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<APlayerController*> ActivePlayers;

	/** ?�폰??몬스??목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<AActor*> SpawnedMonsters;

	/** ?�스?�스 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	FGuid InstanceID;

private:
	/** 몬스???�폰 처리 */
	void ProcessMonsterSpawns(int32 WaveNumber);

	/** ?�이�??�료 체크 */
	void CheckWaveCompletion();

	/** ?�료 조건 체크 */
	void CheckCompletionConditions();

	/** ?�?�머 ?�들 */
	FTimerHandle WaveCheckTimerHandle;
};
