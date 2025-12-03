// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmoniaBaseLifeContentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLifeContentLevelUp, int32, NewLevel, int32, SkillPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLifeContentActivityStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLifeContentActivityCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLifeContentActivityCompleted);

/**
 * Base class for all Life Content components (Fishing, Gathering, Cooking, Farming, Music)
 * Provides common functionality: leveling system, activity state management, experience tracking
 */
UCLASS(Abstract, ClassGroup=(HarmoniaKit))
class HARMONIAKIT_API UHarmoniaBaseLifeContentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBaseLifeContentComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// Activity State Management
	// ====================================

	/** Start the activity (to be implemented by subclasses) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|LifeContent")
	virtual bool StartActivity();

	/** Cancel the current activity */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|LifeContent")
	virtual void CancelActivity();

	/** Check if activity is currently active */
	UFUNCTION(BlueprintPure, Category = "Harmonia|LifeContent")
	bool IsActivityActive() const { return bIsActivityActive; }

	/** Get activity progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|LifeContent")
	float GetActivityProgress() const;

	// ====================================
	// Level and Experience System
	// ====================================

	/** Add experience points */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|LifeContent")
	void AddExperience(int32 Amount);

	/** Get current level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|LifeContent")
	int32 GetLevel() const { return Level; }

	/** Get current experience */
	UFUNCTION(BlueprintPure, Category = "Harmonia|LifeContent")
	int32 GetCurrentExperience() const { return CurrentExperience; }

	/** Get experience required for next level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|LifeContent")
	int32 GetExperienceForNextLevel() const;

	/** Calculate experience required for a specific level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|LifeContent")
	int32 CalculateExperienceForLevel(int32 TargetLevel) const;

	// ====================================
	// Events
	// ====================================

	/** Level up event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|LifeContent")
	FOnLifeContentLevelUp OnLevelUp;

	/** Activity started event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|LifeContent")
	FOnLifeContentActivityStarted OnActivityStarted;

	/** Activity cancelled event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|LifeContent")
	FOnLifeContentActivityCancelled OnActivityCancelled;

	/** Activity completed event */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|LifeContent")
	FOnLifeContentActivityCompleted OnActivityCompleted;

	// ====================================
	// Settings
	// ====================================

	/** Experience multiplier for balancing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeContent Settings")
	float ExperienceMultiplier = 1.0f;

	/** Base experience required per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeContent Settings")
	int32 BaseExperiencePerLevel = 100;

	/** Experience curve exponent (higher = steeper curve) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeContent Settings")
	float ExperienceCurveExponent = 1.5f;

protected:
	// ====================================
	// Protected Functions for Subclasses
	// ====================================

	/** Called when activity completes - subclasses should override to implement specific logic */
	virtual void OnActivityComplete();

	/** Called when level up occurs - subclasses can override for additional logic */
	virtual void OnLevelUpInternal(int32 NewLevel);

	/** Check and process level up */
	void CheckAndProcessLevelUp();

	/** Get elapsed time since activity started */
	float GetElapsedActivityTime() const;

	// ====================================
	// Protected State Variables
	// ====================================

	/** Whether activity is currently active */
	UPROPERTY()
	bool bIsActivityActive = false;

	/** Activity start time */
	UPROPERTY()
	float ActivityStartTime = 0.0f;

	/** Required time for activity completion (0 = no time limit) */
	UPROPERTY()
	float RequiredActivityTime = 0.0f;

	/** Current level */
	UPROPERTY()
	int32 Level = 1;

	/** Current experience points */
	UPROPERTY()
	int32 CurrentExperience = 0;

private:
	/** Handle activity timing and auto-completion */
	void UpdateActivityTiming(float DeltaTime);
};
