// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaQuestSystemDefinitions.h"
#include "Engine/DataTable.h"
#include "HarmoniaQuestSubsystem.generated.h"

/**
 * HarmoniaQuestSubsystem
 *
 * Game-wide quest management subsystem
 * Provides centralized access to quest data and quest-related utilities
 *
 * Features:
 * - Centralized quest data management
 * - Quest lookup and caching
 * - Quest chain validation
 * - Daily/Weekly quest reset tracking
 * - Quest statistics and analytics
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaQuestSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~==============================================
	//~ Quest Data Access
	//~==============================================
public:
	/**
	 * Set the quest data table
	 * @param DataTable - Quest data table to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem")
	void SetQuestDataTable(UDataTable* DataTable);

	/**
	 * Get quest data by ID
	 * @param QuestId - Quest ID to lookup
	 * @param OutQuestData - Output quest data
	 * @return True if quest was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem")
	bool GetQuestData(FHarmoniaID QuestId, FHarmoniaQuestData& OutQuestData) const;

	/**
	 * Get all quests of a specific type
	 * @param QuestType - Type to filter by
	 * @return Array of quest data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem")
	TArray<FHarmoniaQuestData> GetQuestsByType(EQuestType QuestType) const;

	/**
	 * Get all quests with a specific category tag
	 * @param CategoryTag - Category tag to filter by
	 * @return Array of quest data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem")
	TArray<FHarmoniaQuestData> GetQuestsByCategory(FGameplayTag CategoryTag) const;

	/**
	 * Get all quests
	 * @return Array of all quest data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem")
	TArray<FHarmoniaQuestData> GetAllQuests() const;

	/**
	 * Check if a quest exists
	 * @param QuestId - Quest ID to check
	 * @return True if quest exists in data table
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Subsystem")
	bool DoesQuestExist(FHarmoniaID QuestId) const;

	//~==============================================
	//~ Quest Chain Validation
	//~==============================================
public:
	/**
	 * Get all prerequisite quests for a quest
	 * @param QuestId - Quest to check
	 * @param OutPrerequisites - Output array of prerequisite quest IDs
	 * @return True if quest was found and has prerequisites
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Chain")
	bool GetPrerequisiteQuests(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutPrerequisites) const;

	/**
	 * Get all quests that unlock after completing a quest
	 * @param QuestId - Quest to check
	 * @return Array of quest IDs that unlock
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Chain")
	TArray<FHarmoniaID> GetUnlockedQuests(FHarmoniaID QuestId) const;

	/**
	 * Get entire quest chain starting from a quest
	 * @param QuestId - Starting quest ID
	 * @param OutQuestChain - Output array of quest IDs in chain order
	 * @param bIncludeStartQuest - Whether to include the start quest in output
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Chain")
	void GetQuestChain(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutQuestChain, bool bIncludeStartQuest = true) const;

	/**
	 * Validate quest chain (check for circular dependencies)
	 * @param QuestId - Quest to validate
	 * @return True if quest chain is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Chain")
	bool ValidateQuestChain(FHarmoniaID QuestId) const;

	//~==============================================
	//~ Daily/Weekly Quest Reset
	//~==============================================
public:
	/**
	 * Get time until next daily quest reset
	 * @return Time in seconds until reset
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Subsystem|Reset")
	float GetTimeUntilDailyReset() const;

	/**
	 * Get time until next weekly quest reset
	 * @return Time in seconds until reset
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Subsystem|Reset")
	float GetTimeUntilWeeklyReset() const;

	/**
	 * Check if daily quests should reset
	 * @return True if daily quests should reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Reset")
	bool ShouldResetDailyQuests() const;

	/**
	 * Check if weekly quests should reset
	 * @return True if weekly quests should reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Reset")
	bool ShouldResetWeeklyQuests() const;

	/**
	 * Mark daily reset as completed
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Reset")
	void MarkDailyResetCompleted();

	/**
	 * Mark weekly reset as completed
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Reset")
	void MarkWeeklyResetCompleted();

	//~==============================================
	//~ Quest Statistics
	//~==============================================
public:
	/**
	 * Get total number of quests
	 * @return Total quest count
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Subsystem|Stats")
	int32 GetTotalQuestCount() const;

	/**
	 * Get number of quests by type
	 * @param QuestType - Type to count
	 * @return Quest count for this type
	 */
	UFUNCTION(BlueprintPure, Category = "Quest|Subsystem|Stats")
	int32 GetQuestCountByType(EQuestType QuestType) const;

	/**
	 * Get recommended quests for a player level
	 * @param PlayerLevel - Player's current level
	 * @param LevelRange - Level range to consider (e.g., 5 = level ±5)
	 * @return Array of recommended quest data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Stats")
	TArray<FHarmoniaQuestData> GetRecommendedQuestsForLevel(int32 PlayerLevel, int32 LevelRange = 5) const;

	//~==============================================
	//~ Quest Search and Filtering
	//~==============================================
public:
	/**
	 * Search quests by name (case-insensitive, partial match)
	 * @param SearchText - Text to search for
	 * @return Array of matching quest data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Search")
	TArray<FHarmoniaQuestData> SearchQuestsByName(const FString& SearchText) const;

	/**
	 * Find quests with specific objective type
	 * @param ObjectiveType - Objective type to search for
	 * @return Array of quest data containing this objective type
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Search")
	TArray<FHarmoniaQuestData> FindQuestsWithObjectiveType(EQuestObjectiveType ObjectiveType) const;

	/**
	 * Find quests that reward a specific item
	 * @param ItemId - Item ID to search for
	 * @return Array of quest data that reward this item
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Search")
	TArray<FHarmoniaQuestData> FindQuestsRewardingItem(FHarmoniaID ItemId) const;

	/**
	 * Find quests that require a specific item
	 * @param ItemId - Item ID to search for
	 * @return Array of quest data that require this item
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Search")
	TArray<FHarmoniaQuestData> FindQuestsRequiringItem(FHarmoniaID ItemId) const;

	//~==============================================
	//~ Debug and Development
	//~==============================================
public:
#if WITH_EDITOR
	/**
	 * Validate all quests in data table
	 * Checks for:
	 * - Circular quest chains
	 * - Invalid references
	 * - Missing prerequisites
	 * @param OutErrors - Output array of error messages
	 * @return True if all quests are valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Debug")
	bool ValidateAllQuests(TArray<FString>& OutErrors) const;

	/**
	 * Print quest dependency graph
	 * Useful for visualizing quest chains
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Debug")
	void PrintQuestDependencyGraph() const;

	/**
	 * Get quest statistics report
	 * @return Formatted string with quest statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Subsystem|Debug")
	FString GetQuestStatisticsReport() const;
#endif

protected:
	//~==============================================
	//~ Internal Data
	//~==============================================

	/** Quest data table reference */
	UPROPERTY()
	UDataTable* QuestDataTable;

	/** Cached quest data for fast lookups */
	UPROPERTY()
	TMap<FHarmoniaID, FHarmoniaQuestData> QuestDataCache;

	/** Last daily reset timestamp */
	UPROPERTY()
	FDateTime LastDailyResetTime;

	/** Last weekly reset timestamp */
	UPROPERTY()
	FDateTime LastWeeklyResetTime;

	/** Daily reset hour (0-23) */
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Config")
	int32 DailyResetHour = 0;

	/** Weekly reset day (0=Sunday, 6=Saturday) */
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Config")
	int32 WeeklyResetDay = 1;

	/** Weekly reset hour (0-23) */
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Config")
	int32 WeeklyResetHour = 0;

	//~==============================================
	//~ Internal Functions
	//~==============================================

	/**
	 * Cache all quest data from data table
	 */
	void CacheQuestData();

	/**
	 * Clear quest data cache
	 */
	void ClearQuestDataCache();

	/**
	 * Get next daily reset time
	 */
	FDateTime GetNextDailyResetTime() const;

	/**
	 * Get next weekly reset time
	 */
	FDateTime GetNextWeeklyResetTime() const;

	/**
	 * Recursive helper for getting quest chain
	 */
	void GetQuestChainRecursive(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutQuestChain, TSet<FHarmoniaID>& VisitedQuests) const;

	/**
	 * Recursive helper for validating quest chain (circular dependency check)
	 */
	bool ValidateQuestChainRecursive(FHarmoniaID QuestId, TSet<FHarmoniaID>& VisitedQuests, TSet<FHarmoniaID>& RecursionStack) const;
};
