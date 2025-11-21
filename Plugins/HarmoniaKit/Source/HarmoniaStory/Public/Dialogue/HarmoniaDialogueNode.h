// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaDialogueNode.generated.h"

class UHarmoniaDialogueTree;
class USoundBase;
class UAnimMontage;

/**
 * Base class for all dialogue nodes
 */
UCLASS(BlueprintType, EditInlineNew)
class HARMONIASTORY_API UHarmoniaDialogueNode : public UObject
{
	GENERATED_BODY()

public:
	UHarmoniaDialogueNode();

	/** Speaker name (can be overridden by NPC name) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText SpeakerName;

	/** Main dialogue text */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue", meta = (MultiLine = true))
	FText DialogueText;

	/** Voice line audio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	USoundBase* VoiceLine;

	/** Animation to play on speaker */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UAnimMontage* Animation;

	/** Next node to play (if not a choice node) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UHarmoniaDialogueNode* NextNode;

	/** Events to trigger when this node starts */
	// TODO: Add event system integration

#if WITH_EDITOR
	virtual FText GetNodeTitle() const;
#endif
};

/**
 * Condition for dialogue choices
 */
UCLASS(BlueprintType, EditInlineNew)
class HARMONIASTORY_API UHarmoniaDialogueCondition : public UObject
{
	GENERATED_BODY()

public:
	/** Check if condition is met */
	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	bool IsMet(const UObject* ContextObject) const;
	virtual bool IsMet_Implementation(const UObject* ContextObject) const { return true; }
};

UENUM(BlueprintType)
enum class EDialogueConsequenceType : uint8
{
	None,
	StartQuest,
	UpdateObjective,
	ModifyAffinity,
	UnlockJournalEntry
};

USTRUCT(BlueprintType)
struct FDialogueConsequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	EDialogueConsequenceType Type = EDialogueConsequenceType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FHarmoniaID TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString StringValue;
};

/**
 * Choice entry for choice nodes
 */
USTRUCT(BlueprintType)
struct FHarmoniaDialogueChoice
{
	GENERATED_BODY()

	/** Text to display on button */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText ChoiceText;

	/** Target node if chosen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	UHarmoniaDialogueNode* TargetNode;

	/** Conditions to show this choice */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<TSubclassOf<UHarmoniaDialogueCondition>> Conditions;

	/** Consequences of choosing this option */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueConsequence> Consequences;

	FHarmoniaDialogueChoice()
		: ChoiceText(FText::FromString("Next"))
		, TargetNode(nullptr)
	{}
};

/**
 * Node representing player choices
 */
UCLASS(BlueprintType)
class HARMONIASTORY_API UHarmoniaDialogueNode_Choice : public UHarmoniaDialogueNode
{
	GENERATED_BODY()

public:
	UHarmoniaDialogueNode_Choice();

	/** List of choices */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FHarmoniaDialogueChoice> Choices;
};
