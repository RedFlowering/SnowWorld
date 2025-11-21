// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/HarmoniaDialogueNode.h"
#include "HarmoniaDialogueComponent.generated.h"

class UHarmoniaDialogueTree;
class UHarmoniaDialogueNode_Choice;

/**
 * Delegate for dialogue events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, UHarmoniaDialogueTree*, DialogueTree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, UHarmoniaDialogueTree*, DialogueTree);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueNodeVisited, UHarmoniaDialogueNode*, Node);

/**
 * Component to handle dialogue playback
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIASTORY_API UHarmoniaDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaDialogueComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** Start a dialogue tree */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(UHarmoniaDialogueTree* Tree);

	/** Advance to next node (if not a choice node) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue();

	/** Select a choice (if at a choice node) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	/** End current dialogue */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	/** Get current active node */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UHarmoniaDialogueNode* GetCurrentNode() const { return CurrentNode; }

	/** Check if dialogue is active */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return CurrentTree != nullptr; }

public:
	/** Called when dialogue starts */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueStarted OnDialogueStarted;

	/** Called when dialogue ends */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueEnded OnDialogueEnded;

	/** Called when a node is visited */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueNodeVisited OnDialogueNodeVisited;

protected:
	/** Set current node and broadcast event */
	void SetCurrentNode(UHarmoniaDialogueNode* NewNode);

	/** Execute consequences of a choice */
	void ExecuteConsequences(const TArray<FDialogueConsequence>& Consequences);

	/** Current active dialogue tree */
	UPROPERTY(Transient)
	UHarmoniaDialogueTree* CurrentTree;

	/** Current active node */
	UPROPERTY(Transient)
	UHarmoniaDialogueNode* CurrentNode;
};
