// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonInstance.generated.h"

class UDungeonDataAsset;

/**
 * 던전 체크포인트
 */
USTRUCT(BlueprintType)
struct FDungeonCheckpoint
{
	GENERATED_BODY()

	/** 체크포인트 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FVector Location;

	/** 체크포인트 회전 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FRotator Rotation;

	/** 체크포인트 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FText CheckpointName;
};

/**
 * 던전 스폰 포인트
 */
USTRUCT(BlueprintType)
struct FDungeonSpawnPoint
{
	GENERATED_BODY()

	/** 스폰 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	FTransform SpawnTransform;

	/** 스폰할 몬스터 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	TSubclassOf<AActor> MonsterClass;

	/** 스폰 웨이브 번호 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	int32 WaveNumber = 1;

	/** 활성화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon")
	bool bIsActive = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonInstanceStarted, AHarmoniaDungeonInstance*, Instance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonInstanceCompleted, AHarmoniaDungeonInstance*, Instance, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerExitedDungeon, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonWaveCompleted, int32, WaveNumber);

/**
 * 던전 인스턴스 액터
 * 실제 던전 레벨에 배치되어 던전을 관리하는 액터
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

	//~ 델리게이트
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

	//~ 던전 설정

	/** 던전 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	const UDungeonDataAsset* DungeonData;

	/** 현재 난이도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** 입장 지점 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform EntranceTransform;

	/** 출구 지점 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	FTransform ExitTransform;

	/** 체크포인트 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonCheckpoint> Checkpoints;

	/** 스폰 포인트 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon|Settings")
	TArray<FDungeonSpawnPoint> SpawnPoints;

	//~ 던전 제어

	/** 던전 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** 던전 종료 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void EndDungeon(bool bSuccess);

	/** 플레이어 입장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerEnter(APlayerController* Player);

	/** 플레이어 퇴장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void OnPlayerExit(APlayerController* Player);

	/** 현재 웨이브 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartWave(int32 WaveNumber);

	/** 다음 웨이브로 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void AdvanceToNextWave();

	//~ 상태 조회

	/** 던전 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsActive() const { return bIsActive; }

	/** 현재 웨이브 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetCurrentWave() const { return CurrentWave; }

	/** 참가 플레이어 수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetPlayerCount() const { return ActivePlayers.Num(); }

	/** 생존 몬스터 수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	int32 GetAliveMonsterCount() const;

	//~ 몬스터 관리

	/** 몬스터 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	AActor* SpawnMonster(const FDungeonSpawnPoint& SpawnPoint);

	/** 몬스터 등록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void RegisterMonster(AActor* Monster);

	/** 몬스터 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void UnregisterMonster(AActor* Monster);

	/** 모든 몬스터 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ClearAllMonsters();

	//~ 무한 던전용

	/** 다음 층 생성 (무한 던전용) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void GenerateNextFloor();

	/** 현재 층 정보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	FInfiniteDungeonFloor GetCurrentFloorInfo() const;

protected:
	/** 던전 활성화 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	bool bIsActive;

	/** 현재 웨이브 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentWave;

	/** 현재 층 (무한 던전용) */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	int32 CurrentFloor;

	/** 참가 플레이어 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<APlayerController*> ActivePlayers;

	/** 스폰된 몬스터 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	TArray<AActor*> SpawnedMonsters;

	/** 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category = "Dungeon")
	FGuid InstanceID;

private:
	/** 몬스터 스폰 처리 */
	void ProcessMonsterSpawns(int32 WaveNumber);

	/** 웨이브 완료 체크 */
	void CheckWaveCompletion();

	/** 완료 조건 체크 */
	void CheckCompletionConditions();

	/** 타이머 핸들 */
	FTimerHandle WaveCheckTimerHandle;
};
