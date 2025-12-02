// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "HarmoniaDungeonManager.generated.h"

class UDungeonDataAsset;

/**
 * ?˜ì „ ?¸ìŠ¤?´ìŠ¤ ?•ë³´
 */
USTRUCT(BlueprintType)
struct FDungeonInstanceInfo
{
	GENERATED_BODY()

	/** ?¸ìŠ¤?´ìŠ¤ ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid InstanceID;

	/** ?˜ì „ ?°ì´??*/
	UPROPERTY(BlueprintReadOnly)
	const UDungeonDataAsset* DungeonData = nullptr;

	/** ?œì´??*/
	UPROPERTY(BlueprintReadOnly)
	EDungeonDifficulty Difficulty = EDungeonDifficulty::Normal;

	/** ?ì„± ?œê°„ */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreationTime;

	/** ì°¸ì—¬ ?Œë ˆ?´ì–´ ëª©ë¡ */
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> PlayerIDs;

	/** ?¸ìŠ¤?´ìŠ¤ ?íƒœ */
	UPROPERTY(BlueprintReadOnly)
	EDungeonState State = EDungeonState::NotStarted;
};

/**
 * ?˜ì „ ??‚¹ ë°°ì—´ ?˜í¼ (TMap valueë¡??¬ìš©?˜ê¸° ?„í•¨)
 */
USTRUCT(BlueprintType)
struct FDungeonRankingArray
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FDungeonRankingEntry> Rankings;
};

/**
 * ?˜ì „ ë§¤ë‹ˆ?€
 * ê²Œì„ ?„ì²´???˜ì „ ?œìŠ¤?œì„ ê´€ë¦¬í•˜???œë¸Œ?œìŠ¤??
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDungeonManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ ?˜ì „ ?°ì´??ê´€ë¦?

	/** ?˜ì „ ?°ì´???±ë¡ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void RegisterDungeon(UDungeonDataAsset* DungeonData);

	/** ?˜ì „ ?°ì´??ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	UDungeonDataAsset* GetDungeonData(FName DungeonID) const;

	/** ëª¨ë“  ?˜ì „ ?°ì´??ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetAllDungeons() const;

	/** ?€?…ë³„ ?˜ì „ ì¡°íšŒ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<UDungeonDataAsset*> GetDungeonsByType(EDungeonType DungeonType) const;

	//~ ?¸ìŠ¤?´ìŠ¤ ê´€ë¦?

	/** ?˜ì „ ?¸ìŠ¤?´ìŠ¤ ?ì„± */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	FGuid CreateDungeonInstance(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** ?˜ì „ ?¸ìŠ¤?´ìŠ¤ ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DestroyDungeonInstance(FGuid InstanceID);

	/** ?¸ìŠ¤?´ìŠ¤ ?•ë³´ ì¡°íšŒ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	FDungeonInstanceInfo GetInstanceInfo(FGuid InstanceID) const;

	/** ?œì„± ?¸ìŠ¤?´ìŠ¤ ëª©ë¡ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonInstanceInfo> GetActiveInstances() const;

	/** ?Œë ˆ?´ì–´ë¥??¸ìŠ¤?´ìŠ¤??ì¶”ê? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool AddPlayerToInstance(FGuid InstanceID, const FString& PlayerID);

	/** ?Œë ˆ?´ì–´ë¥??¸ìŠ¤?´ìŠ¤?ì„œ ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	bool RemovePlayerFromInstance(FGuid InstanceID, const FString& PlayerID);

	//~ ë§¤ì¹˜ë©”ì´??

	/** ?˜ì „ ë§¤ì¹­ ?œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void StartMatchmaking(const FString& PlayerID, FName DungeonID, EDungeonDifficulty Difficulty);

	/** ë§¤ì¹­ ì·¨ì†Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void CancelMatchmaking(const FString& PlayerID);

	//~ ê¸€ë¡œë²Œ ??‚¹

	/** ê¸€ë¡œë²Œ ??‚¹ ì¡°íšŒ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetGlobalRankings(FName DungeonID, int32 TopCount = 100) const;

	/** ??‚¹ ?±ë¡ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void SubmitGlobalRanking(FName DungeonID, const FDungeonRankingEntry& Entry);

	/** ?œì¦Œ ??‚¹ ì¡°íšŒ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	TArray<FDungeonRankingEntry> GetSeasonRankings(FName DungeonID, int32 SeasonID, int32 TopCount = 100) const;

	//~ ?´ë²¤???˜ì „

	/** ?´ë²¤???˜ì „ ?œì„±??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void ActivateEventDungeon(FName DungeonID, FDateTime StartTime, FDateTime EndTime);

	/** ?´ë²¤???˜ì „ ë¹„í™œ?±í™” */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Manager")
	void DeactivateEventDungeon(FName DungeonID);

	/** ?œì„± ?´ë²¤???˜ì „ ëª©ë¡ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	TArray<FName> GetActiveEventDungeons() const;

	//~ ?µê³„

	/** ?˜ì „ ?´ë¦¬???Ÿìˆ˜ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	int32 GetDungeonClearCount(FName DungeonID) const;

	/** ?‰ê·  ?´ë¦¬???œê°„ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetAverageClearTime(FName DungeonID) const;

	/** ?´ë¦¬?´ìœ¨ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Manager")
	float GetClearRate(FName DungeonID) const;

protected:
	/** ?±ë¡???˜ì „ ëª©ë¡ */
	UPROPERTY()
	TMap<FName, UDungeonDataAsset*> RegisteredDungeons;

	/** ?œì„± ?¸ìŠ¤?´ìŠ¤ ëª©ë¡ */
	UPROPERTY()
	TMap<FGuid, FDungeonInstanceInfo> ActiveInstances;

	/** ê¸€ë¡œë²Œ ??‚¹ (?˜ì „ë³? */
	UPROPERTY()
	TMap<FName, FDungeonRankingArray> GlobalRankings;

	/** ë§¤ì¹­ ??*/
	UPROPERTY()
	TMap<FString, FGuid> MatchmakingQueue;

	/** ?œì„± ?´ë²¤???˜ì „ */
	UPROPERTY()
	TMap<FName, FDateTime> ActiveEventDungeons;

	/** ?˜ì „ ?µê³„ */
	UPROPERTY()
	TMap<FName, int32> DungeonClearCounts;

	UPROPERTY()
	TMap<FName, float> TotalClearTimes;

	UPROPERTY()
	TMap<FName, int32> DungeonAttemptCounts;

private:
	/** ë§Œë£Œ???¸ìŠ¤?´ìŠ¤ ?•ë¦¬ */
	void CleanupExpiredInstances();

	/** ë§¤ì¹­ ì²˜ë¦¬ */
	void ProcessMatchmaking();
};
