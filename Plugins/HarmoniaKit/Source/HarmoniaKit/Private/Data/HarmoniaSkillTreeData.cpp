// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaSkillTreeData.h"

bool UHarmoniaSkillTreeData::FindSkillNodeBP(FName NodeID, FHarmoniaSkillNode& OutNode)
{
	FHarmoniaSkillNode* Node = FindSkillNode(NodeID);
	if (Node)
	{
		OutNode = *Node;
		return true;
	}
	return false;
}

FHarmoniaSkillNode* UHarmoniaSkillTreeData::FindSkillNode(FName NodeID)
{
	for (FHarmoniaSkillNode& Node : SkillNodes)
	{
		if (Node.NodeID == NodeID)
		{
			return &Node;
		}
	}

	return nullptr;
}

TArray<FHarmoniaSkillNode> UHarmoniaSkillTreeData::GetSkillNodesByType(EHarmoniaSkillNodeType NodeType) const
{
	TArray<FHarmoniaSkillNode> Result;

	for (const FHarmoniaSkillNode& Node : SkillNodes)
	{
		if (Node.NodeType == NodeType)
		{
			Result.Add(Node);
		}
	}

	return Result;
}

TArray<FHarmoniaSkillNode> UHarmoniaSkillTreeData::GetRootNodes() const
{
	TArray<FHarmoniaSkillNode> Result;

	for (const FHarmoniaSkillNode& Node : SkillNodes)
	{
		if (Node.PrerequisiteNodeIDs.Num() == 0)
		{
			Result.Add(Node);
		}
	}

	return Result;
}

TArray<FHarmoniaSkillNode> UHarmoniaSkillTreeData::GetChildNodes(FName ParentNodeID) const
{
	TArray<FHarmoniaSkillNode> Result;

	for (const FHarmoniaSkillNode& Node : SkillNodes)
	{
		if (Node.PrerequisiteNodeIDs.Contains(ParentNodeID))
		{
			Result.Add(Node);
		}
	}

	return Result;
}

bool UHarmoniaSkillTreeData::IsAccessible(EHarmoniaCharacterClass PlayerClass, int32 PlayerLevel) const
{
	// Check level requirement
	if (PlayerLevel < MinimumLevel)
	{
		return false;
	}

	// Check class requirement
	if (RequiredClass != EHarmoniaCharacterClass::None && RequiredClass != PlayerClass)
	{
		return false;
	}

	return true;
}
