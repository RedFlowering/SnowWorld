// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaLeaderboardSystemDefinitions.h"
#include "HarmoniaLeaderboardSubsystem.generated.h"

/**
 * Subsystem for managing leaderboards
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaLeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Submit a score to a leaderboard
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Leaderboard")
	void SubmitScore(const FHarmoniaID& LeaderboardId, int64 Score);

	// Get top N entries from a leaderboard
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Leaderboard")
	void GetTopEntries(const FHarmoniaID& LeaderboardId, int32 Count);

	// Get entries around the player
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Leaderboard")
	void GetFriendsEntries(const FHarmoniaID& LeaderboardId);

protected:
	// Internal helper for Steam integration
	void InternalSubmitScore(const FString& LeaderboardName, int64 Score);
};
