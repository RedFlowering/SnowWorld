// Copyright (c) 2025 RedFlowering. All Rights Reserved.

/**
 * @file HarmoniaDungeonComponent.h
 * @brief Dungeon progression state management component
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonComponent.generated.h"

class UHarmoniaProgressionComponent;
class UHarmoniaInventoryComponent;

/**
 * Dungeon progression state delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonStateChanged, EDungeonState, OldState, EDungeonState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonTimeUpdate, float, RemainingTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfiniteDungeonFloorChanged, int32, OldFloor, int32, NewFloor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonRankingUpdated, const FDungeonRankingEntry&, NewEntry);

/**
 * @class UHarmoniaDungeonComponent
 * @brief Dungeon component for managing player's dungeon progression state
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
	//~ Delegates
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

	//~ Dungeon Entry/Exit
	
	/** Enter dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool EnterDungeon(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** Exit dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ExitDungeon(bool bSaveProgress = false);

	/** Check if can enter dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool CanEnterDungeon(const UDungeonDataAsset* DungeonData) const;

	//~ Dungeon State Management

	/** Start dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** Complete dungeon */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void CompleteDungeon(bool bSuccess);

	/** Set dungeon state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void SetDungeonState(EDungeonState NewState);

	/** Get current dungeon state */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	EDungeonState GetDungeonState() const { return CurrentDungeonState; }

	/** Check if in dungeon */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsInDungeon() const { return CurrentDungeon != nullptr; }

	//~ Infinite Dungeon

	/** Advance to next floor */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void AdvanceToNextFloor();

	/** Get current floor number */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetCurrentFloor() const { return CurrentFloor; }

	/** Get highest reached floor */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetHighestFloor() const { return HighestFloorReached; }

	//~ Challenge Mode

	/** Apply challenge modifier */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void ApplyChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** Remove activated modifier */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void RemoveChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** Get currently active modifiers */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Challenge")
	TArray<FDungeonChallengeModifier> GetActiveModifiers() const { return ActiveChallengeModifiers; }

	//~ Ranking

	/** Submit ranking */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	void SubmitRanking(const FString& PlayerName, float ClearTime, int32 Score);

	/** Get dungeon rankings */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	TArray<FDungeonRankingEntry> GetDungeonRankings(int32 TopCount = 10) const;

	/** Get personal best record */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	FDungeonRankingEntry GetPersonalBest() const;

	//~ Rewards

	/** Grant dungeon reward */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Reward")
	void GrantDungeonReward();

	/** Calculate expected reward */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Reward")
	FDungeonReward CalculateReward() const;

	//~ Time Management

	/** Get remaining time */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetRemainingTime() const { return RemainingTime; }

	/** Get elapsed time */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetElapsedTime() const { return ElapsedTime; }

	//~ Party/Raid

	/** Get party size */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Party")
	int32 GetPartySize() const;

	/** Get role composition (for raids) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Raid")
	TMap<ERaidRole, int32> GetRoleComposition() const;

protected:
	/** Current dungeon data */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	const UDungeonDataAsset* CurrentDungeon;

	/** Current difficulty */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonDifficulty CurrentDifficulty;

	/** Current dungeon state */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonState CurrentDungeonState;

	/** Remaining time */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float RemainingTime;

	/** Elapsed time */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float ElapsedTime;

	/** Current floor (infinite dungeon) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentFloor;

	/** Highest reached floor (infinite dungeon) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 HighestFloorReached;

	/** Active challenge modifiers */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonChallengeModifier> ActiveChallengeModifiers;

	/** Dungeon rankings list */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonRankingEntry> DungeonRankings;

	/** Player personal best record */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	FDungeonRankingEntry PersonalBestRecord;

	/** Current score */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentScore;

private:
	/** Progression component reference (for level check) */
	UPROPERTY()
	TObjectPtr<UHarmoniaProgressionComponent> ProgressionComponent;

	/** Inventory component reference (for item check) */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** Update timer */
	void UpdateTimer(float DeltaTime);

	/** Validate requirements */
	bool ValidateRequirements(const UDungeonDataAsset* DungeonData) const;

	/** Calculate score */
	int32 CalculateScore() const;

	/** Get player level */
	int32 GetPlayerLevel() const;

	/** Check if has required items */
	bool HasRequiredItems(const TArray<FName>& ItemIDs) const;

	/** Consume required items */
	bool ConsumeRequiredItems(const TArray<FName>& ItemIDs);
};
