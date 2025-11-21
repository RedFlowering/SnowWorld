// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonManager.generated.h"

class UDungeonDataAsset;

/**
 * 던전 인스턴스 정보
 */
USTRUCT(BlueprintType)
struct FDungeonInstanceInfo
{
	GENERATED_BODY()

	/** 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid InstanceID;

	/** 던전 데이터 */
	UPROPERTY(BlueprintReadOnly)
	const UDungeonDataAsset* DungeonData = nullptr;

	/** 난이도 */
	UPROPERTY(BlueprintReadOnly)
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** 생성 시간 */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreationTime;

	/** 참여 플레이어 목록 */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PlayerIDs;

	/** 인스턴스 상태 */
	UPROPERTY(BlueprintReadOnly)
	EDungeonState State = EDungeonState::NotStarted;
};

/**
 * 던전 랭킹 배열 래퍼 (TMap value로 사용하기 위함)
 */
USTRUCT(BlueprintType)
struct FDungeonRankingArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FDungeonRankingEntry> Rankings;
};

/**
 * 던전 매니저
 * 게임 전체의 던전 시스템을 관리하는 서브시스템
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDungeonManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ 던전 데이터 관리

	/** 던전 데이터 등록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void RegisterDungeon(UDungeonDataAsset* DungeonData);

	/** 던전 데이터 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	UDungeonDataAsset* GetDungeonData(FName DungeonID) const;

	/** 모든 던전 데이터 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetAllDungeons() const;

	/** 타입별 던전 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetDungeonsByType(EDungeonType DungeonType) const;

	//~ 인스턴스 관리

	/** 던전 인스턴스 생성 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	FGuid CreateDungeonInstance(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** 던전 인스턴스 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DestroyDungeonInstance(FGuid InstanceID);

	/** 인스턴스 정보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	FDungeonInstanceInfo GetInstanceInfo(FGuid InstanceID) const;

	/** 활성 인스턴스 목록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonInstanceInfo> GetActiveInstances() const;

	/** 플레이어를 인스턴스에 추가 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool AddPlayerToInstance(FGuid InstanceID, const FString& PlayerID);

	/** 플레이어를 인스턴스에서 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool RemovePlayerFromInstance(FGuid InstanceID, const FString& PlayerID);

	//~ 매치메이킹

	/** 던전 매칭 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void StartMatchmaking(const FString& PlayerID, FName DungeonID, EDungeonDifficulty Difficulty);

	/** 매칭 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void CancelMatchmaking(const FString& PlayerID);

	//~ 글로벌 랭킹

	/** 글로벌 랭킹 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetGlobalRankings(FName DungeonID, int32 TopCount = 100) const;

	/** 랭킹 등록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void SubmitGlobalRanking(FName DungeonID, const FDungeonRankingEntry& Entry);

	/** 시즌 랭킹 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetSeasonRankings(FName DungeonID, int32 SeasonID, int32 TopCount = 100) const;

	//~ 이벤트 던전

	/** 이벤트 던전 활성화 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void ActivateEventDungeon(FName DungeonID, FDateTime StartTime, FDateTime EndTime);

	/** 이벤트 던전 비활성화 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DeactivateEventDungeon(FName DungeonID);

	/** 활성 이벤트 던전 목록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FName> GetActiveEventDungeons() const;

	//~ 통계

	/** 던전 클리어 횟수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	int32 GetDungeonClearCount(FName DungeonID) const;

	/** 평균 클리어 시간 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetAverageClearTime(FName DungeonID) const;

	/** 클리어율 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetClearRate(FName DungeonID) const;

protected:
	/** 등록된 던전 목록 */
	UPROPERTY()
	TMap<FName, UDungeonDataAsset*> RegisteredDungeons;

	/** 활성 인스턴스 목록 */
	UPROPERTY()
	TMap<FGuid, FDungeonInstanceInfo> ActiveInstances;

	/** 글로벌 랭킹 (던전별) */
	UPROPERTY()
	TMap<FName, FDungeonRankingArray> GlobalRankings;

	/** 매칭 큐 */
	UPROPERTY()
	TMap<FString, FGuid> MatchmakingQueue;

	/** 활성 이벤트 던전 */
	UPROPERTY()
	TMap<FName, FDateTime> ActiveEventDungeons;

	/** 던전 통계 */
	UPROPERTY()
	TMap<FName, int32> DungeonClearCounts;

	UPROPERTY()
	TMap<FName, float> TotalClearTimes;

	UPROPERTY()
	TMap<FName, int32> DungeonAttemptCounts;

private:
	/** 만료된 인스턴스 정리 */
	void CleanupExpiredInstances();

	/** 매칭 처리 */
	void ProcessMatchmaking();
};
