// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "Definitions/HarmoniaFishingSystemDefinitions.h"
#include "HarmoniaFishingComponent.generated.h"

class UFishingSpotData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishingStarted, UFishingSpotData*, FishingSpot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFishingCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFishBite);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishCaught, const FCaughtFish&, Fish, bool, bPerfectCatch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFishEscaped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFishingLevelUp, int32, NewLevel, int32, SkillPoints);

/**
 * 낚시 시스템 컴포넌트
 * 낚시 미니게임, 물고기 잡기, 레벨링 시스템 처리
 * Inherits leveling, experience, and activity management from UHarmoniaBaseLifeContentComponent
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaFishingComponent : public UHarmoniaBaseLifeContentComponent
{
	GENERATED_BODY()

public:
	UHarmoniaFishingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnActivityComplete() override;
	virtual void OnLevelUpInternal(int32 NewLevel) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// 낚시 기본 기능
	// ====================================

	/** 낚시 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	bool StartFishing(UFishingSpotData* FishingSpot);

	/** 낚시 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void CancelFishing();

	/** 물고기가 물었을 때 호출 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void OnFishBiteDetected();

	/** 미니게임 완료 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void CompleteFishingMinigame(bool bSuccess, float PerformanceScore);

	/** 낚시 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool IsFishing() const { return bIsFishing; }

	/** 미니게임 진행 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool IsMinigameActive() const { return bMinigameActive; }

	// ====================================
	// 레벨 및 경험치 시스템 (Base class에서 상속)
	// ====================================

	/** 낚시 경험치 획득 (wrapper for base class) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void AddFishingExperience(int32 Amount) { AddExperience(Amount); }

	/** 현재 낚시 레벨 (wrapper for base class) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	int32 GetFishingLevel() const { return GetLevel(); }

	/** 현재 경험치는 base class의 GetCurrentExperience() 사용 */
	/** 다음 레벨까지 필요한 경험치는 base class의 GetExperienceForNextLevel() 사용 */

	// ====================================
	// 물고기 도감
	// ====================================

	/** 물고기 도감에 등록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void RegisterFishToCollection(const FCaughtFish& Fish);

	/** 도감에 등록된 물고기 목록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	TArray<FCaughtFish> GetFishCollection() const { return FishCollection; }

	/** 특정 물고기의 최고 기록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool GetBestCatchRecord(FName FishID, FCaughtFish& OutBestCatch) const;

	// ====================================
	// 미니게임 관련
	// ====================================

	/** 미니게임 타입 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	EFishingMinigameType GetCurrentMinigameType() const { return CurrentMinigameSettings.MinigameType; }

	/** 미니게임 설정 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	FFishingMinigameSettings GetCurrentMinigameSettings() const { return CurrentMinigameSettings; }

	/** 미니게임 진행도 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	float GetMinigameProgress() const { return MinigameProgress; }

	/** 미니게임 난이도 반환 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	int32 GetMinigameDifficulty() const { return CurrentMinigameSettings.Difficulty; }

	// ====================================
	// 이벤트
	// ====================================

	/** 낚시 시작 이벤t */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingStarted OnFishingStarted;

	/** 낚시 취소 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingCancelled OnFishingCancelled;

	/** 물고기가 물었을 때 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishBite OnFishBite;

	/** 물고기를 잡았을 때 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishCaught OnFishCaught;

	/** 물고기가 도망갔을 때 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishEscaped OnFishEscaped;

	/** 레벨업 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingLevelUp OnFishingLevelUp;

	// ====================================
	// 설정
	// ====================================

	/** 물고기 데이터베이스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	TMap<FName, FFishData> FishDatabase;

	/** 물고기가 물기까지 최소 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	float MinBiteTime = 3.0f;

	/** 물고기가 물기까지 최대 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	float MaxBiteTime = 15.0f;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel은 base class에 정의됨

private:
	/** 낚시 중 플래그 */
	UPROPERTY()
	bool bIsFishing = false;

	/** 미니게임 활성화 플래그 */
	UPROPERTY()
	bool bMinigameActive = false;

	/** 현재 낚시터 */
	UPROPERTY()
	TObjectPtr<UFishingSpotData> CurrentFishingSpot;

	/** 현재 미니게임 설정 */
	UPROPERTY()
	FFishingMinigameSettings CurrentMinigameSettings;

	/** 미니게임 진행도 */
	UPROPERTY()
	float MinigameProgress = 0.0f;

	/** 물고기가 물기까지 남은 시간 */
	UPROPERTY()
	float TimeUntilBite = 0.0f;

	// Note: Level, CurrentExperience는 base class에 정의됨

	/** 잡은 물고기 컬렉션 */
	UPROPERTY()
	TArray<FCaughtFish> FishCollection;

	/** 물고기별 최고 기록 */
	UPROPERTY()
	TMap<FName, FCaughtFish> BestCatchRecords;

	/** 낚시 시작 시간 */
	UPROPERTY()
	float FishingStartTime = 0.0f;

	/** 물고기 선택 */
	FCaughtFish SelectFishFromSpawnTable();

	/** 물고기 생성 */
	FCaughtFish GenerateFish(FName FishID, const FFishData& FishData);

	// Note: CheckAndProcessLevelUp는 base class에 정의됨
};
