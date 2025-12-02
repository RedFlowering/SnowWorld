// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonComponent.generated.h"

class UHarmoniaProgressionComponent;
class UHarmoniaInventoryComponent;

/**
 * ?˜ì „ ì§„í–‰ ?íƒœ ?¸ë¦¬ê²Œì´??
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonStateChanged, EDungeonState, OldState, EDungeonState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonTimeUpdate, float, RemainingTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfiniteDungeonFloorChanged, int32, OldFloor, int32, NewFloor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonRankingUpdated, const FDungeonRankingEntry&, NewEntry);

/**
 * ?˜ì „ ì»´í¬?ŒíŠ¸
 * ?Œë ˆ?´ì–´???˜ì „ ì§„í–‰ ?íƒœë¥?ê´€ë¦¬í•˜??ì»´í¬?ŒíŠ¸
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaDungeonComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaDungeonComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//~ ?¸ë¦¬ê²Œì´??
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonStateChanged OnDungeonStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonCompleted OnDungeonCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonTimeUpdate OnDungeonTimeUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnInfiniteDungeonFloorChanged OnInfiniteDungeonFloorChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Dungeon")
	FOnDungeonRankingUpdated OnDungeonRankingUpdated;

	//~ ?˜ì „ ì§„ì…/ì¢…ë£Œ
	
	/** ?˜ì „ ?…ì¥ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool EnterDungeon(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** ?˜ì „ ?´ì¥ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ExitDungeon(bool bSaveProgress = false);

	/** ?˜ì „ ?…ì¥ ê°€???¬ë? ?•ì¸ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool CanEnterDungeon(const UDungeonDataAsset* DungeonData) const;

	//~ ?˜ì „ ?íƒœ ê´€ë¦?

	/** ?˜ì „ ?œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** ?˜ì „ ?„ë£Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void CompleteDungeon(bool bSuccess);

	/** ?˜ì „ ?íƒœ ?¤ì • */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void SetDungeonState(EDungeonState NewState);

	/** ?„ì¬ ?˜ì „ ?íƒœ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	EDungeonState GetDungeonState() const { return CurrentDungeonState; }

	/** ?˜ì „ ì§„í–‰ ì¤‘ì¸ì§€ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsInDungeon() const { return CurrentDungeon != nullptr; }

	//~ ë¬´í•œ ?˜ì „

	/** ?¤ìŒ ì¸µìœ¼ë¡??´ë™ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void AdvanceToNextFloor();

	/** ?„ì¬ ì¸?ë²ˆí˜¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetCurrentFloor() const { return CurrentFloor; }

	/** ìµœê³  ?„ë‹¬ ì¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetHighestFloor() const { return HighestFloorReached; }

	//~ ì±Œë¦°ì§€ ëª¨ë“œ

	/** ì±Œë¦°ì§€ ëª¨ë””?Œì´???œì„±??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void ApplyChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** ?œì„±?”ëœ ëª¨ë””?Œì´???œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void RemoveChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** ?„ì¬ ?œì„±?”ëœ ëª¨ë””?Œì´??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Challenge")
	TArray<FDungeonChallengeModifier> GetActiveModifiers() const { return ActiveChallengeModifiers; }

	//~ ??‚¹

	/** ??‚¹ ?±ë¡ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	void SubmitRanking(const FString& PlayerName, float ClearTime, int32 Score);

	/** ??‚¹ ì¡°íšŒ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	TArray<FDungeonRankingEntry> GetDungeonRankings(int32 TopCount = 10) const;

	/** ??ìµœê³  ê¸°ë¡ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	FDungeonRankingEntry GetPersonalBest() const;

	//~ ë³´ìƒ

	/** ë³´ìƒ ì§€ê¸?*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Reward")
	void GrantDungeonReward();

	/** ?ˆìƒ ë³´ìƒ ê³„ì‚° */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Reward")
	FDungeonReward CalculateReward() const;

	//~ ?œê°„ ê´€ë¦?

	/** ?¨ì? ?œê°„ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetRemainingTime() const { return RemainingTime; }

	/** ê²½ê³¼ ?œê°„ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetElapsedTime() const { return ElapsedTime; }

	//~ ?Œí‹°/ê³µê²©?€

	/** ?Œí‹° ?¬ê¸° ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Party")
	int32 GetPartySize() const;

	/** ??• ë³??¸ì› ?•ì¸ (?ˆì´?œìš©) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Raid")
	TMap<ERaidRole, int32> GetRoleComposition() const;

protected:
	/** ?„ì¬ ?˜ì „ ?°ì´??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	const UDungeonDataAsset* CurrentDungeon;

	/** ?„ì¬ ?œì´??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonDifficulty CurrentDifficulty;

	/** ?„ì¬ ?˜ì „ ?íƒœ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonState CurrentDungeonState;

	/** ?¨ì? ?œê°„ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float RemainingTime;

	/** ê²½ê³¼ ?œê°„ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float ElapsedTime;

	/** ?„ì¬ ì¸?(ë¬´í•œ ?˜ì „?? */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentFloor;

	/** ìµœê³  ?„ë‹¬ ì¸?(ë¬´í•œ ?˜ì „?? */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 HighestFloorReached;

	/** ?œì„±?”ëœ ì±Œë¦°ì§€ ëª¨ë””?Œì´??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonChallengeModifier> ActiveChallengeModifiers;

	/** ?˜ì „ ??‚¹ ëª©ë¡ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonRankingEntry> DungeonRankings;

	/** ?Œë ˆ?´ì–´ ê°œì¸ ìµœê³  ê¸°ë¡ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	FDungeonRankingEntry PersonalBestRecord;

	/** ?„ì¬ ?ìˆ˜ */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentScore;

private:
	/** ?„ë¡œê·¸ë ˆ??ì»´í¬?ŒíŠ¸ ì°¸ì¡° (?ˆë²¨ ?•ì¸?? */
	UPROPERTY()
	TObjectPtr<UHarmoniaProgressionComponent> ProgressionComponent;

	/** ?¸ë²¤? ë¦¬ ì»´í¬?ŒíŠ¸ ì°¸ì¡° (?„ì´???•ì¸?? */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** ?€?´ë¨¸ ?…ë°?´íŠ¸ */
	void UpdateTimer(float DeltaTime);

	/** ?”êµ¬?¬í•­ ê²€ì¦?*/
	bool ValidateRequirements(const UDungeonDataAsset* DungeonData) const;

	/** ?ìˆ˜ ê³„ì‚° */
	int32 CalculateScore() const;

	/** ?Œë ˆ?´ì–´ ?ˆë²¨ ê°€?¸ì˜¤ê¸?*/
	int32 GetPlayerLevel() const;

	/** ?„ìš” ?„ì´??ë³´ìœ  ?•ì¸ */
	bool HasRequiredItems(const TArray<FName>& ItemIDs) const;

	/** ?„ìš” ?„ì´???Œë¹„ */
	bool ConsumeRequiredItems(const TArray<FName>& ItemIDs);
};
