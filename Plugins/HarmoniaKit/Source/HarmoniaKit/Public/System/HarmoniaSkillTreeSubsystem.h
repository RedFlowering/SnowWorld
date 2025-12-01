// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaSkillTreeSubsystem.generated.h"

class UHarmoniaSkillTreeNode;
class UGameplayAbility;

/**
 * Skill node state
 */
UENUM(BlueprintType)
enum class EHarmoniaSkillNodeState : uint8
{
	Locked,        // Cannot be unlocked yet (prerequisites not met)
	Available,     // Can be unlocked (prerequisites met, has points)
	Unlocked,      // Unlocked but not maxed
	Maxed          // Reached maximum level
};

/**
 * Single skill node definition (local version for subsystem)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSkillNodeData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this node */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FText Description;

	/** Icon for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Node type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EHarmoniaSkillNodeType NodeType = EHarmoniaSkillNodeType::Passive;

	/** Skill tree this node belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FGameplayTag SkillTreeTag;

	/** Maximum level for this node */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxLevel = 1;

	/** Skill points required per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TArray<int32> PointCostPerLevel;

	/** Character level required to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredCharacterLevel = 1;

	/** Prerequisite node IDs (all must be unlocked) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> PrerequisiteNodes;

	/** Minimum total points invested in tree before this can be unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 RequiredTreePointsInvested = 0;

	/** For Active type: Ability class granted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (EditCondition = "NodeType == EHarmoniaSkillNodeType::Active"))
	TSubclassOf<UGameplayAbility> GrantedAbility;

	/** For Passive type: Gameplay effect applied per level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects", meta = (EditCondition = "NodeType == EHarmoniaSkillNodeType::Passive"))
	TSubclassOf<class UGameplayEffect> PassiveEffect;

	/** Tags applied when this node is unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FGameplayTagContainer GrantedTags;

	/** Position in skill tree UI (grid-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FVector2D GridPosition;

	/** Connections to other nodes (for visual lines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FName> ConnectedNodes;

	/** Get point cost for a specific level (1-indexed) */
	int32 GetPointCostForLevel(int32 Level) const
	{
		if (PointCostPerLevel.Num() == 0) return 1;
		int32 Index = FMath::Clamp(Level - 1, 0, PointCostPerLevel.Num() - 1);
		return PointCostPerLevel[Index];
	}

	/** Get total points to max this node */
	int32 GetTotalPointCost() const
	{
		int32 Total = 0;
		for (int32 i = 1; i <= MaxLevel; ++i)
		{
			Total += GetPointCostForLevel(i);
		}
		return Total;
	}
};

/**
 * Skill tree definition (subsystem local version)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSkillTreeDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique tag for this skill tree */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree")
	FGameplayTag TreeTag;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree")
	FText Description;

	/** Icon for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Background image for skill tree UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree")
	TSoftObjectPtr<UTexture2D> BackgroundImage;

	/** All nodes in this tree */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree")
	TArray<FHarmoniaSkillNodeData> Nodes;
};

/**
 * Runtime skill node instance (player progress)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSkillNodeProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	FName NodeID;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	int32 CurrentLevel = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	EHarmoniaSkillNodeState State = EHarmoniaSkillNodeState::Locked;

	/** Handle for the granted ability (runtime only) */
	UPROPERTY(Transient)
	FGameplayAbilitySpecHandle GrantedAbilityHandle;
};

/**
 * Player's skill tree progress
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSkillTreeProgress
{
	GENERATED_BODY()

	/** Skill tree tag */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	FGameplayTag TreeTag;

	/** Total points invested in this tree */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	int32 TotalPointsInvested = 0;

	/** Node progress map */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	TMap<FName, FHarmoniaSkillNodeProgress> NodeProgress;
};

/**
 * Player skill data (all trees)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPlayerSkillData
{
	GENERATED_BODY()

	/** Available skill points */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Skills")
	int32 AvailableSkillPoints = 0;

	/** Total skill points ever earned */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Skills")
	int32 TotalSkillPointsEarned = 0;

	/** Progress per skill tree */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Skills")
	TMap<FGameplayTag, FHarmoniaSkillTreeProgress> TreeProgress;
};

/**
 * Delegate for skill point changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillPointsChanged, int32, NewPoints, int32, Delta);

/**
 * Delegate for node state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillNodeStateChanged, FName, NodeID, EHarmoniaSkillNodeState, OldState, EHarmoniaSkillNodeState, NewState);

/**
 * Delegate for node level changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillNodeLevelChanged, FName, NodeID, int32, OldLevel, int32, NewLevel);

/**
 * Harmonia Skill Tree Subsystem
 * 
 * Manages skill trees, skill points, and node unlocking for RPG progression.
 *
 * Features:
 * - Multiple skill trees per character
 * - Node prerequisites and requirements
 * - Passive effects and active ability grants
 * - Point investment and refund
 * - Save/Load integration
 * - Blueprint-friendly API
 */
UCLASS(Config = Game)
class HARMONIAKIT_API UHarmoniaSkillTreeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ============================================================================
	// Skill Tree Data
	// ============================================================================

	/**
	 * Load skill tree definitions from DataTable
	 * @param DataTable - DataTable containing FHarmoniaSkillTreeDefinition rows
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	void LoadSkillTreesFromDataTable(UDataTable* DataTable);

	/**
	 * Get all available skill trees
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	TArray<FGameplayTag> GetAllSkillTrees() const;

	/**
	 * Get skill tree data by tag
	 * @param TreeTag - Tag of the skill tree
	 * @return Skill tree data, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	bool GetSkillTreeData(FGameplayTag TreeTag, FHarmoniaSkillTreeDefinition& OutData) const;

	/**
	 * Get node data by ID
	 * @param NodeID - Unique node ID
	 * @return Node data, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	bool GetNodeData(FName NodeID, FHarmoniaSkillNodeData& OutData) const;

	// ============================================================================
	// Skill Points
	// ============================================================================

	/**
	 * Get available skill points for a player
	 * @param PlayerController - The player
	 * @return Available skill points
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	int32 GetAvailableSkillPoints(APlayerController* PlayerController) const;

	/**
	 * Add skill points to a player
	 * @param PlayerController - The player
	 * @param Points - Points to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	void AddSkillPoints(APlayerController* PlayerController, int32 Points);

	/**
	 * Award skill points on level up
	 * @param PlayerController - The player
	 * @param NewLevel - The new character level
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	void OnCharacterLevelUp(APlayerController* PlayerController, int32 NewLevel);

	// ============================================================================
	// Node Unlocking
	// ============================================================================

	/**
	 * Check if a node can be unlocked
	 * @param PlayerController - The player
	 * @param NodeID - Node to check
	 * @param OutReason - Reason if cannot unlock
	 * @return True if node can be unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	bool CanUnlockNode(APlayerController* PlayerController, FName NodeID, FText& OutReason) const;

	/**
	 * Unlock or upgrade a node
	 * @param PlayerController - The player
	 * @param NodeID - Node to unlock
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	bool UnlockNode(APlayerController* PlayerController, FName NodeID);

	/**
	 * Get current level of a node
	 * @param PlayerController - The player
	 * @param NodeID - Node to query
	 * @return Current level (0 = not unlocked)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	int32 GetNodeLevel(APlayerController* PlayerController, FName NodeID) const;

	/**
	 * Get state of a node
	 * @param PlayerController - The player
	 * @param NodeID - Node to query
	 * @return Current state
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	EHarmoniaSkillNodeState GetNodeState(APlayerController* PlayerController, FName NodeID) const;

	/**
	 * Check if a node is unlocked (level >= 1)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	bool IsNodeUnlocked(APlayerController* PlayerController, FName NodeID) const;

	// ============================================================================
	// Respec / Reset
	// ============================================================================

	/**
	 * Reset a single skill tree, refunding points
	 * @param PlayerController - The player
	 * @param TreeTag - Tree to reset
	 * @return Points refunded
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	int32 ResetSkillTree(APlayerController* PlayerController, FGameplayTag TreeTag);

	/**
	 * Reset all skill trees, refunding all points
	 * @param PlayerController - The player
	 * @return Total points refunded
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	int32 ResetAllSkillTrees(APlayerController* PlayerController);

	// ============================================================================
	// Progress Queries
	// ============================================================================

	/**
	 * Get total points invested in a skill tree
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	int32 GetPointsInvestedInTree(APlayerController* PlayerController, FGameplayTag TreeTag) const;

	/**
	 * Get all unlocked nodes in a tree
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	TArray<FName> GetUnlockedNodesInTree(APlayerController* PlayerController, FGameplayTag TreeTag) const;

	/**
	 * Get player's complete skill data (for saving)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SkillTree")
	FHarmoniaPlayerSkillData GetPlayerSkillData(APlayerController* PlayerController) const;

	/**
	 * Set player's complete skill data (for loading)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SkillTree")
	void SetPlayerSkillData(APlayerController* PlayerController, const FHarmoniaPlayerSkillData& Data);

	// ============================================================================
	// Events
	// ============================================================================

	/** Fired when skill points change */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SkillTree")
	FOnSkillPointsChanged OnSkillPointsChanged;

	/** Fired when a node's state changes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SkillTree")
	FOnSkillNodeStateChanged OnSkillNodeStateChanged;

	/** Fired when a node's level changes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|SkillTree")
	FOnSkillNodeLevelChanged OnSkillNodeLevelChanged;

protected:
	/** Apply node effects to character (abilities, effects, tags) */
	void ApplyNodeEffects(APlayerController* PlayerController, const FHarmoniaSkillNodeData& NodeData, int32 Level);

	/** Remove node effects from character */
	void RemoveNodeEffects(APlayerController* PlayerController, const FHarmoniaSkillNodeData& NodeData);

	/** Update node states after any change */
	void RefreshNodeStates(APlayerController* PlayerController, FGameplayTag TreeTag);

	/** Get or create player skill data */
	FHarmoniaPlayerSkillData& GetOrCreatePlayerData(APlayerController* PlayerController);
	FHarmoniaPlayerSkillData* FindPlayerData(APlayerController* PlayerController);
	const FHarmoniaPlayerSkillData* FindPlayerData(APlayerController* PlayerController) const;

private:
	/** Skill tree definitions (loaded from DataTables) */
	UPROPERTY()
	TMap<FGameplayTag, FHarmoniaSkillTreeDefinition> SkillTrees;

	/** Node ID to tree tag mapping for fast lookup */
	UPROPERTY()
	TMap<FName, FGameplayTag> NodeToTreeMap;

	/** Player skill data (keyed by player unique ID) */
	UPROPERTY()
	TMap<FString, FHarmoniaPlayerSkillData> PlayerSkillData;

	/** Skill points awarded per level (configurable) */
	UPROPERTY(Config)
	int32 SkillPointsPerLevel = 1;

	/** Bonus points at specific levels (e.g., every 10 levels) */
	UPROPERTY(Config)
	TMap<int32, int32> BonusPointsAtLevel;
};
