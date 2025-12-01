// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaDialogueComponent.h"
#include "Core/HarmoniaCoreBFL.h"
#include "Components/HarmoniaQuestComponent.h"
#include "Relationship/HarmoniaRelationshipComponent.h"
#include "Journal/HarmoniaJournalSubsystem.h"
#include "Dialogue/HarmoniaDialogueTree.h"
#include "Dialogue/HarmoniaDialogueNode.h"
#include "HarmoniaStoryLog.h"
#include "Kismet/GameplayStatics.h"

UHarmoniaDialogueComponent::UHarmoniaDialogueComponent()
	: CurrentTree(nullptr)
	, CurrentNode(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHarmoniaDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaDialogueComponent::StartDialogue(UHarmoniaDialogueTree* Tree)
{
	if (!Tree)
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("StartDialogue called with null tree"));
		return;
	}

	if (IsDialogueActive())
	{
		EndDialogue();
	}

	CurrentTree = Tree;
	OnDialogueStarted.Broadcast(CurrentTree);

	if (CurrentTree->RootNode)
	{
		SetCurrentNode(CurrentTree->RootNode);
	}
	else
	{
		EndDialogue();
	}
}

void UHarmoniaDialogueComponent::AdvanceDialogue()
{
	if (!IsDialogueActive() || !CurrentNode)
	{
		return;
	}

	// If it's a choice node, we can't just advance
	if (CurrentNode->IsA(UHarmoniaDialogueNode_Choice::StaticClass()))
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("Cannot auto-advance from a choice node"));
		return;
	}

	if (CurrentNode->NextNode)
	{
		SetCurrentNode(CurrentNode->NextNode);
	}
	else
	{
		EndDialogue();
	}
}

void UHarmoniaDialogueComponent::SelectChoice(int32 ChoiceIndex)
{
	if (!IsDialogueActive() || !CurrentNode)
	{
		return;
	}

	UHarmoniaDialogueNode_Choice* ChoiceNode = Cast<UHarmoniaDialogueNode_Choice>(CurrentNode);
	if (!ChoiceNode)
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("SelectChoice called but current node is not a choice node"));
		return;
	}

	if (ChoiceNode->Choices.IsValidIndex(ChoiceIndex))
	{
		const FHarmoniaDialogueChoice& Choice = ChoiceNode->Choices[ChoiceIndex];
		
		// Check conditions again just in case
		bool bConditionsMet = true;
		for (TSubclassOf<UHarmoniaDialogueCondition> ConditionClass : Choice.Conditions)
		{
			if (ConditionClass)
			{
				const UHarmoniaDialogueCondition* Condition = ConditionClass.GetDefaultObject();
				if (Condition && !Condition->IsMet(GetOwner()))
				{
					bConditionsMet = false;
					break;
				}
			}
		}

		if (bConditionsMet)
		{
			// Execute consequences
			ExecuteConsequences(Choice.Consequences);

			if (Choice.TargetNode)
			{
				SetCurrentNode(Choice.TargetNode);
			}
			else
			{
				EndDialogue();
			}
		}
		else
		{
			UE_LOG(LogHarmoniaStory, Warning, TEXT("Selected choice conditions not met"));
		}
	}
	else
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("Invalid choice index: %d"), ChoiceIndex);
	}
}

void UHarmoniaDialogueComponent::ExecuteConsequences(const TArray<FDialogueConsequence>& Consequences)
{
	for (const FDialogueConsequence& Consequence : Consequences)
	{
		switch (Consequence.Type)
		{
			case EDialogueConsequenceType::StartQuest:
			{
				if (Consequence.TargetId.IsValid())
				{
					UHarmoniaQuestComponent* QuestComp = GetOwner()->FindComponentByClass<UHarmoniaQuestComponent>();
					if (QuestComp)
					{
						QuestComp->StartQuest(Consequence.TargetId);
					}
				}
				break;
			}
			case EDialogueConsequenceType::UpdateObjective:
			{
				// This requires knowing which quest and objective index.
				// Assuming TargetId is QuestId and Amount is ObjectiveIndex?
				// Or maybe we need a more complex structure.
				// For now, skipping as it requires more data.
				break;
			}
			case EDialogueConsequenceType::ModifyAffinity:
			{
				if (Consequence.TargetId.IsValid())
				{
					UHarmoniaRelationshipComponent* RelComp = GetOwner()->FindComponentByClass<UHarmoniaRelationshipComponent>();
					if (RelComp)
					{
						RelComp->ModifyAffinity(Consequence.TargetId, Consequence.Amount);
					}
				}
				break;
			}
			case EDialogueConsequenceType::UnlockJournalEntry:
			{
				if (Consequence.TargetId.IsValid())
				{
					UHarmoniaJournalSubsystem* Journal = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaJournalSubsystem>(this);
					if (Journal)
					{
						Journal->UnlockEntry(Consequence.TargetId);
					}
				}
				break;
			}
			default:
				break;
		}
	}
}

void UHarmoniaDialogueComponent::EndDialogue()
{
	if (IsDialogueActive())
	{
		UHarmoniaDialogueTree* EndedTree = CurrentTree;
		CurrentTree = nullptr;
		CurrentNode = nullptr;
		OnDialogueEnded.Broadcast(EndedTree);
	}
}

void UHarmoniaDialogueComponent::SetCurrentNode(UHarmoniaDialogueNode* NewNode)
{
	CurrentNode = NewNode;
	if (CurrentNode)
	{
		OnDialogueNodeVisited.Broadcast(CurrentNode);
		
		// Play audio/anim if present
		// Note: In a real implementation, we might want to delegate this to a UI controller or similar
		// For now, we just expose the data via the node
	}
}
