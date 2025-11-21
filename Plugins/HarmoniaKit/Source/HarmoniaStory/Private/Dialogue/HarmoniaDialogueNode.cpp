// Copyright 2025 Snow Game Studio.

#include "Dialogue/HarmoniaDialogueNode.h"

UHarmoniaDialogueNode::UHarmoniaDialogueNode()
	: SpeakerName(FText::FromString("Speaker"))
	, DialogueText(FText::FromString("..."))
	, VoiceLine(nullptr)
	, Animation(nullptr)
	, NextNode(nullptr)
{
}

#if WITH_EDITOR
FText UHarmoniaDialogueNode::GetNodeTitle() const
{
	return DialogueText.IsEmpty() ? FText::FromString("Empty Node") : DialogueText;
}
#endif

UHarmoniaDialogueNode_Choice::UHarmoniaDialogueNode_Choice()
{
}
