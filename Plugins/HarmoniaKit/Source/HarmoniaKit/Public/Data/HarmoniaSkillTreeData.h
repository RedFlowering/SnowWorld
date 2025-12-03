// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaSkillTreeData.h
 * @brief Skill tree data asset for skill node definitions and unlock conditions
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaSkillTreeData.generated.h"

/**
 * @class UHarmoniaSkillTreeData
 * @brief Skill tree data asset
 * 
 * Manages skill node definitions, unlock conditions, and rewards.
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaSkillTreeData : public UDataAsset
{
	GENERATED_BODY()

public:
	//~ Skill Tree Basic Info
	
	/** Skill tree ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FName TreeID;

	/** Skill tree name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeName;

	/** Skill tree description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	FText TreeDescription;

	/** Skill tree category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
	EHarmoniaSkillTreeCategory Category;

	/** Skill tree icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Visual")
	TSoftObjectPtr<UTexture2D> TreeIcon;

	//~ Access Requirements
	
	/** Required class (None for all classes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	EHarmoniaCharacterClass RequiredClass = EHarmoniaCharacterClass::None;

	/** Minimum level requirement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Requirements")
	int32 MinimumLevel = 1;

	//~ Skill Nodes
	
	/** All skill nodes in this skill tree */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree|Nodes")
	TArray<FHarmoniaSkillNode> SkillNodes;

	//~ Utility Functions
	
	/** Find skill node by ID (Blueprint) */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree", DisplayName = "Find Skill Node")
	bool FindSkillNodeBP(FName NodeID, FHarmoniaSkillNode& OutNode);

	/** Find skill node by ID (C++) */
	FHarmoniaSkillNode* FindSkillNode(FName NodeID);

	/** Get skill nodes by type */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetSkillNodesByType(EHarmoniaSkillNodeType NodeType) const;

	/** Get root nodes (nodes without prerequisites) */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetRootNodes() const;

	/** Get child nodes of a specific node */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetChildNodes(FName ParentNodeID) const;

	/** Check if player can access this skill tree */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsAccessible(EHarmoniaCharacterClass PlayerClass, int32 PlayerLevel) const;
};
