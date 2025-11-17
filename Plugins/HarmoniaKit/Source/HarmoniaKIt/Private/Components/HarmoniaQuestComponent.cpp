// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaQuestComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Components/HarmoniaCraftingComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UHarmoniaQuestComponent::UHarmoniaQuestComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true; // Always tick for time-limited quests
	SetIsReplicatedByDefault(true);

	QuestDataTable = nullptr;
	InventoryComponent = nullptr;
	CraftingComponent = nullptr;
	TrackedQuest = FHarmoniaID();

	// Security: Rate limiting
	LastOperationTime = 0.0f;
	MinTimeBetweenOperations = 0.1f;
	MaxActiveQuests = 25;
}

void UHarmoniaQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache component references
	GetInventoryComponent();
	GetCraftingComponent();
}

void UHarmoniaQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only server updates time-limited quests
	if (GetOwnerRole() == ROLE_Authority)
	{
		UpdateTimeLimitedQuests(DeltaTime);
	}
}

void UHarmoniaQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaQuestComponent, ActiveQuests);
	DOREPLIFETIME(UHarmoniaQuestComponent, CompletedQuests);
	DOREPLIFETIME(UHarmoniaQuestComponent, FailedQuests);
	DOREPLIFETIME(UHarmoniaQuestComponent, TrackedQuest);
}

void UHarmoniaQuestComponent::OnRep_ActiveQuests()
{
	// Client-side notification when active quests change
	// UI can listen to delegate events
}

void UHarmoniaQuestComponent::OnRep_TrackedQuest()
{
	// Client-side notification when tracked quest changes
}

//~==============================================
//~ Quest Operations
//~==============================================

bool UHarmoniaQuestComponent::StartQuest(FHarmoniaID QuestId)
{
	if (!QuestId.IsValid())
	{
		return false;
	}

	// Client -> Server
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStartQuest(QuestId);
		return true;
	}

	// Check if already active
	if (IsQuestActive(QuestId))
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s is already active"), *QuestId.ToString());
		return false;
	}

	// Check if already completed
	if (IsQuestCompleted(QuestId))
	{
		// Check if repeatable
		FQuestData QuestData;
		if (GetQuestData(QuestId, QuestData))
		{
			if (QuestData.QuestType != EQuestType::Repeatable &&
				QuestData.QuestType != EQuestType::Daily &&
				QuestData.QuestType != EQuestType::Weekly)
			{
				UE_LOG(LogTemp, Warning, TEXT("Quest %s is already completed and not repeatable"), *QuestId.ToString());
				return false;
			}
		}
	}

	// Get quest data
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		UE_LOG(LogTemp, Error, TEXT("Quest %s not found in data table"), *QuestId.ToString());
		return false;
	}

	// Check if quest is available (conditions met)
	if (!IsQuestAvailable(QuestId))
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s is not available (conditions not met)"), *QuestId.ToString());
		return false;
	}

	// Check max active quests limit
	if (ActiveQuests.Num() >= MaxActiveQuests)
	{
		UE_LOG(LogTemp, Warning, TEXT("Max active quests limit reached (%d)"), MaxActiveQuests);
		return false;
	}

	// Create active quest progress
	FActiveQuestProgress NewProgress;
	NewProgress.QuestId = QuestId;
	NewProgress.State = EQuestState::InProgress;
	NewProgress.ObjectiveProgress = QuestData.Objectives;
	NewProgress.StartTime = GetWorld()->GetTimeSeconds();
	NewProgress.ElapsedTime = 0.0f;
	NewProgress.bTracked = false;

	// Add to active quests
	ActiveQuests.Add(NewProgress);

	// Broadcast event
	OnQuestStarted.Broadcast(QuestId, QuestData);

	// Notify client
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientQuestStarted(QuestId);
	}

	UE_LOG(LogTemp, Log, TEXT("Quest %s started"), *QuestId.ToString());
	return true;
}

bool UHarmoniaQuestComponent::CompleteQuest(FHarmoniaID QuestId, const TArray<int32>& SelectedOptionalRewards)
{
	if (!QuestId.IsValid())
	{
		return false;
	}

	// Client -> Server
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerCompleteQuest(QuestId, SelectedOptionalRewards);
		return true;
	}

	// Check if quest is active
	FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (!Progress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s is not active"), *QuestId.ToString());
		return false;
	}

	// Get quest data
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		UE_LOG(LogTemp, Error, TEXT("Quest %s not found in data table"), *QuestId.ToString());
		return false;
	}

	// Check if all required objectives are completed
	if (!Progress->AreObjectivesCompleted())
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s objectives not completed"), *QuestId.ToString());
		return false;
	}

	// Validate optional reward selection
	TArray<FQuestReward> AllRewards = QuestData.Rewards;
	if (SelectedOptionalRewards.Num() > 0)
	{
		if (SelectedOptionalRewards.Num() > QuestData.MaxOptionalRewardChoices)
		{
			UE_LOG(LogTemp, Warning, TEXT("Too many optional rewards selected"));
			return false;
		}

		for (int32 Index : SelectedOptionalRewards)
		{
			if (QuestData.OptionalRewards.IsValidIndex(Index))
			{
				AllRewards.Add(QuestData.OptionalRewards[Index]);
			}
		}
	}

	// Grant rewards
	GrantRewards(AllRewards);

	// Remove from active quests
	ActiveQuests.RemoveAll([QuestId](const FActiveQuestProgress& Item) {
		return Item.QuestId == QuestId;
	});

	// Add to completed quests
	if (!CompletedQuests.Contains(QuestId))
	{
		CompletedQuests.Add(QuestId);
	}

	// Remove from failed quests if it was there
	FailedQuests.Remove(QuestId);

	// Broadcast event
	OnQuestCompleted.Broadcast(QuestId, QuestData, AllRewards);

	// Notify client
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientQuestCompleted(QuestId, AllRewards);
	}

	// Check quest chain
	CheckQuestChain(QuestId);

	UE_LOG(LogTemp, Log, TEXT("Quest %s completed"), *QuestId.ToString());
	return true;
}

bool UHarmoniaQuestComponent::AbandonQuest(FHarmoniaID QuestId)
{
	if (!QuestId.IsValid())
	{
		return false;
	}

	// Client -> Server
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerAbandonQuest(QuestId);
		return true;
	}

	// Check if quest is active
	if (!IsQuestActive(QuestId))
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s is not active"), *QuestId.ToString());
		return false;
	}

	// Get quest data
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		UE_LOG(LogTemp, Error, TEXT("Quest %s not found in data table"), *QuestId.ToString());
		return false;
	}

	// Check if quest can be abandoned
	if (!QuestData.bCanAbandon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s cannot be abandoned"), *QuestId.ToString());
		return false;
	}

	// Remove from active quests
	ActiveQuests.RemoveAll([QuestId](const FActiveQuestProgress& Item) {
		return Item.QuestId == QuestId;
	});

	// Untrack if this was tracked quest
	if (TrackedQuest == QuestId)
	{
		TrackedQuest = FHarmoniaID();
	}

	// Broadcast event
	OnQuestAbandoned.Broadcast(QuestId, QuestData);

	// Notify client
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientQuestAbandoned(QuestId);
	}

	UE_LOG(LogTemp, Log, TEXT("Quest %s abandoned"), *QuestId.ToString());
	return true;
}

bool UHarmoniaQuestComponent::FailQuest(FHarmoniaID QuestId)
{
	if (!QuestId.IsValid())
	{
		return false;
	}

	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return false;
	}

	// Check if quest is active
	if (!IsQuestActive(QuestId))
	{
		return false;
	}

	// Get quest data
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return false;
	}

	// Remove from active quests
	ActiveQuests.RemoveAll([QuestId](const FActiveQuestProgress& Item) {
		return Item.QuestId == QuestId;
	});

	// Add to failed quests
	if (!FailedQuests.Contains(QuestId))
	{
		FailedQuests.Add(QuestId);
	}

	// Untrack if this was tracked quest
	if (TrackedQuest == QuestId)
	{
		TrackedQuest = FHarmoniaID();
	}

	// Broadcast event
	OnQuestFailed.Broadcast(QuestId, QuestData);

	// Notify client
	ClientQuestFailed(QuestId);

	UE_LOG(LogTemp, Log, TEXT("Quest %s failed"), *QuestId.ToString());
	return true;
}

bool UHarmoniaQuestComponent::UpdateQuestObjective(FHarmoniaID QuestId, int32 ObjectiveIndex, int32 Progress)
{
	if (!QuestId.IsValid() || Progress == 0)
	{
		return false;
	}

	// Client -> Server
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerUpdateObjective(QuestId, ObjectiveIndex, Progress);
		return true;
	}

	// Find active quest
	FActiveQuestProgress* QuestProgress = FindActiveQuest(QuestId);
	if (!QuestProgress)
	{
		return false;
	}

	// Validate objective index
	if (!QuestProgress->ObjectiveProgress.IsValidIndex(ObjectiveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid objective index %d for quest %s"), ObjectiveIndex, *QuestId.ToString());
		return false;
	}

	// Update objective
	FQuestObjective& Objective = QuestProgress->ObjectiveProgress[ObjectiveIndex];
	int32 OldCount = Objective.CurrentCount;
	Objective.CurrentCount = FMath::Clamp(Objective.CurrentCount + Progress, 0, Objective.RequiredCount);

	// Check if objective was completed
	bool bObjectiveCompleted = Objective.IsCompleted() && OldCount < Objective.RequiredCount;

	// Broadcast event
	OnQuestObjectiveUpdated.Broadcast(QuestId, ObjectiveIndex, Objective);

	// Notify client
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientObjectiveUpdated(QuestId, ObjectiveIndex);
	}

	// Check if all objectives are completed
	if (QuestProgress->AreObjectivesCompleted())
	{
		QuestProgress->State = EQuestState::ReadyToComplete;

		// Get quest data
		FQuestData QuestData;
		if (GetQuestData(QuestId, QuestData))
		{
			// Broadcast ready to complete event
			OnQuestReadyToComplete.Broadcast(QuestId, QuestData);

			// Auto-complete if enabled
			if (QuestData.bAutoComplete)
			{
				CompleteQuest(QuestId, TArray<int32>());
			}
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("Quest %s objective %d updated: %d/%d"),
		*QuestId.ToString(), ObjectiveIndex, Objective.CurrentCount, Objective.RequiredCount);

	return true;
}

void UHarmoniaQuestComponent::UpdateQuestObjectivesByType(EQuestObjectiveType ObjectiveType, FHarmoniaID TargetId, int32 Progress)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	// Update all matching objectives in active quests
	for (FActiveQuestProgress& QuestProgress : ActiveQuests)
	{
		for (int32 i = 0; i < QuestProgress.ObjectiveProgress.Num(); ++i)
		{
			FQuestObjective& Objective = QuestProgress.ObjectiveProgress[i];

			// Check if objective matches
			if (Objective.ObjectiveType == ObjectiveType &&
				Objective.TargetId == TargetId &&
				!Objective.IsCompleted())
			{
				UpdateQuestObjective(QuestProgress.QuestId, i, Progress);
			}
		}
	}
}

void UHarmoniaQuestComponent::UpdateQuestObjectivesByTags(EQuestObjectiveType ObjectiveType, FGameplayTagContainer TargetTags, int32 Progress)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	// Update all matching objectives in active quests
	for (FActiveQuestProgress& QuestProgress : ActiveQuests)
	{
		for (int32 i = 0; i < QuestProgress.ObjectiveProgress.Num(); ++i)
		{
			FQuestObjective& Objective = QuestProgress.ObjectiveProgress[i];

			// Check if objective matches by tags
			if (Objective.ObjectiveType == ObjectiveType &&
				!Objective.IsCompleted() &&
				Objective.TargetTags.HasAny(TargetTags))
			{
				UpdateQuestObjective(QuestProgress.QuestId, i, Progress);
			}
		}
	}
}

void UHarmoniaQuestComponent::SetTrackedQuest(FHarmoniaID QuestId)
{
	// Client -> Server
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerSetTrackedQuest(QuestId);
		return;
	}

	// Validate quest is active (or allow invalid to untrack)
	if (QuestId.IsValid() && !IsQuestActive(QuestId))
	{
		return;
	}

	TrackedQuest = QuestId;

	// Update tracked flag on quest progress
	for (FActiveQuestProgress& Progress : ActiveQuests)
	{
		Progress.bTracked = (Progress.QuestId == QuestId);
	}
}

//~==============================================
//~ Quest Queries
//~==============================================

bool UHarmoniaQuestComponent::GetQuestData(FHarmoniaID QuestId, FQuestData& OutQuestData) const
{
	if (!QuestDataTable || !QuestId.IsValid())
	{
		return false;
	}

	FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(QuestId.Id, TEXT("GetQuestData"));
	if (QuestData)
	{
		OutQuestData = *QuestData;
		return true;
	}

	return false;
}

bool UHarmoniaQuestComponent::GetActiveQuestProgress(FHarmoniaID QuestId, FActiveQuestProgress& OutProgress) const
{
	const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (Progress)
	{
		OutProgress = *Progress;
		return true;
	}
	return false;
}

bool UHarmoniaQuestComponent::IsQuestActive(FHarmoniaID QuestId) const
{
	return FindActiveQuest(QuestId) != nullptr;
}

bool UHarmoniaQuestComponent::IsQuestCompleted(FHarmoniaID QuestId) const
{
	return CompletedQuests.Contains(QuestId);
}

bool UHarmoniaQuestComponent::IsQuestFailed(FHarmoniaID QuestId) const
{
	return FailedQuests.Contains(QuestId);
}

bool UHarmoniaQuestComponent::IsQuestReadyToComplete(FHarmoniaID QuestId) const
{
	const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (Progress)
	{
		return Progress->AreObjectivesCompleted();
	}
	return false;
}

TArray<FHarmoniaID> UHarmoniaQuestComponent::GetAvailableQuests() const
{
	TArray<FHarmoniaID> AvailableQuests;

	if (!QuestDataTable)
	{
		return AvailableQuests;
	}

	// Get all quest rows
	TArray<FQuestData*> AllQuests;
	QuestDataTable->GetAllRows<FQuestData>(TEXT("GetAvailableQuests"), AllQuests);

	// Check each quest
	for (FQuestData* QuestData : AllQuests)
	{
		if (QuestData && IsQuestAvailable(QuestData->QuestId))
		{
			AvailableQuests.Add(QuestData->QuestId);
		}
	}

	return AvailableQuests;
}

bool UHarmoniaQuestComponent::IsQuestAvailable(FHarmoniaID QuestId) const
{
	// Can't be available if already active
	if (IsQuestActive(QuestId))
	{
		return false;
	}

	// Get quest data
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return false;
	}

	// Check if already completed (and not repeatable)
	if (IsQuestCompleted(QuestId))
	{
		if (QuestData.QuestType != EQuestType::Repeatable &&
			QuestData.QuestType != EQuestType::Daily &&
			QuestData.QuestType != EQuestType::Weekly)
		{
			return false;
		}
	}

	// Check unlock conditions
	return CheckQuestConditions(QuestData.UnlockConditions);
}

EQuestState UHarmoniaQuestComponent::GetQuestState(FHarmoniaID QuestId) const
{
	if (IsQuestActive(QuestId))
	{
		const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
		if (Progress)
		{
			return Progress->State;
		}
		return EQuestState::InProgress;
	}
	else if (IsQuestCompleted(QuestId))
	{
		return EQuestState::Completed;
	}
	else if (IsQuestFailed(QuestId))
	{
		return EQuestState::Failed;
	}
	else if (IsQuestAvailable(QuestId))
	{
		return EQuestState::Available;
	}
	else
	{
		return EQuestState::Locked;
	}
}

//~==============================================
//~ Quest Conditions
//~==============================================

bool UHarmoniaQuestComponent::CheckQuestConditions(const TArray<FQuestCondition>& Conditions) const
{
	// If no conditions, always true
	if (Conditions.Num() == 0)
	{
		return true;
	}

	// All conditions must be met
	for (const FQuestCondition& Condition : Conditions)
	{
		if (!CheckQuestCondition(Condition))
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaQuestComponent::CheckQuestCondition(const FQuestCondition& Condition) const
{
	switch (Condition.ConditionType)
	{
		case EQuestConditionType::None:
			return true;

		case EQuestConditionType::QuestCompleted:
			return IsQuestCompleted(Condition.TargetQuestId);

		case EQuestConditionType::QuestNotCompleted:
			return !IsQuestCompleted(Condition.TargetQuestId);

		case EQuestConditionType::Level:
			return GetCharacterLevel() >= Condition.RequiredLevel;

		case EQuestConditionType::HasItem:
		{
			UHarmoniaInventoryComponent* Inventory = const_cast<UHarmoniaQuestComponent*>(this)->GetInventoryComponent();
			if (Inventory)
			{
				// Check if player has required item amount
				// Note: You'll need to implement GetItemCount in inventory component
				// For now, just return true as placeholder
				return true;
			}
			return false;
		}

		case EQuestConditionType::HasTag:
		{
			FGameplayTagContainer CharacterTags = GetCharacterTags();
			return CharacterTags.HasAll(Condition.RequiredTags);
		}

		case EQuestConditionType::RecipeKnown:
		{
			UHarmoniaCraftingComponent* Crafting = const_cast<UHarmoniaQuestComponent*>(this)->GetCraftingComponent();
			if (Crafting)
			{
				return Crafting->HasLearnedRecipe(Condition.RequiredRecipeId);
			}
			return false;
		}

		case EQuestConditionType::Custom:
			// Custom conditions can be handled by overriding this function
			return true;

		default:
			return false;
	}
}

//~==============================================
//~ Quest Rewards
//~==============================================

void UHarmoniaQuestComponent::GrantRewards(const TArray<FQuestReward>& Rewards)
{
	for (const FQuestReward& Reward : Rewards)
	{
		GrantReward(Reward);
	}
}

void UHarmoniaQuestComponent::GrantReward(const FQuestReward& Reward)
{
	switch (Reward.RewardType)
	{
		case EQuestRewardType::Experience:
		{
			// Grant experience
			// Note: You'll need to implement experience system
			// For now, log the reward
			UE_LOG(LogTemp, Log, TEXT("Granted %d experience"), Reward.ExperienceAmount);
			break;
		}

		case EQuestRewardType::Gold:
		{
			// Grant gold
			// Note: You'll need to implement currency system
			UE_LOG(LogTemp, Log, TEXT("Granted %d gold"), Reward.GoldAmount);
			break;
		}

		case EQuestRewardType::Item:
		{
			// Grant item to inventory
			UHarmoniaInventoryComponent* Inventory = GetInventoryComponent();
			if (Inventory && Reward.ItemId.IsValid())
			{
				// Note: You'll need to implement AddItem in inventory component
				UE_LOG(LogTemp, Log, TEXT("Granted item %s x%d"), *Reward.ItemId.ToString(), Reward.ItemAmount);
			}
			break;
		}

		case EQuestRewardType::Recipe:
		{
			// Learn recipe
			UHarmoniaCraftingComponent* Crafting = GetCraftingComponent();
			if (Crafting && Reward.RecipeId.IsValid())
			{
				Crafting->LearnRecipe(Reward.RecipeId);
				UE_LOG(LogTemp, Log, TEXT("Learned recipe %s"), *Reward.RecipeId.ToString());
			}
			break;
		}

		case EQuestRewardType::Tag:
		{
			// Grant gameplay tags
			// Note: You'll need to implement tag granting system
			UE_LOG(LogTemp, Log, TEXT("Granted gameplay tags"));
			break;
		}

		case EQuestRewardType::UnlockQuest:
		{
			// Unlock quest (notify via delegate)
			if (Reward.QuestToUnlock.IsValid())
			{
				FQuestData UnlockedQuestData;
				if (GetQuestData(Reward.QuestToUnlock, UnlockedQuestData))
				{
					OnQuestUnlocked.Broadcast(Reward.QuestToUnlock, UnlockedQuestData);
				}
			}
			break;
		}

		case EQuestRewardType::Custom:
			// Custom rewards can be handled by overriding this function
			break;

		default:
			break;
	}
}

//~==============================================
//~ Quest Chain System
//~==============================================

void UHarmoniaQuestComponent::CheckQuestChain(FHarmoniaID CompletedQuestId)
{
	// Get quest data
	FQuestData QuestData;
	if (!GetQuestData(CompletedQuestId, QuestData))
	{
		return;
	}

	// Auto-start next quests in chain
	for (const FHarmoniaID& NextQuestId : QuestData.NextQuests)
	{
		if (NextQuestId.IsValid() && IsQuestAvailable(NextQuestId))
		{
			StartQuest(NextQuestId);
		}
	}
}

//~==============================================
//~ Time-Limited Quests
//~==============================================

void UHarmoniaQuestComponent::UpdateTimeLimitedQuests(float DeltaTime)
{
	TArray<FHarmoniaID> QuestsToFail;

	// Update elapsed time for all active quests
	for (FActiveQuestProgress& Progress : ActiveQuests)
	{
		Progress.ElapsedTime += DeltaTime;

		// Check time limit
		FQuestData QuestData;
		if (GetQuestData(Progress.QuestId, QuestData))
		{
			if (Progress.IsTimeUp(QuestData))
			{
				QuestsToFail.Add(Progress.QuestId);
			}
		}
	}

	// Fail quests that exceeded time limit
	for (const FHarmoniaID& QuestId : QuestsToFail)
	{
		FailQuest(QuestId);
	}
}

bool UHarmoniaQuestComponent::CheckQuestTimeLimit(FHarmoniaID QuestId, const FActiveQuestProgress& Progress)
{
	FQuestData QuestData;
	if (GetQuestData(QuestId, QuestData))
	{
		return Progress.IsTimeUp(QuestData);
	}
	return false;
}

//~==============================================
//~ Server/Client RPCs
//~==============================================

void UHarmoniaQuestComponent::ServerStartQuest_Implementation(FHarmoniaID QuestId)
{
	StartQuest(QuestId);
}

bool UHarmoniaQuestComponent::ServerStartQuest_Validate(FHarmoniaID QuestId)
{
	return QuestId.IsValid();
}

void UHarmoniaQuestComponent::ServerCompleteQuest_Implementation(FHarmoniaID QuestId, const TArray<int32>& SelectedOptionalRewards)
{
	CompleteQuest(QuestId, SelectedOptionalRewards);
}

bool UHarmoniaQuestComponent::ServerCompleteQuest_Validate(FHarmoniaID QuestId, const TArray<int32>& SelectedOptionalRewards)
{
	return QuestId.IsValid();
}

void UHarmoniaQuestComponent::ServerAbandonQuest_Implementation(FHarmoniaID QuestId)
{
	AbandonQuest(QuestId);
}

bool UHarmoniaQuestComponent::ServerAbandonQuest_Validate(FHarmoniaID QuestId)
{
	return QuestId.IsValid();
}

void UHarmoniaQuestComponent::ServerUpdateObjective_Implementation(FHarmoniaID QuestId, int32 ObjectiveIndex, int32 Progress)
{
	UpdateQuestObjective(QuestId, ObjectiveIndex, Progress);
}

bool UHarmoniaQuestComponent::ServerUpdateObjective_Validate(FHarmoniaID QuestId, int32 ObjectiveIndex, int32 Progress)
{
	return QuestId.IsValid() && ObjectiveIndex >= 0;
}

void UHarmoniaQuestComponent::ServerSetTrackedQuest_Implementation(FHarmoniaID QuestId)
{
	SetTrackedQuest(QuestId);
}

bool UHarmoniaQuestComponent::ServerSetTrackedQuest_Validate(FHarmoniaID QuestId)
{
	return true; // Allow invalid to untrack
}

void UHarmoniaQuestComponent::ClientQuestStarted_Implementation(FHarmoniaID QuestId)
{
	// Client notification
}

void UHarmoniaQuestComponent::ClientQuestCompleted_Implementation(FHarmoniaID QuestId, const TArray<FQuestReward>& Rewards)
{
	// Client notification
}

void UHarmoniaQuestComponent::ClientQuestFailed_Implementation(FHarmoniaID QuestId)
{
	// Client notification
}

void UHarmoniaQuestComponent::ClientQuestAbandoned_Implementation(FHarmoniaID QuestId)
{
	// Client notification
}

void UHarmoniaQuestComponent::ClientObjectiveUpdated_Implementation(FHarmoniaID QuestId, int32 ObjectiveIndex)
{
	// Client notification
}

//~==============================================
//~ Save/Load System
//~==============================================

FQuestSaveData UHarmoniaQuestComponent::GetQuestSaveData() const
{
	FQuestSaveData SaveData;
	SaveData.ActiveQuests = ActiveQuests;
	SaveData.CompletedQuests = CompletedQuests;
	SaveData.FailedQuests = FailedQuests;
	return SaveData;
}

void UHarmoniaQuestComponent::LoadQuestFromSaveData(const FQuestSaveData& SaveData)
{
	ActiveQuests = SaveData.ActiveQuests;
	CompletedQuests = SaveData.CompletedQuests;
	FailedQuests = SaveData.FailedQuests;
}

//~==============================================
//~ Utility Functions
//~==============================================

UHarmoniaInventoryComponent* UHarmoniaQuestComponent::GetInventoryComponent()
{
	if (!InventoryComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
		}
	}
	return InventoryComponent;
}

UHarmoniaCraftingComponent* UHarmoniaQuestComponent::GetCraftingComponent()
{
	if (!CraftingComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			CraftingComponent = Owner->FindComponentByClass<UHarmoniaCraftingComponent>();
		}
	}
	return CraftingComponent;
}

int32 UHarmoniaQuestComponent::GetCharacterLevel() const
{
	// Override in subclass if you have a level system
	// For now, return 1 as default
	return 1;
}

FGameplayTagContainer UHarmoniaQuestComponent::GetCharacterTags() const
{
	// Try to get tags from Ability System Component
	AActor* Owner = GetOwner();
	if (Owner)
	{
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
		if (ASC)
		{
			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);
			return OwnedTags;
		}
	}

	return FGameplayTagContainer();
}

FActiveQuestProgress* UHarmoniaQuestComponent::FindActiveQuest(FHarmoniaID QuestId)
{
	return ActiveQuests.FindByPredicate([QuestId](const FActiveQuestProgress& Item) {
		return Item.QuestId == QuestId;
	});
}

const FActiveQuestProgress* UHarmoniaQuestComponent::FindActiveQuest(FHarmoniaID QuestId) const
{
	return ActiveQuests.FindByPredicate([QuestId](const FActiveQuestProgress& Item) {
		return Item.QuestId == QuestId;
	});
}

void UHarmoniaQuestComponent::CheckAutoComplete(FHarmoniaID QuestId)
{
	FQuestData QuestData;
	if (GetQuestData(QuestId, QuestData) && QuestData.bAutoComplete)
	{
		if (IsQuestReadyToComplete(QuestId))
		{
			CompleteQuest(QuestId, TArray<int32>());
		}
	}
}

//~==============================================
//~ Debug
//~==============================================

#if WITH_EDITOR
void UHarmoniaQuestComponent::Debug_CompleteQuestObjectives(FHarmoniaID QuestId)
{
	FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (Progress)
	{
		for (int32 i = 0; i < Progress->ObjectiveProgress.Num(); ++i)
		{
			FQuestObjective& Objective = Progress->ObjectiveProgress[i];
			Objective.CurrentCount = Objective.RequiredCount;
		}
		Progress->State = EQuestState::ReadyToComplete;

		UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Quest %s objectives completed"), *QuestId.ToString());
	}
}

void UHarmoniaQuestComponent::Debug_UnlockAllQuests()
{
	// Clear all completion/failure data
	CompletedQuests.Empty();
	FailedQuests.Empty();

	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] All quests unlocked"));
}

void UHarmoniaQuestComponent::Debug_ResetAllQuests()
{
	ActiveQuests.Empty();
	CompletedQuests.Empty();
	FailedQuests.Empty();
	TrackedQuest = FHarmoniaID();

	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] All quests reset"));
}
#endif
