// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaGameService.h
 * @brief Main game service subsystem for platform integration
 * 
 * This file contains UHarmoniaGameService, a GameInstanceSubsystem that provides
 * unified access to platform services across Steam, Epic, Xbox, PlayStation,
 * Nintendo Switch, and GOG platforms.
 * 
 * Features:
 * - Achievement unlocking and progress tracking
 * - Cloud save upload/download with conflict resolution
 * - Leaderboard score submission and queries
 * - Player statistics tracking with auto-upload
 * - DLC/Content entitlement verification
 * - Cross-play session management
 * 
 * @see UHarmoniaGameService
 * @see HarmoniaGameServiceDefinitions.h
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaGameServiceDefinitions.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "Interfaces/OnlineUserCloudInterface.h"
#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "HarmoniaGameService.generated.h"

class IOnlineSubsystem;
class FOnlineAchievementsWrite;
class FOnlineLeaderboardWrite;

// Delegates for async operations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaAchievementUnlocked, FName, AchievementId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaAchievementsQueried, const TArray<FHarmoniaServiceAchievementData>&, Achievements, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaCloudSaveSynced, const FString&, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHarmoniaLeaderboardScoreUploaded, FName, LeaderboardId, int64, Score, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaLeaderboardQueried, const TArray<FHarmoniaLeaderboardEntry>&, Entries, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaStatisticsUpdated, const FHarmoniaPlayerStatistics&, Stats, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaDLCOwnershipChecked, FName, DLCId, EHarmoniaDLCOwnershipStatus, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaSessionJoined, const FString&, SessionId, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHarmoniaSessionCreated, const FString&, SessionId, bool, bSuccess);

/**
 * HarmoniaGameService - Game Service Platform Integration Layer
 * Provides unified interface for achievements, cloud saves, leaderboards, statistics,
 * DLC management, and cross-play sessions across Steam, Epic, Xbox, and other platforms.
 */
UCLASS(Config=Game)
class HARMONIAGAMESERVICE_API UHarmoniaGameService : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaGameService();

	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==================== PLATFORM DETECTION ====================

	/**
	 * Get the current platform type
	 * @return The platform the game is running on
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Platform")
	EHarmoniaPlatform GetCurrentPlatform() const;

	/**
	 * Check if a specific platform feature is available
	 * @param Platform The platform to check
	 * @return True if the platform subsystem is available
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Platform")
	bool IsPlatformAvailable(EHarmoniaPlatform Platform) const;

	// ==================== ACHIEVEMENTS ====================

	/**
	 * Unlock an achievement
	 * @param AchievementId The achievement identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Achievements")
	void UnlockAchievement(FName AchievementId);

	/**
	 * Update achievement progress (for progressive achievements)
	 * @param AchievementId The achievement identifier
	 * @param Progress Progress value (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Achievements")
	void UpdateAchievementProgress(FName AchievementId, float Progress);

	/**
	 * Query all achievements for the current player
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Achievements")
	void QueryAchievements();

	/**
	 * Get cached achievement data
	 * @param AchievementId The achievement identifier
	 * @param OutAchievement The achievement data
	 * @return True if the achievement was found in cache
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Achievements")
	bool GetAchievementData(FName AchievementId, FHarmoniaServiceAchievementData& OutAchievement) const;

	/**
	 * Get all cached achievements
	 * @return Array of all achievement data
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Achievements")
	TArray<FHarmoniaServiceAchievementData> GetAllAchievements() const;

	// Achievement events
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaAchievementsQueried OnAchievementsQueried;

	// ==================== CLOUD SAVE ====================

	/**
	 * Upload save data to the cloud
	 * @param SlotName Save slot identifier
	 * @param SaveData Binary save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|CloudSave")
	void UploadCloudSave(const FString& SlotName, const TArray<uint8>& SaveData);

	/**
	 * Download save data from the cloud
	 * @param SlotName Save slot identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|CloudSave")
	void DownloadCloudSave(const FString& SlotName);

	/**
	 * Synchronize local and cloud saves (auto-resolve conflicts)
	 * @param SlotName Save slot identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|CloudSave")
	void SyncCloudSave(const FString& SlotName);

	/**
	 * Delete cloud save
	 * @param SlotName Save slot identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|CloudSave")
	void DeleteCloudSave(const FString& SlotName);

	/**
	 * Get cloud save status
	 * @param SlotName Save slot identifier
	 * @return Current sync status
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|CloudSave")
	EHarmoniaCloudSaveStatus GetCloudSaveStatus(const FString& SlotName) const;

	// Cloud save events
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaCloudSaveSynced OnCloudSaveSynced;

	// ==================== LEADERBOARDS ====================

	/**
	 * Upload a score to a leaderboard
	 * @param LeaderboardId Leaderboard identifier
	 * @param Score Score value (time in ms for boss fights, etc.)
	 * @param UpdateMethod How to handle score updates
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Leaderboards")
	void UploadLeaderboardScore(FName LeaderboardId, int64 Score,
	                            EHarmoniaLeaderboardUpdateMethod UpdateMethod = EHarmoniaLeaderboardUpdateMethod::KeepBest);

	/**
	 * Query leaderboard entries
	 * @param LeaderboardId Leaderboard identifier
	 * @param StartRank Starting rank (1-based)
	 * @param EntryCount Number of entries to retrieve
	 * @param TimeRange Time range filter
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Leaderboards")
	void QueryLeaderboard(FName LeaderboardId, int32 StartRank = 1, int32 EntryCount = 10,
	                      EHarmoniaLeaderboardTimeRange TimeRange = EHarmoniaLeaderboardTimeRange::AllTime);

	/**
	 * Query leaderboard around the current player
	 * @param LeaderboardId Leaderboard identifier
	 * @param Range Number of entries above and below player
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Leaderboards")
	void QueryLeaderboardAroundPlayer(FName LeaderboardId, int32 Range = 5);

	/**
	 * Query leaderboard for friends only
	 * @param LeaderboardId Leaderboard identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Leaderboards")
	void QueryLeaderboardFriends(FName LeaderboardId);

	// Leaderboard events
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaLeaderboardScoreUploaded OnLeaderboardScoreUploaded;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaLeaderboardQueried OnLeaderboardQueried;

	// ==================== STATISTICS ====================

	/**
	 * Increment a statistic value
	 * @param StatName Statistic name
	 * @param Amount Amount to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Statistics")
	void IncrementStat(FName StatName, int64 Amount = 1);

	/**
	 * Set a statistic value directly
	 * @param StatName Statistic name
	 * @param Value New value
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Statistics")
	void SetStat(FName StatName, int64 Value);

	/**
	 * Get a statistic value
	 * @param StatName Statistic name
	 * @return Current value
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Statistics")
	int64 GetStat(FName StatName) const;

	/**
	 * Get all player statistics
	 * @return Complete statistics structure
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Statistics")
	FHarmoniaPlayerStatistics GetPlayerStatistics() const;

	/**
	 * Upload statistics to the platform
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Statistics")
	void UploadStatistics();

	/**
	 * Query statistics from the platform
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Statistics")
	void QueryStatistics();

	// Statistics events
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaStatisticsUpdated OnStatisticsUpdated;

	// ==================== DLC / CONTENT OWNERSHIP ====================

	/**
	 * Check DLC ownership status
	 * @param DLCId DLC identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|DLC")
	void CheckDLCOwnership(FName DLCId);

	/**
	 * Check if DLC is owned (from cache)
	 * @param DLCId DLC identifier
	 * @return True if owned
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|DLC")
	bool IsDLCOwned(FName DLCId) const;

	/**
	 * Get DLC data
	 * @param DLCId DLC identifier
	 * @param OutDLCData DLC data structure
	 * @return True if DLC data was found
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|DLC")
	bool GetDLCData(FName DLCId, FHarmoniaDLCData& OutDLCData) const;

	/**
	 * Query all owned DLC
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|DLC")
	void QueryOwnedDLC();

	// DLC events
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaDLCOwnershipChecked OnDLCOwnershipChecked;

	// ==================== CROSS-PLAY SESSIONS ====================

	/**
	 * Create a cross-play session
	 * @param SessionName Session name
	 * @param MaxPlayers Maximum players
	 * @param Joinability Session joinability
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Sessions")
	void CreateCrossPlaySession(const FString& SessionName, int32 MaxPlayers = 4,
	                            EHarmoniaSessionJoinability Joinability = EHarmoniaSessionJoinability::Public);

	/**
	 * Find available cross-play sessions
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Sessions")
	void FindCrossPlaySessions();

	/**
	 * Join a cross-play session
	 * @param SessionId Session identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Sessions")
	void JoinCrossPlaySession(const FString& SessionId);

	/**
	 * Leave the current session
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|GameService|Sessions")
	void LeaveCrossPlaySession();

	/**
	 * Get current session info
	 * @param OutSession Session data
	 * @return True if in a session
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|GameService|Sessions")
	bool GetCurrentSession(FHarmoniaCrossPlaySession& OutSession) const;

	// Session events
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|GameService|Events")
	FOnHarmoniaSessionJoined OnSessionJoined;

protected:
	// Internal platform detection
	void DetectCurrentPlatform();
	IOnlineSubsystem* GetPlatformSubsystem(EHarmoniaPlatform Platform) const;
	IOnlineSubsystem* GetCurrentPlatformSubsystem() const;

	// Achievement helpers
	void OnAchievementWriteComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful);
	void OnAchievementQueryComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful);

	// Cloud save helpers
	void OnCloudSaveWriteComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);
	void OnCloudSaveReadComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);
	void OnCloudSaveDeleteComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

	// Leaderboard helpers
	void OnLeaderboardWriteComplete(const FName& SessionName, bool bWasSuccessful);
	void OnLeaderboardReadComplete(bool bWasSuccessful);

	// Statistics helpers
	void OnStatsWriteComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful);
	void OnStatsReadComplete(const FUniqueNetId& PlayerId, bool bWasSuccessful);

	// Session helpers
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnFindSessionsComplete(bool bWasSuccessful);

private:
	// Current platform
	UPROPERTY()
	EHarmoniaPlatform CurrentPlatform = EHarmoniaPlatform::Unknown;

	// Achievement cache
	UPROPERTY()
	TMap<FName, FHarmoniaServiceAchievementData> AchievementCache;

	// Cloud save cache
	UPROPERTY()
	TMap<FString, FHarmoniaCloudSaveData> CloudSaveCache;

	// DLC cache
	UPROPERTY()
	TMap<FName, FHarmoniaDLCData> DLCCache;

	// Player statistics
	UPROPERTY()
	FHarmoniaPlayerStatistics PlayerStats;

	// Current session
	UPROPERTY()
	FHarmoniaCrossPlaySession CurrentSession;

	// Cached downloaded save data
	TMap<FString, TArray<uint8>> DownloadedSaveData;

	// Cached leaderboard entries
	TArray<FHarmoniaLeaderboardEntry> CachedLeaderboardEntries;

	// Session search results
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	// Delegate handles
	FDelegateHandle AchievementWriteDelegateHandle;
	FDelegateHandle AchievementQueryDelegateHandle;
	FDelegateHandle CloudSaveWriteDelegateHandle;
	FDelegateHandle CloudSaveReadDelegateHandle;
	FDelegateHandle CloudSaveDeleteDelegateHandle;
	FDelegateHandle LeaderboardWriteDelegateHandle;
	FDelegateHandle LeaderboardReadDelegateHandle;
	FDelegateHandle StatsWriteDelegateHandle;
	FDelegateHandle StatsReadDelegateHandle;
	FDelegateHandle CreateSessionDelegateHandle;
	FDelegateHandle JoinSessionDelegateHandle;
	FDelegateHandle FindSessionsDelegateHandle;

	// Configuration
	UPROPERTY(Config)
	bool bEnableAchievements = true;

	UPROPERTY(Config)
	bool bEnableCloudSaves = true;

	UPROPERTY(Config)
	bool bEnableLeaderboards = true;

	UPROPERTY(Config)
	bool bEnableStatistics = true;

	UPROPERTY(Config)
	bool bEnableCrossPlay = true;

	UPROPERTY(Config)
	float StatUploadIntervalSeconds = 300.0f;

	// Auto-upload timer
	FTimerHandle StatUploadTimerHandle;
};
