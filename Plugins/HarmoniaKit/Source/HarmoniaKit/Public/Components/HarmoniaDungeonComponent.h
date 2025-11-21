// Copyright (c) 2025 RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaDungeonSystemDefinitions.h"
#include "GameplayTagContainer.h"
#include "HarmoniaDungeonComponent.generated.h"

/**
 * 던전 진행 상태 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonStateChanged, EDungeonState, OldState, EDungeonState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDungeonTimeUpdate, float, RemainingTime, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfiniteDungeonFloorChanged, int32, OldFloor, int32, NewFloor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDungeonRankingUpdated, const FDungeonRankingEntry&, NewEntry);

/**
 * 던전 컴포넌트
 * 플레이어의 던전 진행 상태를 관리하는 컴포넌트
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
	//~ 델리게이트
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

	//~ 던전 진입/종료
	
	/** 던전 입장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool EnterDungeon(const UDungeonDataAsset* DungeonData, EDungeonDifficulty Difficulty);

	/** 던전 퇴장 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void ExitDungeon(bool bSaveProgress = false);

	/** 던전 입장 가능 여부 확인 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	bool CanEnterDungeon(const UDungeonDataAsset* DungeonData) const;

	//~ 던전 상태 관리

	/** 던전 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void StartDungeon();

	/** 던전 완료 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void CompleteDungeon(bool bSuccess);

	/** 던전 상태 설정 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon")
	void SetDungeonState(EDungeonState NewState);

	/** 현재 던전 상태 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	EDungeonState GetDungeonState() const { return CurrentDungeonState; }

	/** 던전 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon")
	bool IsInDungeon() const { return CurrentDungeon != nullptr; }

	//~ 무한 던전

	/** 다음 층으로 이동 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Infinite")
	void AdvanceToNextFloor();

	/** 현재 층 번호 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetCurrentFloor() const { return CurrentFloor; }

	/** 최고 도달 층 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Infinite")
	int32 GetHighestFloor() const { return HighestFloorReached; }

	//~ 챌린지 모드

	/** 챌린지 모디파이어 활성화 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void ApplyChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** 활성화된 모디파이어 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Challenge")
	void RemoveChallengeModifier(const FDungeonChallengeModifier& Modifier);

	/** 현재 활성화된 모디파이어 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Challenge")
	TArray<FDungeonChallengeModifier> GetActiveModifiers() const { return ActiveChallengeModifiers; }

	//~ 랭킹

	/** 랭킹 등록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	void SubmitRanking(const FString& PlayerName, float ClearTime, int32 Score);

	/** 랭킹 조회 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	TArray<FDungeonRankingEntry> GetDungeonRankings(int32 TopCount = 10) const;

	/** 내 최고 기록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Ranking")
	FDungeonRankingEntry GetPersonalBest() const;

	//~ 보상

	/** 보상 지급 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Reward")
	void GrantDungeonReward();

	/** 예상 보상 계산 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Reward")
	FDungeonReward CalculateReward() const;

	//~ 시간 관리

	/** 남은 시간 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetRemainingTime() const { return RemainingTime; }

	/** 경과 시간 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Time")
	float GetElapsedTime() const { return ElapsedTime; }

	//~ 파티/공격대

	/** 파티 크기 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Dungeon|Party")
	int32 GetPartySize() const;

	/** 역할별 인원 확인 (레이드용) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Dungeon|Raid")
	TMap<ERaidRole, int32> GetRoleComposition() const;

protected:
	/** 현재 던전 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	const UDungeonDataAsset* CurrentDungeon;

	/** 현재 난이도 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonDifficulty CurrentDifficulty;

	/** 현재 던전 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	EDungeonState CurrentDungeonState;

	/** 남은 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float RemainingTime;

	/** 경과 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	float ElapsedTime;

	/** 현재 층 (무한 던전용) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentFloor;

	/** 최고 도달 층 (무한 던전용) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 HighestFloorReached;

	/** 활성화된 챌린지 모디파이어 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonChallengeModifier> ActiveChallengeModifiers;

	/** 던전 랭킹 목록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	TArray<FDungeonRankingEntry> DungeonRankings;

	/** 플레이어 개인 최고 기록 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	FDungeonRankingEntry PersonalBestRecord;

	/** 현재 점수 */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dungeon")
	int32 CurrentScore;

private:
	/** 타이머 업데이트 */
	void UpdateTimer(float DeltaTime);

	/** 요구사항 검증 */
	bool ValidateRequirements(const UDungeonDataAsset* DungeonData) const;

	/** 점수 계산 */
	int32 CalculateScore() const;
};
