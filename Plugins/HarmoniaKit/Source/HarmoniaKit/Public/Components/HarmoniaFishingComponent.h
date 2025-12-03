// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaFishingComponent.h
 * @brief Fishing system component for minigame, catch, and progression
 * @author Harmonia Team
 */

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
 * @class UHarmoniaFishingComponent
 * @brief Fishing system component for fish catching minigame
 * 
 * Handles fishing minigame, fish catching, and leveling system.
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
	// Fishing Basic Functions
	// ====================================

	/** Start fishing at specified spot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	bool StartFishing(UFishingSpotData* FishingSpot);

	/** Cancel current fishing */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void CancelFishing();

	/** Called when fish bite is detected */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void OnFishBiteDetected();

	/** Called when minigame completes */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void CompleteFishingMinigame(bool bSuccess, float PerformanceScore);

	/** Check if currently fishing */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool IsFishing() const { return bIsFishing; }

	/** Check if minigame is active */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool IsMinigameActive() const { return bMinigameActive; }

	// ====================================
	// Level & Experience System (Inherited from Base class)
	// ====================================

	/** Add fishing experience (wrapper for base class) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void AddFishingExperience(int32 Amount) { AddExperience(Amount); }

	/** Get current fishing level (wrapper for base class) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	int32 GetFishingLevel() const { return GetLevel(); }

	/** Current experience uses base class GetCurrentExperience() */
	/** Experience for next level uses base class GetExperienceForNextLevel() */

	// ====================================
	// Fish Collection
	// ====================================

	/** Register caught fish to collection */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Fishing")
	void RegisterFishToCollection(const FCaughtFish& Fish);

	/** Get list of collected fish */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	TArray<FCaughtFish> GetFishCollection() const { return FishCollection; }

	/** Get best catch record for specific fish */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	bool GetBestCatchRecord(FName FishID, FCaughtFish& OutBestCatch) const;

	// ====================================
	// Minigame Management
	// ====================================
	// ====================================

	/** Get current minigame type */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	EFishingMinigameType GetCurrentMinigameType() const { return CurrentMinigameSettings.MinigameType; }

	/** Get current minigame settings */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	FFishingMinigameSettings GetCurrentMinigameSettings() const { return CurrentMinigameSettings; }

	/** Get minigame progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	float GetMinigameProgress() const { return MinigameProgress; }

	/** Get minigame difficulty */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Fishing")
	int32 GetMinigameDifficulty() const { return CurrentMinigameSettings.Difficulty; }

	// ====================================
	// Events
	// ====================================

	/** Event fired when fishing starts */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingStarted OnFishingStarted;

	/** Event fired when fishing is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingCancelled OnFishingCancelled;

	/** Event fired when fish bites */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishBite OnFishBite;

	/** Event fired when fish is caught */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishCaught OnFishCaught;

	/** Event fired when fish escapes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishEscaped OnFishEscaped;

	/** Event fired on level up */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Fishing")
	FOnFishingLevelUp OnFishingLevelUp;

	// ====================================
	// Settings
	// ====================================

	/** Fish database */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	TMap<FName, FFishData> FishDatabase;

	/** Minimum time until fish bites */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	float MinBiteTime = 3.0f;

	/** Maximum time until fish bites */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Settings")
	float MaxBiteTime = 15.0f;

	// Note: ExperienceMultiplier, BaseExperiencePerLevel are defined in base class

private:
	/** Fishing status flag */
	UPROPERTY()
	bool bIsFishing = false;

	/** Minigame active flag */
	UPROPERTY()
	bool bMinigameActive = false;

	/** Current fishing spot */
	UPROPERTY()
	TObjectPtr<UFishingSpotData> CurrentFishingSpot;

	/** Current minigame settings */
	UPROPERTY()
	FFishingMinigameSettings CurrentMinigameSettings;

	/** Minigame progress */
	UPROPERTY()
	float MinigameProgress = 0.0f;

	/** Time remaining until fish bites */
	UPROPERTY()
	float TimeUntilBite = 0.0f;

	// Note: Level, CurrentExperience are defined in base class

	/** Caught fish collection */
	UPROPERTY()
	TArray<FCaughtFish> FishCollection;

	/** Best catch records per fish type */
	UPROPERTY()
	TMap<FName, FCaughtFish> BestCatchRecords;

	/** Fishing start time */
	UPROPERTY()
	float FishingStartTime = 0.0f;

	/** Select fish from spawn table */
	FCaughtFish SelectFishFromSpawnTable();

	/** Generate fish instance */
	FCaughtFish GenerateFish(FName FishID, const FFishData& FishData);

	// Note: CheckAndProcessLevelUp is defined in base class
};
