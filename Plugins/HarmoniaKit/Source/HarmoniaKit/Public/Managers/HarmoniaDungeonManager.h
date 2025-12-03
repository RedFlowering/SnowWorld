// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonManager.generated.h"

class UDungeonDataAsset;

/**
 * ?�전 ?�스?�스 ?�보
 */
USTRUCT(BlueprintType)
struct FDungeonInstanceInfo
{
	GENERATED_BODY()

	/** ?�스?�스 ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid InstanceID;

	/** ?�전 ?�이??*/
	UPROPERTY(BlueprintReadOnly)
	const UDungeonDataAsset* DungeonData = nullptr;

	/** ?�이??*/
	UPROPERTY(BlueprintReadOnly)
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** ?�성 ?�간 */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreationTime;

	/** 참여 ?�레?�어 목록 */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PlayerIDs;

	/** ?�스?�스 ?�태 */
	UPROPERTY(BlueprintReadOnly)
	EDungeonState State = EDungeonState::NotStarted;
};

/**
 * ?�전 ??�� 배열 ?�퍼 (TMap value�??�용?�기 ?�함)
 */
USTRUCT(BlueprintType)
struct FDungeonRankingArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FDungeonRankingEntry> Rankings;
};

/**
 * ?�전 매니?�
 * 게임 ?�체???�전 ?�스?�을 관리하???�브?�스??
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDungeonManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ ?�전 ?�이??관�?

	/** ?�전 ?�이???�록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void RegisterDungeon(UDungeonDataAsset* DungeonData);

	/** ?�전 ?�이??조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	UDungeonDataAsset* GetDungeonData(FName DungeonID) const;

	/** 모든 ?�전 ?�이??조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetAllDungeons() const;

	/** ?�?�별 ?�전 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetDungeonsByType(EDungeonType DungeonType) const;

	//~ ?�스?�스 관�?

	/** ?�전 ?�스?�스 ?�성 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	FGuid CreateDungeonInstance(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** ?�전 ?�스?�스 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DestroyDungeonInstance(FGuid InstanceID);

	/** ?�스?�스 ?�보 조회 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	FDungeonInstanceInfo GetInstanceInfo(FGuid InstanceID) const;

	/** ?�성 ?�스?�스 목록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonInstanceInfo> GetActiveInstances() const;

	/** ?�레?�어�??�스?�스??추�? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool AddPlayerToInstance(FGuid InstanceID, const FString& PlayerID);

	/** ?�레?�어�??�스?�스?�서 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool RemovePlayerFromInstance(FGuid InstanceID, const FString& PlayerID);

	//~ 매치메이??

	/** ?�전 매칭 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void StartMatchmaking(const FString& PlayerID, FName DungeonID, EDungeonDifficulty Difficulty);

	/** 매칭 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void CancelMatchmaking(const FString& PlayerID);

	//~ 글로벌 ??��

	/** 글로벌 ??�� 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetGlobalRankings(FName DungeonID, int32 TopCount = 100) const;

	/** ??�� ?�록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void SubmitGlobalRanking(FName DungeonID, const FDungeonRankingEntry& Entry);

	/** ?�즌 ??�� 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetSeasonRankings(FName DungeonID, int32 SeasonID, int32 TopCount = 100) const;

	//~ ?�벤???�전

	/** ?�벤???�전 ?�성??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void ActivateEventDungeon(FName DungeonID, FDateTime StartTime, FDateTime EndTime);

	/** ?�벤???�전 비활?�화 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DeactivateEventDungeon(FName DungeonID);

	/** ?�성 ?�벤???�전 목록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FName> GetActiveEventDungeons() const;

	//~ ?�계

	/** ?�전 ?�리???�수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	int32 GetDungeonClearCount(FName DungeonID) const;

	/** ?�균 ?�리???�간 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetAverageClearTime(FName DungeonID) const;

	/** ?�리?�율 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetClearRate(FName DungeonID) const;

protected:
	/** ?�록???�전 목록 */
	UPROPERTY()
	TMap<FName, UDungeonDataAsset*> RegisteredDungeons;

	/** ?�성 ?�스?�스 목록 */
	UPROPERTY()
	TMap<FGuid, FDungeonInstanceInfo> ActiveInstances;

	/** 글로벌 ??�� (?�전�? */
	UPROPERTY()
	TMap<FName, FDungeonRankingArray> GlobalRankings;

	/** 매칭 ??*/
	UPROPERTY()
	TMap<FString, FGuid> MatchmakingQueue;

	/** ?�성 ?�벤???�전 */
	UPROPERTY()
	TMap<FName, FDateTime> ActiveEventDungeons;

	/** ?�전 ?�계 */
	UPROPERTY()
	TMap<FName, int32> DungeonClearCounts;

	UPROPERTY()
	TMap<FName, float> TotalClearTimes;

	UPROPERTY()
	TMap<FName, int32> DungeonAttemptCounts;

private:
	/** 만료???�스?�스 ?�리 */
	void CleanupExpiredInstances();

	/** 매칭 처리 */
	void ProcessMatchmaking();
};
