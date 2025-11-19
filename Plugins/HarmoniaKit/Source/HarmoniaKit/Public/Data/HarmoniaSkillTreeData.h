// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Definitions/HarmoniaProgressionDefinitions.h"
#include "HarmoniaSkillTreeData.generated.h"

/**
 * Data asset containing skill tree definitions
 * Can be used for weapon-specific, style-specific, or general skill trees
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaSkillTreeData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Skill tree name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree")
	FText SkillTreeName;

	/** Skill tree description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree")
	FText SkillTreeDescription;

	/** Skill tree category */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree")
	EHarmoniaSkillTreeCategory TreeCategory;

	/** Icon for this skill tree */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree")
	TObjectPtr<UTexture2D> TreeIcon;

	/** All skill nodes in this tree */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> SkillNodes;

	/** Required class to access this tree (None = available to all) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	EHarmoniaCharacterClass RequiredClass;

	/** Minimum level to access this tree */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 MinimumLevel = 1;

	//~ Queries

	/** Find a skill node by ID */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree")
	FHarmoniaSkillNode* FindSkillNode(FName NodeID);

	/** Get all skill nodes of a specific type */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetSkillNodesByType(EHarmoniaSkillNodeType NodeType) const;

	/** Get all root nodes (nodes with no prerequisites) */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetRootNodes() const;

	/** Get child nodes of a given node */
	UFUNCTION(BlueprintCallable, Category = "Skill Tree")
	TArray<FHarmoniaSkillNode> GetChildNodes(FName ParentNodeID) const;

	/** Check if this tree is accessible for a given class and level */
	UFUNCTION(BlueprintPure, Category = "Skill Tree")
	bool IsAccessible(EHarmoniaCharacterClass PlayerClass, int32 PlayerLevel) const;
};
