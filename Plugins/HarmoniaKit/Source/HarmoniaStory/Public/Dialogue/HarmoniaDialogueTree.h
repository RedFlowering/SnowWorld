// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HarmoniaDialogueTree.generated.h"

class UHarmoniaDialogueNode;

/**
 * Data asset containing a dialogue graph
 */
UCLASS(BlueprintType)
class HARMONIASTORY_API UHarmoniaDialogueTree : public UDataAsset
{
	GENERATED_BODY()

public:
	UHarmoniaDialogueTree();

	/** Root node of the dialogue */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (Instanced))
	UHarmoniaDialogueNode* RootNode;

	/** All nodes in the tree (for editor visualization/management) */
	UPROPERTY()
	TArray<UHarmoniaDialogueNode*> AllNodes;

#if WITH_EDITOR
	/** Create a new node in the tree */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Editor")
	UHarmoniaDialogueNode* CreateNode(TSubclassOf<UHarmoniaDialogueNode> NodeClass);
#endif
};
