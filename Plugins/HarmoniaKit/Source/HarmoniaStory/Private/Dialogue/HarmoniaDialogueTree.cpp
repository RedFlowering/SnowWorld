// Copyright 2025 Snow Game Studio.

#include "Dialogue/HarmoniaDialogueTree.h"
#include "Dialogue/HarmoniaDialogueNode.h"

UHarmoniaDialogueTree::UHarmoniaDialogueTree()
	: RootNode(nullptr)
{
}

#if WITH_EDITOR
UHarmoniaDialogueNode* UHarmoniaDialogueTree::CreateNode(TSubclassOf<UHarmoniaDialogueNode> NodeClass)
{
	if (!NodeClass) return nullptr;

	UHarmoniaDialogueNode* NewNode = NewObject<UHarmoniaDialogueNode>(this, NodeClass);
	AllNodes.Add(NewNode);
	return NewNode;
}
#endif
