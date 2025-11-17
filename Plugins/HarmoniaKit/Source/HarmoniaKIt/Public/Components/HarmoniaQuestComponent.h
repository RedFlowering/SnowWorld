// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaQuestSystemDefinitions.h"
#include "Engine/DataTable.h"
#include "HarmoniaQuestComponent.generated.h"

// Forward declarations
class UHarmoniaInventoryComponent;
class UHarmoniaCraftingComponent;
class UDataTable;

/**
 * Delegates for quest events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAdded, FHarmoniaID, QuestId, const FQuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStarted, FHarmoniaID, QuestId, const FQuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestObjectiveUpdated, FHarmoniaID, QuestId, int32, ObjectiveIndex, const FQuestObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestReadyToComplete, FHarmoniaID, QuestId, const FQuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnQuestCompleted, FHarmoniaID, QuestId, const FQuestData&, QuestData, const TArray<FQuestReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestFailed, FHarmoniaID, QuestId, const FQuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestAbandoned, FHarmoniaID, QuestId, const FQuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUnlocked, FHarmoniaID, QuestId, const FQuestData&, QuestData);

/**
 * HarmoniaQuestComponent
 *
 * Manages quest system with:
 * - Quest tracking and progression
 * - Multiple objective types (Kill, Collect, Talk, Reach, Craft, Build, etc.)
 * - Quest conditions and prerequisites
 * - Quest rewards and optional rewards
 * - Quest chains and scenario support
 * - Time-limited quests
 * - Daily/Weekly/Repeatable quests
 * - Save/Load support
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaQuestComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	//~==============================================
	//~ Quest Data Configuration
	//~==============================================
public:
	/** Quest data table reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
	UDataTable* QuestDataTable;

	//~==============================================
	//~ Quest State
	//~==============================================
protected:
	/** Active quests in progress */
	UPROPERTY(ReplicatedUsing = OnRep_ActiveQuests, BlueprintReadOnly, Category = "Quest")
	TArray<FActiveQuestProgress> ActiveQuests;

	/** Completed quest IDs */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Quest")
	TArray<FHarmoniaID> CompletedQuests;

	/** Failed quest IDs */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Quest")
	TArray<FHarmoniaID> FailedQuests;

	/** Currently tracked quest (displayed prominently in UI) */
	UPROPERTY(ReplicatedUsing = OnRep_TrackedQuest, BlueprintReadOnly, Category = "Quest")
	FHarmoniaID TrackedQuest;

	/** Cached inventory component reference */
	UPROPERTY()
	UHarmoniaInventoryComponent* InventoryComponent;

	/** Cached crafting component reference */
	UPROPERTY()
	UHarmoniaCraftingComponent* CraftingComponent;

	UFUNCTION()
	void OnRep_ActiveQuests();

	UFUNCTION()
	void OnRep_TrackedQuest();

	//~==============================================
	//~ Quest Operations
	//~==============================================
public:
	/**
	 * Start a quest (accept quest from NPC)
	 * @param QuestId - Quest to start
	 * @return True if quest was started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool StartQuest(FHarmoniaID QuestId);

	/**
	 * Complete a quest and receive rewards
	 * @param QuestId - Quest to complete
	 * @param SelectedOptionalRewards - Optional rewards to claim (indices)
	 * @return True if quest was completed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuest(FHarmoniaID QuestId, const TArray<int32>& SelectedOptionalRewards = TArray<int32>());

	/**
	 * Abandon a quest
	 * @param QuestId - Quest to abandon
	 * @return True if quest was abandoned successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AbandonQuest(FHarmoniaID QuestId);

	/**
	 * Fail a quest (called by game logic, not player)
	 * @param QuestId - Quest to fail
	 * @return True if quest was failed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool FailQuest(FHarmoniaID QuestId);

	/**
	 * Update quest objective progress
	 * @param QuestId - Quest to update
	 * @param ObjectiveIndex - Objective index to update
	 * @param Progress - Progress to add (can be negative)
	 * @return True if objective was updated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool UpdateQuestObjective(FHarmoniaID QuestId, int32 ObjectiveIndex, int32 Progress);

	/**
	 * Update quest objective by type and target
	 * Automatically finds matching objectives and updates them
	 * @param ObjectiveType - Type of objective
	 * @param TargetId - Target ID (enemy type, item ID, etc.)
	 * @param Progress - Progress to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateQuestObjectivesByType(EQuestObjectiveType ObjectiveType, FHarmoniaID TargetId, int32 Progress = 1);

	/**
	 * Update quest objective by gameplay tags
	 * @param ObjectiveType - Type of objective
	 * @param TargetTags - Tags to match
	 * @param Progress - Progress to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateQuestObjectivesByTags(EQuestObjectiveType ObjectiveType, FGameplayTagContainer TargetTags, int32 Progress = 1);

	/**
	 * Set tracked quest (displayed prominently in UI)
	 * @param QuestId - Quest to track (invalid = untrack all)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTrackedQuest(FHarmoniaID QuestId);

	//~==============================================
	//~ Quest Queries
	//~==============================================
public:
	/**
	 * Get quest data by ID
	 * @param QuestId - Quest ID to lookup
	 * @param OutQuestData - Output quest data
	 * @return True if quest was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetQuestData(FHarmoniaID QuestId, FQuestData& OutQuestData) const;

	/**
	 * Get active quest progress
	 * @param QuestId - Quest ID to lookup
	 * @param OutProgress - Output quest progress
	 * @return True if quest is active
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetActiveQuestProgress(FHarmoniaID QuestId, FActiveQuestProgress& OutProgress) const;

	/**
	 * Get all active quests
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FActiveQuestProgress> GetActiveQuests() const { return ActiveQuests; }

	/**
	 * Get all completed quests
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FHarmoniaID> GetCompletedQuests() const { return CompletedQuests; }

	/**
	 * Get all failed quests
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FHarmoniaID> GetFailedQuests() const { return FailedQuests; }

	/**
	 * Get currently tracked quest ID
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	FHarmoniaID GetTrackedQuest() const { return TrackedQuest; }

	/**
	 * Check if quest is active
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestActive(FHarmoniaID QuestId) const;

	/**
	 * Check if quest is completed
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestCompleted(FHarmoniaID QuestId) const;

	/**
	 * Check if quest is failed
	 */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestFailed(FHarmoniaID QuestId) const;

	/**
	 * Check if quest is ready to complete (all objectives done)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestReadyToComplete(FHarmoniaID QuestId) const;

	/**
	 * Get available quests (unlocked but not started)
	 * @return Array of available quest IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FHarmoniaID> GetAvailableQuests() const;

	/**
	 * Check if quest is available (conditions met)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestAvailable(FHarmoniaID QuestId) const;

	/**
	 * Get quest state
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	EQuestState GetQuestState(FHarmoniaID QuestId) const;

	//~==============================================
	//~ Quest Conditions
	//~==============================================
protected:
	/**
	 * Check if quest conditions are met
	 * @param Conditions - Conditions to check
	 * @return True if all conditions are met
	 */
	bool CheckQuestConditions(const TArray<FQuestCondition>& Conditions) const;

	/**
	 * Check a single quest condition
	 * @param Condition - Condition to check
	 * @return True if condition is met
	 */
	bool CheckQuestCondition(const FQuestCondition& Condition) const;

	//~==============================================
	//~ Quest Rewards
	//~==============================================
protected:
	/**
	 * Grant quest rewards to player
	 * @param Rewards - Rewards to grant
	 */
	void GrantRewards(const TArray<FQuestReward>& Rewards);

	/**
	 * Grant a single reward
	 * @param Reward - Reward to grant
	 */
	void GrantReward(const FQuestReward& Reward);

	//~==============================================
	//~ Quest Chain System
	//~==============================================
protected:
	/**
	 * Check and start next quests in chain
	 * @param CompletedQuestId - Quest that was just completed
	 */
	void CheckQuestChain(FHarmoniaID CompletedQuestId);

	//~==============================================
	//~ Time-Limited Quests
	//~==============================================
protected:
	/**
	 * Update time-limited quests
	 * @param DeltaTime - Time elapsed since last tick
	 */
	void UpdateTimeLimitedQuests(float DeltaTime);

	/**
	 * Check if quest time limit expired
	 * @param QuestId - Quest to check
	 * @param Progress - Quest progress
	 * @return True if time limit expired
	 */
	bool CheckQuestTimeLimit(FHarmoniaID QuestId, const FActiveQuestProgress& Progress);

	//~==============================================
	//~ Server/Client RPCs
	//~==============================================
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartQuest(FHarmoniaID QuestId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCompleteQuest(FHarmoniaID QuestId, const TArray<int32>& SelectedOptionalRewards);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAbandonQuest(FHarmoniaID QuestId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUpdateObjective(FHarmoniaID QuestId, int32 ObjectiveIndex, int32 Progress);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetTrackedQuest(FHarmoniaID QuestId);

	UFUNCTION(Client, Reliable)
	void ClientQuestStarted(FHarmoniaID QuestId);

	UFUNCTION(Client, Reliable)
	void ClientQuestCompleted(FHarmoniaID QuestId, const TArray<FQuestReward>& Rewards);

	UFUNCTION(Client, Reliable)
	void ClientQuestFailed(FHarmoniaID QuestId);

	UFUNCTION(Client, Reliable)
	void ClientQuestAbandoned(FHarmoniaID QuestId);

	UFUNCTION(Client, Reliable)
	void ClientObjectiveUpdated(FHarmoniaID QuestId, int32 ObjectiveIndex);

	//~==============================================
	//~ Delegates
	//~==============================================
public:
	/** Called when a quest is added to available quests */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestAdded OnQuestAdded;

	/** Called when a quest is started */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestStarted OnQuestStarted;

	/** Called when a quest objective is updated */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

	/** Called when a quest becomes ready to complete */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestReadyToComplete OnQuestReadyToComplete;

	/** Called when a quest is completed */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestCompleted OnQuestCompleted;

	/** Called when a quest is failed */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestFailed OnQuestFailed;

	/** Called when a quest is abandoned */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestAbandoned OnQuestAbandoned;

	/** Called when a quest is unlocked (becomes available) */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
	FOnQuestUnlocked OnQuestUnlocked;

	//~==============================================
	//~ Save/Load System
	//~==============================================
public:
	/**
	 * Get quest save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	FQuestSaveData GetQuestSaveData() const;

	/**
	 * Load quest data from save
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void LoadQuestFromSaveData(const FQuestSaveData& SaveData);

	//~==============================================
	//~ Utility Functions
	//~==============================================
protected:
	/**
	 * Get inventory component (cached)
	 */
	UHarmoniaInventoryComponent* GetInventoryComponent();

	/**
	 * Get crafting component (cached)
	 */
	UHarmoniaCraftingComponent* GetCraftingComponent();

	/**
	 * Get character level (override in subclass if you have level system)
	 */
	virtual int32 GetCharacterLevel() const;

	/**
	 * Get character gameplay tags (for condition checking)
	 */
	virtual FGameplayTagContainer GetCharacterTags() const;

	/**
	 * Find active quest by ID
	 */
	FActiveQuestProgress* FindActiveQuest(FHarmoniaID QuestId);
	const FActiveQuestProgress* FindActiveQuest(FHarmoniaID QuestId) const;

	/**
	 * Check auto-complete for a quest
	 */
	void CheckAutoComplete(FHarmoniaID QuestId);

	//~==============================================
	//~ Security & Anti-Cheat
	//~==============================================
protected:
	/** Security: Minimum time between quest operations */
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Security")
	float MinTimeBetweenOperations;

	/** Security: Maximum active quests at once */
	UPROPERTY(EditDefaultsOnly, Category = "Quest|Security")
	int32 MaxActiveQuests;

	/** Rate limiting: Last operation time */
	UPROPERTY()
	float LastOperationTime;

	//~==============================================
	//~ Debug
	//~==============================================
public:
#if WITH_EDITOR
	/**
	 * Debug: Complete all objectives of a quest instantly
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
	void Debug_CompleteQuestObjectives(FHarmoniaID QuestId);

	/**
	 * Debug: Unlock all quests
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
	void Debug_UnlockAllQuests();

	/**
	 * Debug: Reset all quest progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
	void Debug_ResetAllQuests();
#endif
};
