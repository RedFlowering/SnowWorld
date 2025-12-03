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
 * ?�전 진행 ?�태 ?�리게이??
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonStateChanged, EDungeonState, OldState, EDungeonState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonTimeUpdate, float, RemainingTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfiniteDungeonFloorChanged, int32, OldFloor, int32, NewFloor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonRankingUpdated, const FDungeonRankingEntry&, NewEntry);

/**
 * ?�전 컴포?�트
 * ?�레?�어???�전 진행 ?�태�?관리하??컴포?�트
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
	//~ ?�리게이??
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

	//~ ?�전 진입/종료
	
	/** ?�전 ?�장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool EnterDungeon(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** ?�전 ?�장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ExitDungeon(bool bSaveProgress = false);

	/** ?�전 ?�장 가???��? ?�인 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool CanEnterDungeon(const UDungeonDataAsset* DungeonData) const;

	//~ ?�전 ?�태 관�?

	/** ?�전 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** ?�전 ?�료 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void CompleteDungeon(bool bSuccess);

	/** ?�전 ?�태 ?�정 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void SetDungeonState(EDungeonState NewState);

	/** ?�재 ?�전 ?�태 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	EDungeonState GetDungeonState() const { return CurrentDungeonState; }

	/** ?�전 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsInDungeon() const { return CurrentDungeon != nullptr; }

	//~ 무한 ?�전

	/** ?�음 층으�??�동 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void AdvanceToNextFloor();

	/** ?�재 �?번호 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetCurrentFloor() const { return CurrentFloor; }

	/** 최고 ?�달 �?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetHighestFloor() const { return HighestFloorReached; }

	//~ 챌린지 모드

	/** 챌린지 모디?�이???�성??*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void ApplyChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** ?�성?�된 모디?�이???�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void RemoveChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** ?�재 ?�성?�된 모디?�이??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Challenge")
	TArray<FDungeonChallengeModifier> GetActiveModifiers() const { return ActiveChallengeModifiers; }

	//~ ??��

	/** ??�� ?�록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	void SubmitRanking(const FString& PlayerName, float ClearTime, int32 Score);

	/** ??�� 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	TArray<FDungeonRankingEntry> GetDungeonRankings(int32 TopCount = 10) const;

	/** ??최고 기록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	FDungeonRankingEntry GetPersonalBest() const;

	//~ 보상

	/** 보상 지�?*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Reward")
	void GrantDungeonReward();

	/** ?�상 보상 계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Reward")
	FDungeonReward CalculateReward() const;

	//~ ?�간 관�?

	/** ?��? ?�간 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetRemainingTime() const { return RemainingTime; }

	/** 경과 ?�간 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetElapsedTime() const { return ElapsedTime; }

	//~ ?�티/공격?�

	/** ?�티 ?�기 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Party")
	int32 GetPartySize() const;

	/** ??���??�원 ?�인 (?�이?�용) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Raid")
	TMap<ERaidRole, int32> GetRoleComposition() const;

protected:
	/** ?�재 ?�전 ?�이??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	const UDungeonDataAsset* CurrentDungeon;

	/** ?�재 ?�이??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonDifficulty CurrentDifficulty;

	/** ?�재 ?�전 ?�태 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonState CurrentDungeonState;

	/** ?��? ?�간 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float RemainingTime;

	/** 경과 ?�간 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float ElapsedTime;

	/** ?�재 �?(무한 ?�전?? */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentFloor;

	/** 최고 ?�달 �?(무한 ?�전?? */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 HighestFloorReached;

	/** ?�성?�된 챌린지 모디?�이??*/
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonChallengeModifier> ActiveChallengeModifiers;

	/** ?�전 ??�� 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonRankingEntry> DungeonRankings;

	/** ?�레?�어 개인 최고 기록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	FDungeonRankingEntry PersonalBestRecord;

	/** ?�재 ?�수 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentScore;

private:
	/** ?�로그레??컴포?�트 참조 (?�벨 ?�인?? */
	UPROPERTY()
	TObjectPtr<UHarmoniaProgressionComponent> ProgressionComponent;

	/** ?�벤?�리 컴포?�트 참조 (?�이???�인?? */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** ?�?�머 ?�데?�트 */
	void UpdateTimer(float DeltaTime);

	/** ?�구?�항 검�?*/
	bool ValidateRequirements(const UDungeonDataAsset* DungeonData) const;

	/** ?�수 계산 */
	int32 CalculateScore() const;

	/** ?�레?�어 ?�벨 가?�오�?*/
	int32 GetPlayerLevel() const;

	/** ?�요 ?�이??보유 ?�인 */
	bool HasRequiredItems(const TArray<FName>& ItemIDs) const;

	/** ?�요 ?�이???�비 */
	bool ConsumeRequiredItems(const TArray<FName>& ItemIDs);
};
