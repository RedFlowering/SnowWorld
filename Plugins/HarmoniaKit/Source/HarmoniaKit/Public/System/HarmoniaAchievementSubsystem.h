// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaAchievementSystemDefinitions.h"
#include "HarmoniaAchievementSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, const FHarmoniaID&, AchievementId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementProgressUpdated, const FHarmoniaID&, AchievementId, int32, NewProgress);

/**
 * Subsystem for managing achievements
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Unlock an achievement
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Achievement")
	void UnlockAchievement(const FHarmoniaID& AchievementId);

	// Update progress for a progressive achievement
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Achievement")
	void UpdateProgress(const FHarmoniaID& AchievementId, int32 Amount);

	// Get the current state of an achievement
	UFUNCTION(BlueprintPure, Category = "Harmonia|Achievement")
	EAchievementState GetAchievementState(const FHarmoniaID& AchievementId) const;

	// Get current progress
	UFUNCTION(BlueprintPure, Category = "Harmonia|Achievement")
	int32 GetAchievementProgress(const FHarmoniaID& AchievementId) const;

	// Get achievement definition
	UFUNCTION(BlueprintPure, Category = "Harmonia|Achievement")
	bool GetAchievementDefinition(const FHarmoniaID& AchievementId, FHarmoniaAchievementData& OutDefinition) const;

	// Event delegates
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Achievement")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Achievement")
	FOnAchievementProgressUpdated OnAchievementProgressUpdated;

protected:
	// Load achievement data
	void LoadAchievementData();

	// Internal helper to find state
	FAchievementState* FindAchievementState(const FHarmoniaID& AchievementId);

protected:
	// Map of Achievement ID to State
	UPROPERTY()
	TMap<FHarmoniaID, FAchievementState> AchievementStates;

	// Data table reference (to be set in Blueprints or loaded)
	UPROPERTY(EditDefaultsOnly, Category = "Harmonia|Achievement")
	TSoftObjectPtr<UDataTable> AchievementDataTable;
};
