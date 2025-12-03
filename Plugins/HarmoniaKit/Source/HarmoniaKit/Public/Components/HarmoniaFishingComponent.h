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
 * ?�시 ?�스??컴포?�트
 * ?�시 미니게임, 물고�??�기, ?�벨�??�스??처리
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
	// ?�시 기본 기능
	// ====================================

	/** ?�시 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	bool StartFishing(UFishingSpotData* FishingSpot);

	/** ?�시 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void CancelFishing();

	/** 물고기�? 물었?????�출 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void OnFishBiteDetected();

	/** 미니게임 ?�료 ???�출 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void CompleteFishingMinigame(bool bSuccess, float PerformanceScore);

	/** ?�시 중인지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool IsFishing() const { return bIsFishing; }

	/** 미니게임 진행 중인지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool IsMinigameActive() const { return bMinigameActive; }

	// ====================================
	// ?�벨 �?경험�??�스??(Base class?�서 ?�속)
	// ====================================

	/** ?�시 경험�??�득 (wrapper for base class) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void AddFishingExperience(int32 Amount) { AddExperience(Amount); }

	/** ?�재 ?�시 ?�벨 (wrapper for base class) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	int32 GetFishingLevel() const { return GetLevel(); }

	/** ?�재 경험치는 base class??GetCurrentExperience() ?�용 */
	/** ?�음 ?�벨까�? ?�요??경험치는 base class??GetExperienceForNextLevel() ?�용 */

	// ====================================
	// 물고�??�감
	// ====================================

	/** 물고�??�감???�록 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void RegisterFishToCollection(const FCaughtFish& Fish);

	/** ?�감???�록??물고�?목록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	TArray<FCaughtFish> GetFishCollection() const { return FishCollection; }

	/** ?�정 물고기의 최고 기록 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool GetBestCatchRecord(FName FishID, FCaughtFish& OutBestCatch) const;

	// ====================================
	// 미니게임 관??
	// ====================================

	/** 미니게임 ?�??가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	EFishingMinigameType GetCurrentMinigameType() const { return CurrentMinigameSettings.MinigameType; }

	/** 미니게임 ?�정 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	FFishingMinigameSettings GetCurrentMinigameSettings() const { return CurrentMinigameSettings; }

	/** 미니게임 진행??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	float GetMinigameProgress() const { return MinigameProgress; }

	/** 미니게임 ?�이??반환 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	int32 GetMinigameDifficulty() const { return CurrentMinigameSettings.Difficulty; }

	// ====================================
	// ?�벤??
	// ====================================

	/** ?�시 ?�작 ?�벤t */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingStarted OnFishingStarted;

	/** ?�시 취소 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingCancelled OnFishingCancelled;

	/** 물고기�? 물었?????�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishBite OnFishBite;

	/** 물고기�? ?�았?????�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishCaught OnFishCaught;

	/** 물고기�? ?�망갔을 ???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishEscaped OnFishEscaped;

	/** ?�벨???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingLevelUp OnFishingLevelUp;

	// ====================================
	// ?�정
	// ====================================

	/** 물고�??�이?�베?�스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	TMap<FName, FFishData> FishDatabase;

	/** 물고기�? 물기까�? 최소 ?�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	float MinBiteTime = 3.0f;

	/** 물고기�? 물기까�? 최�? ?�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	float MaxBiteTime = 15.0f;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel?� base class???�의??

private:
	/** ?�시 �??�래�?*/
	UPROPERTY()
	bool bIsFishing = false;

	/** 미니게임 ?�성???�래�?*/
	UPROPERTY()
	bool bMinigameActive = false;

	/** ?�재 ?�시??*/
	UPROPERTY()
	TObjectPtr<UFishingSpotData> CurrentFishingSpot;

	/** ?�재 미니게임 ?�정 */
	UPROPERTY()
	FFishingMinigameSettings CurrentMinigameSettings;

	/** 미니게임 진행??*/
	UPROPERTY()
	float MinigameProgress = 0.0f;

	/** 물고기�? 물기까�? ?��? ?�간 */
	UPROPERTY()
	float TimeUntilBite = 0.0f;

	// Note: Level, CurrentExperience??base class???�의??

	/** ?��? 물고�?컬렉??*/
	UPROPERTY()
	TArray<FCaughtFish> FishCollection;

	/** 물고기별 최고 기록 */
	UPROPERTY()
	TMap<FName, FCaughtFish> BestCatchRecords;

	/** ?�시 ?�작 ?�간 */
	UPROPERTY()
	float FishingStartTime = 0.0f;

	/** 물고�??�택 */
	FCaughtFish SelectFishFromSpawnTable();

	/** 물고�??�성 */
	FCaughtFish GenerateFish(FName FishID, const FFishData& FishData);

	// Note: CheckAndProcessLevelUp??base class???�의??
};
