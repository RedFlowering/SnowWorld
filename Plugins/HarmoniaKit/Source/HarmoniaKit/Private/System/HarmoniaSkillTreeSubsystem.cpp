// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaSkillTreeSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "HarmoniaLogCategories.h"

// ============================================================================
// Subsystem Lifecycle
// ============================================================================

void UHarmoniaSkillTreeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaSkillTreeSubsystem: Initialized"));
}

void UHarmoniaSkillTreeSubsystem::Deinitialize()
{
	SkillTrees.Empty();
	NodeToTreeMap.Empty();
	PlayerSkillData.Empty();
	Super::Deinitialize();
}

// ============================================================================
// Skill Tree Data
// ============================================================================

void UHarmoniaSkillTreeSubsystem::LoadSkillTreesFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("SkillTree: Cannot load from null DataTable"));
		return;
	}

	TArray<FHarmoniaSkillTreeDefinition*> Rows;
	DataTable->GetAllRows<FHarmoniaSkillTreeDefinition>(TEXT("LoadSkillTrees"), Rows);

	for (FHarmoniaSkillTreeDefinition* Row : Rows)
	{
		if (Row && Row->TreeTag.IsValid())
		{
			SkillTrees.Add(Row->TreeTag, *Row);

			// Build node to tree mapping
			for (const FHarmoniaSkillNodeData& Node : Row->Nodes)
			{
				NodeToTreeMap.Add(Node.NodeID, Row->TreeTag);
			}

			UE_LOG(LogHarmoniaKit, Log, TEXT("SkillTree: Loaded tree '%s' with %d nodes"),
				*Row->TreeTag.ToString(), Row->Nodes.Num());
		}
	}
}

TArray<FGameplayTag> UHarmoniaSkillTreeSubsystem::GetAllSkillTrees() const
{
	TArray<FGameplayTag> Result;
	SkillTrees.GetKeys(Result);
	return Result;
}

bool UHarmoniaSkillTreeSubsystem::GetSkillTreeData(FGameplayTag TreeTag, FHarmoniaSkillTreeDefinition& OutData) const
{
	if (const FHarmoniaSkillTreeDefinition* Data = SkillTrees.Find(TreeTag))
	{
		OutData = *Data;
		return true;
	}
	return false;
}

bool UHarmoniaSkillTreeSubsystem::GetNodeData(FName NodeID, FHarmoniaSkillNodeData& OutData) const
{
	if (const FGameplayTag* TreeTag = NodeToTreeMap.Find(NodeID))
	{
		if (const FHarmoniaSkillTreeDefinition* TreeData = SkillTrees.Find(*TreeTag))
		{
			for (const FHarmoniaSkillNodeData& Node : TreeData->Nodes)
			{
				if (Node.NodeID == NodeID)
				{
					OutData = Node;
					return true;
				}
			}
		}
	}
	return false;
}

// ============================================================================
// Skill Points
// ============================================================================

int32 UHarmoniaSkillTreeSubsystem::GetAvailableSkillPoints(APlayerController* PlayerController) const
{
	if (const FHarmoniaPlayerSkillData* Data = FindPlayerData(PlayerController))
	{
		return Data->AvailableSkillPoints;
	}
	return 0;
}

void UHarmoniaSkillTreeSubsystem::AddSkillPoints(APlayerController* PlayerController, int32 Points)
{
	if (Points <= 0) return;

	FHarmoniaPlayerSkillData& Data = GetOrCreatePlayerData(PlayerController);
	int32 OldPoints = Data.AvailableSkillPoints;
	Data.AvailableSkillPoints += Points;
	Data.TotalSkillPointsEarned += Points;

	OnSkillPointsChanged.Broadcast(Data.AvailableSkillPoints, Points);

	UE_LOG(LogHarmoniaKit, Log, TEXT("SkillTree: Added %d skill points (now: %d)"), Points, Data.AvailableSkillPoints);
}

void UHarmoniaSkillTreeSubsystem::OnCharacterLevelUp(APlayerController* PlayerController, int32 NewLevel)
{
	int32 PointsToAward = SkillPointsPerLevel;

	// Check for bonus points at this level
	if (const int32* BonusPoints = BonusPointsAtLevel.Find(NewLevel))
	{
		PointsToAward += *BonusPoints;
	}

	AddSkillPoints(PlayerController, PointsToAward);
}

// ============================================================================
// Node Unlocking
// ============================================================================

bool UHarmoniaSkillTreeSubsystem::CanUnlockNode(APlayerController* PlayerController, FName NodeID, FText& OutReason) const
{
	FHarmoniaSkillNodeData NodeData;
	if (!GetNodeData(NodeID, NodeData))
	{
		OutReason = FText::FromString(TEXT("Node not found"));
		return false;
	}

	const FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData)
	{
		OutReason = FText::FromString(TEXT("No player data"));
		return false;
	}

	// Check current level
	int32 CurrentLevel = GetNodeLevel(PlayerController, NodeID);
	if (CurrentLevel >= NodeData.MaxLevel)
	{
		OutReason = FText::FromString(TEXT("Node already at max level"));
		return false;
	}

	// Check skill points
	int32 PointCost = NodeData.GetPointCostForLevel(CurrentLevel + 1);
	if (PlayerData->AvailableSkillPoints < PointCost)
	{
		OutReason = FText::Format(FText::FromString(TEXT("Not enough skill points (need {0})")), PointCost);
		return false;
	}

	// Check character level (would need character level from somewhere)
	// TODO: Integrate with progression component

	// Check tree points invested
	int32 TreePoints = GetPointsInvestedInTree(PlayerController, NodeData.SkillTreeTag);
	if (TreePoints < NodeData.RequiredTreePointsInvested)
	{
		OutReason = FText::Format(FText::FromString(TEXT("Need {0} points invested in tree")), NodeData.RequiredTreePointsInvested);
		return false;
	}

	// Check prerequisites
	for (const FName& PrereqID : NodeData.PrerequisiteNodes)
	{
		if (!IsNodeUnlocked(PlayerController, PrereqID))
		{
			OutReason = FText::Format(FText::FromString(TEXT("Prerequisite not met: {0}")), FText::FromName(PrereqID));
			return false;
		}
	}

	return true;
}

bool UHarmoniaSkillTreeSubsystem::UnlockNode(APlayerController* PlayerController, FName NodeID)
{
	FText Reason;
	if (!CanUnlockNode(PlayerController, NodeID, Reason))
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("SkillTree: Cannot unlock node %s: %s"), *NodeID.ToString(), *Reason.ToString());
		return false;
	}

	FHarmoniaSkillNodeData NodeData;
	if (!GetNodeData(NodeID, NodeData))
	{
		return false;
	}

	FHarmoniaPlayerSkillData& PlayerData = GetOrCreatePlayerData(PlayerController);
	FHarmoniaSkillTreeProgress& TreeProgress = PlayerData.TreeProgress.FindOrAdd(NodeData.SkillTreeTag);
	TreeProgress.TreeTag = NodeData.SkillTreeTag;

	FHarmoniaSkillNodeProgress& NodeProgress = TreeProgress.NodeProgress.FindOrAdd(NodeID);
	int32 OldLevel = NodeProgress.CurrentLevel;
	EHarmoniaSkillNodeState OldState = NodeProgress.State;

	// Spend points
	int32 PointCost = NodeData.GetPointCostForLevel(OldLevel + 1);
	PlayerData.AvailableSkillPoints -= PointCost;
	TreeProgress.TotalPointsInvested += PointCost;

	// Upgrade node
	NodeProgress.NodeID = NodeID;
	NodeProgress.CurrentLevel++;

	// Update state
	if (NodeProgress.CurrentLevel >= NodeData.MaxLevel)
	{
		NodeProgress.State = EHarmoniaSkillNodeState::Maxed;
	}
	else
	{
		NodeProgress.State = EHarmoniaSkillNodeState::Unlocked;
	}

	// Apply effects
	ApplyNodeEffects(PlayerController, NodeData, NodeProgress.CurrentLevel);

	// Refresh states
	RefreshNodeStates(PlayerController, NodeData.SkillTreeTag);

	// Fire events
	OnSkillPointsChanged.Broadcast(PlayerData.AvailableSkillPoints, -PointCost);
	OnSkillNodeLevelChanged.Broadcast(NodeID, OldLevel, NodeProgress.CurrentLevel);
	if (OldState != NodeProgress.State)
	{
		OnSkillNodeStateChanged.Broadcast(NodeID, OldState, NodeProgress.State);
	}

	UE_LOG(LogHarmoniaKit, Log, TEXT("SkillTree: Unlocked node %s to level %d"), *NodeID.ToString(), NodeProgress.CurrentLevel);
	return true;
}

int32 UHarmoniaSkillTreeSubsystem::GetNodeLevel(APlayerController* PlayerController, FName NodeID) const
{
	const FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData) return 0;

	FHarmoniaSkillNodeData NodeData;
	if (!GetNodeData(NodeID, NodeData)) return 0;

	if (const FHarmoniaSkillTreeProgress* TreeProgress = PlayerData->TreeProgress.Find(NodeData.SkillTreeTag))
	{
		if (const FHarmoniaSkillNodeProgress* NodeProgress = TreeProgress->NodeProgress.Find(NodeID))
		{
			return NodeProgress->CurrentLevel;
		}
	}
	return 0;
}

EHarmoniaSkillNodeState UHarmoniaSkillTreeSubsystem::GetNodeState(APlayerController* PlayerController, FName NodeID) const
{
	const FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData) return EHarmoniaSkillNodeState::Locked;

	FHarmoniaSkillNodeData NodeData;
	if (!GetNodeData(NodeID, NodeData)) return EHarmoniaSkillNodeState::Locked;

	if (const FHarmoniaSkillTreeProgress* TreeProgress = PlayerData->TreeProgress.Find(NodeData.SkillTreeTag))
	{
		if (const FHarmoniaSkillNodeProgress* NodeProgress = TreeProgress->NodeProgress.Find(NodeID))
		{
			return NodeProgress->State;
		}
	}

	// Not in progress data, check if it can be unlocked
	FText Reason;
	if (CanUnlockNode(PlayerController, NodeID, Reason))
	{
		return EHarmoniaSkillNodeState::Available;
	}
	return EHarmoniaSkillNodeState::Locked;
}

bool UHarmoniaSkillTreeSubsystem::IsNodeUnlocked(APlayerController* PlayerController, FName NodeID) const
{
	return GetNodeLevel(PlayerController, NodeID) >= 1;
}

// ============================================================================
// Respec / Reset
// ============================================================================

int32 UHarmoniaSkillTreeSubsystem::ResetSkillTree(APlayerController* PlayerController, FGameplayTag TreeTag)
{
	FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData) return 0;

	FHarmoniaSkillTreeProgress* TreeProgress = PlayerData->TreeProgress.Find(TreeTag);
	if (!TreeProgress) return 0;

	int32 RefundedPoints = TreeProgress->TotalPointsInvested;

	// Remove all node effects
	FHarmoniaSkillTreeDefinition TreeData;
	if (GetSkillTreeData(TreeTag, TreeData))
	{
		for (const FHarmoniaSkillNodeData& Node : TreeData.Nodes)
		{
			if (const FHarmoniaSkillNodeProgress* NodeProgress = TreeProgress->NodeProgress.Find(Node.NodeID))
			{
				if (NodeProgress->CurrentLevel > 0)
				{
					RemoveNodeEffects(PlayerController, Node);
				}
			}
		}
	}

	// Reset progress
	TreeProgress->NodeProgress.Empty();
	TreeProgress->TotalPointsInvested = 0;

	// Refund points
	PlayerData->AvailableSkillPoints += RefundedPoints;

	OnSkillPointsChanged.Broadcast(PlayerData->AvailableSkillPoints, RefundedPoints);

	UE_LOG(LogHarmoniaKit, Log, TEXT("SkillTree: Reset tree %s, refunded %d points"), *TreeTag.ToString(), RefundedPoints);
	return RefundedPoints;
}

int32 UHarmoniaSkillTreeSubsystem::ResetAllSkillTrees(APlayerController* PlayerController)
{
	int32 TotalRefunded = 0;
	TArray<FGameplayTag> Trees = GetAllSkillTrees();
	for (const FGameplayTag& TreeTag : Trees)
	{
		TotalRefunded += ResetSkillTree(PlayerController, TreeTag);
	}
	return TotalRefunded;
}

// ============================================================================
// Progress Queries
// ============================================================================

int32 UHarmoniaSkillTreeSubsystem::GetPointsInvestedInTree(APlayerController* PlayerController, FGameplayTag TreeTag) const
{
	const FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData) return 0;

	if (const FHarmoniaSkillTreeProgress* TreeProgress = PlayerData->TreeProgress.Find(TreeTag))
	{
		return TreeProgress->TotalPointsInvested;
	}
	return 0;
}

TArray<FName> UHarmoniaSkillTreeSubsystem::GetUnlockedNodesInTree(APlayerController* PlayerController, FGameplayTag TreeTag) const
{
	TArray<FName> Result;

	const FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData) return Result;

	if (const FHarmoniaSkillTreeProgress* TreeProgress = PlayerData->TreeProgress.Find(TreeTag))
	{
		for (const auto& Pair : TreeProgress->NodeProgress)
		{
			if (Pair.Value.CurrentLevel > 0)
			{
				Result.Add(Pair.Key);
			}
		}
	}
	return Result;
}

FHarmoniaPlayerSkillData UHarmoniaSkillTreeSubsystem::GetPlayerSkillData(APlayerController* PlayerController) const
{
	if (const FHarmoniaPlayerSkillData* Data = FindPlayerData(PlayerController))
	{
		return *Data;
	}
	return FHarmoniaPlayerSkillData();
}

void UHarmoniaSkillTreeSubsystem::SetPlayerSkillData(APlayerController* PlayerController, const FHarmoniaPlayerSkillData& Data)
{
	FHarmoniaPlayerSkillData& PlayerData = GetOrCreatePlayerData(PlayerController);
	PlayerData = Data;

	// Re-apply all node effects
	for (const auto& TreePair : Data.TreeProgress)
	{
		FHarmoniaSkillTreeDefinition TreeData;
		if (GetSkillTreeData(TreePair.Key, TreeData))
		{
			for (const FHarmoniaSkillNodeData& Node : TreeData.Nodes)
			{
				if (const FHarmoniaSkillNodeProgress* NodeProgress = TreePair.Value.NodeProgress.Find(Node.NodeID))
				{
					if (NodeProgress->CurrentLevel > 0)
					{
						ApplyNodeEffects(PlayerController, Node, NodeProgress->CurrentLevel);
					}
				}
			}
		}
	}

	UE_LOG(LogHarmoniaKit, Log, TEXT("SkillTree: Loaded player skill data (%d points, %d trees)"),
		Data.AvailableSkillPoints, Data.TreeProgress.Num());
}

// ============================================================================
// Internal Functions
// ============================================================================

void UHarmoniaSkillTreeSubsystem::ApplyNodeEffects(APlayerController* PlayerController, const FHarmoniaSkillNodeData& NodeData, int32 Level)
{
	if (!PlayerController) return;

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!ASC) return;

	switch (NodeData.NodeType)
	{
	case EHarmoniaSkillNodeType::Active:
		if (NodeData.GrantedAbility && Level == 1) // Only grant on first unlock
		{
			FGameplayAbilitySpec AbilitySpec(NodeData.GrantedAbility, Level);
			ASC->GiveAbility(AbilitySpec);
		}
		break;

	case EHarmoniaSkillNodeType::Passive:
		if (NodeData.PassiveEffect)
		{
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(NodeData.PassiveEffect, Level, ContextHandle);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
		break;

	default:
		break;
	}

	// Apply granted tags
	if (NodeData.GrantedTags.Num() > 0)
	{
		ASC->AddLooseGameplayTags(NodeData.GrantedTags);
	}
}

void UHarmoniaSkillTreeSubsystem::RemoveNodeEffects(APlayerController* PlayerController, const FHarmoniaSkillNodeData& NodeData)
{
	if (!PlayerController) return;

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn) return;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!ASC) return;

	switch (NodeData.NodeType)
	{
	case EHarmoniaSkillNodeType::Active:
		if (NodeData.GrantedAbility)
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(NodeData.GrantedAbility);
			if (Spec)
			{
				ASC->ClearAbility(Spec->Handle);
			}
		}
		break;

	case EHarmoniaSkillNodeType::Passive:
		if (NodeData.PassiveEffect)
		{
			ASC->RemoveActiveGameplayEffectBySourceEffect(NodeData.PassiveEffect, ASC);
		}
		break;

	default:
		break;
	}

	// Remove granted tags
	if (NodeData.GrantedTags.Num() > 0)
	{
		ASC->RemoveLooseGameplayTags(NodeData.GrantedTags);
	}
}

void UHarmoniaSkillTreeSubsystem::RefreshNodeStates(APlayerController* PlayerController, FGameplayTag TreeTag)
{
	FHarmoniaSkillTreeDefinition TreeData;
	if (!GetSkillTreeData(TreeTag, TreeData)) return;

	FHarmoniaPlayerSkillData* PlayerData = FindPlayerData(PlayerController);
	if (!PlayerData) return;

	FHarmoniaSkillTreeProgress& TreeProgress = PlayerData->TreeProgress.FindOrAdd(TreeTag);

	for (const FHarmoniaSkillNodeData& Node : TreeData.Nodes)
	{
		FHarmoniaSkillNodeProgress& NodeProgress = TreeProgress.NodeProgress.FindOrAdd(Node.NodeID);
		NodeProgress.NodeID = Node.NodeID;

		EHarmoniaSkillNodeState OldState = NodeProgress.State;

		if (NodeProgress.CurrentLevel >= Node.MaxLevel)
		{
			NodeProgress.State = EHarmoniaSkillNodeState::Maxed;
		}
		else if (NodeProgress.CurrentLevel > 0)
		{
			NodeProgress.State = EHarmoniaSkillNodeState::Unlocked;
		}
		else
		{
			FText Reason;
			if (CanUnlockNode(PlayerController, Node.NodeID, Reason))
			{
				NodeProgress.State = EHarmoniaSkillNodeState::Available;
			}
			else
			{
				NodeProgress.State = EHarmoniaSkillNodeState::Locked;
			}
		}

		if (OldState != NodeProgress.State)
		{
			OnSkillNodeStateChanged.Broadcast(Node.NodeID, OldState, NodeProgress.State);
		}
	}
}

FHarmoniaPlayerSkillData& UHarmoniaSkillTreeSubsystem::GetOrCreatePlayerData(APlayerController* PlayerController)
{
	FString PlayerID = TEXT("LocalPlayer");
	if (PlayerController && PlayerController->PlayerState)
	{
		PlayerID = PlayerController->PlayerState->GetUniqueId().ToString();
	}

	return PlayerSkillData.FindOrAdd(PlayerID);
}

FHarmoniaPlayerSkillData* UHarmoniaSkillTreeSubsystem::FindPlayerData(APlayerController* PlayerController)
{
	FString PlayerID = TEXT("LocalPlayer");
	if (PlayerController && PlayerController->PlayerState)
	{
		PlayerID = PlayerController->PlayerState->GetUniqueId().ToString();
	}

	return PlayerSkillData.Find(PlayerID);
}

const FHarmoniaPlayerSkillData* UHarmoniaSkillTreeSubsystem::FindPlayerData(APlayerController* PlayerController) const
{
	FString PlayerID = TEXT("LocalPlayer");
	if (PlayerController && PlayerController->PlayerState)
	{
		PlayerID = PlayerController->PlayerState->GetUniqueId().ToString();
	}

	return PlayerSkillData.Find(PlayerID);
}
