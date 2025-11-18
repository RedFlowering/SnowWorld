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

	// Only server updates time-limited quests and fail conditions
	if (GetOwnerRole() == ROLE_Authority)
	{
		UpdateTimeLimitedQuests(DeltaTime);
		CheckFailConditions(DeltaTime);
		UpdateHintSystem(DeltaTime);
	}
}

void UHarmoniaQuestComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaQuestComponent, ActiveQuests);
	DOREPLIFETIME(UHarmoniaQuestComponent, CompletedQuests);
	DOREPLIFETIME(UHarmoniaQuestComponent, FailedQuests);
	DOREPLIFETIME(UHarmoniaQuestComponent, TrackedQuest);
	// Note: QuestStatistics and QuestLog are not replicated (contain TMap which can't be replicated)
	// Clients can query them via GetQuestStatistics() and GetQuestLogEntry()
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
	NewProgress.CurrentPhase = 0;

	// If quest uses phases, set objectives from first phase
	if (QuestData.Phases.Num() > 0)
	{
		NewProgress.ObjectiveProgress = QuestData.Phases[0].PhaseObjectives;
	}
	else
	{
		NewProgress.ObjectiveProgress = QuestData.Objectives;
	}

	NewProgress.StartTime = GetWorld()->GetTimeSeconds();
	NewProgress.ElapsedTime = 0.0f;
	NewProgress.bTracked = false;

	// Add to active quests
	ActiveQuests.Add(NewProgress);

	// Update quest log
	UpdateQuestLog(QuestId, QuestData, false);

	// Trigger OnStart events
	TriggerQuestEvents(QuestId, EQuestEventTrigger::OnStart);

	// Show notification
	ShowNotification(EQuestNotificationType::QuestStarted, QuestId,
		FText::Format(FText::FromString(TEXT("Quest Started: {0}")), QuestData.QuestName));

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

	// Calculate completion time
	float CompletionTime = Progress->ElapsedTime;

	// Grant rewards
	GrantRewards(AllRewards);

	// Check and grant bonus rewards
	for (const FBonusObjective& BonusObj : QuestData.BonusObjectives)
	{
		// Check if bonus objective was completed (this would need custom tracking)
		// For now, we skip bonus objectives - they would need progress tracking
	}

	// Update statistics
	UpdateQuestStatistics(QuestId, QuestData, CompletionTime);

	// Update quest log
	UpdateQuestLog(QuestId, QuestData, true);

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

	// Trigger OnComplete events
	TriggerQuestEvents(QuestId, EQuestEventTrigger::OnComplete);

	// Show notification
	ShowNotification(EQuestNotificationType::QuestCompleted, QuestId,
		FText::Format(FText::FromString(TEXT("Quest Completed: {0}")), QuestData.QuestName));

	// Broadcast event
	OnQuestCompleted.Broadcast(QuestId, QuestData, AllRewards);

	// Notify client
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientQuestCompleted(QuestId, AllRewards);
	}

	// Check quest chain
	CheckQuestChain(QuestId);

	UE_LOG(LogTemp, Log, TEXT("Quest %s completed in %.2f seconds"), *QuestId.ToString(), CompletionTime);
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

	// Trigger OnAbandon events
	TriggerQuestEvents(QuestId, EQuestEventTrigger::OnAbandon);

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

	// Trigger OnFail events
	TriggerQuestEvents(QuestId, EQuestEventTrigger::OnFail);

	// Show notification
	ShowNotification(EQuestNotificationType::QuestFailed, QuestId,
		FText::Format(FText::FromString(TEXT("Quest Failed: {0}")), QuestData.QuestName));

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

	// Show notification for objective completion
	if (bObjectiveCompleted)
	{
		ShowNotification(EQuestNotificationType::ObjectiveComplete, QuestId, Objective.Description);
		TriggerQuestEvents(QuestId, EQuestEventTrigger::OnObjectiveComplete);
	}

	// Notify client
	if (GetOwnerRole() == ROLE_Authority)
	{
		ClientObjectiveUpdated(QuestId, ObjectiveIndex);
	}

	// Get quest data
	FQuestData QuestData;
	if (GetQuestData(QuestId, QuestData))
	{
		// Check if all objectives are completed
		if (QuestProgress->AreObjectivesCompleted())
		{
			// Check if this is a phased quest
			if (QuestData.Phases.Num() > 0)
			{
				// Check if we should auto-advance to next phase
				int32 NextPhase = QuestProgress->CurrentPhase + 1;
				if (NextPhase < QuestData.Phases.Num())
				{
					// Phase complete notification
					ShowNotification(EQuestNotificationType::PhaseComplete, QuestId,
						FText::Format(FText::FromString(TEXT("Phase {0} Complete")), FText::AsNumber(QuestProgress->CurrentPhase + 1)));

					// Auto-advance if enabled
					const FQuestPhase& CurrentPhase = QuestData.Phases[QuestProgress->CurrentPhase];
					if (CurrentPhase.bAutoAdvance)
					{
						AdvanceToNextPhase(QuestId);
					}
				}
				else
				{
					// Last phase complete, quest is ready to complete
					QuestProgress->State = EQuestState::ReadyToComplete;
					OnQuestReadyToComplete.Broadcast(QuestId, QuestData);

					if (QuestData.bAutoComplete)
					{
						CompleteQuest(QuestId, TArray<int32>());
					}
				}
			}
			else
			{
				// Non-phased quest, ready to complete
				QuestProgress->State = EQuestState::ReadyToComplete;
				OnQuestReadyToComplete.Broadcast(QuestId, QuestData);

				// Auto-complete if enabled
				if (QuestData.bAutoComplete)
				{
					CompleteQuest(QuestId, TArray<int32>());
				}
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
	// Anti-cheat: Validate quest ID
	if (!QuestId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerUpdateObjective: Invalid QuestId"));
		return false;
	}

	// Validate objective index lower bound
	if (ObjectiveIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerUpdateObjective: Negative objective index %d"), ObjectiveIndex);
		return false;
	}

	// Validate quest is actually active
	const FActiveQuestProgress* QuestProgress = FindActiveQuest(QuestId);
	if (!QuestProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerUpdateObjective: Quest not active %s"), *QuestId.ToString());
		return false;
	}

	// Validate objective index upper bound
	if (!QuestProgress->ObjectiveProgress.IsValidIndex(ObjectiveIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerUpdateObjective: Invalid objective index %d (Max: %d)"),
			ObjectiveIndex, QuestProgress->ObjectiveProgress.Num() - 1);
		return false;
	}

	// Validate progress amount is reasonable
	if (FMath::Abs(Progress) > 1000)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerUpdateObjective: Suspicious progress amount %d"), Progress);
		return false;
	}

	return true;
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
//~ Quest Phase System
//~==============================================

int32 UHarmoniaQuestComponent::GetCurrentPhase(FHarmoniaID QuestId) const
{
	const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (Progress)
	{
		return Progress->CurrentPhase;
	}
	return -1;
}

bool UHarmoniaQuestComponent::AdvanceToNextPhase(FHarmoniaID QuestId)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return false;
	}

	FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (!Progress)
	{
		return false;
	}

	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return false;
	}

	// Check if quest uses phases
	if (QuestData.Phases.Num() == 0)
	{
		return false;
	}

	// Check if current phase objectives are complete
	if (!ArePhaseObjectivesComplete(QuestId, Progress->CurrentPhase))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot advance phase: current phase objectives not complete"));
		return false;
	}

	int32 NextPhase = Progress->CurrentPhase + 1;
	if (NextPhase >= QuestData.Phases.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot advance: already at last phase"));
		return false;
	}

	// Trigger current phase completion events
	TriggerPhaseEvents(QuestId, Progress->CurrentPhase);

	// Advance to next phase
	Progress->CurrentPhase = NextPhase;

	// Update objectives for new phase
	if (QuestData.Phases.IsValidIndex(NextPhase))
	{
		Progress->ObjectiveProgress = QuestData.Phases[NextPhase].PhaseObjectives;
	}

	// Trigger next phase start events
	TriggerQuestEvents(QuestId, EQuestEventTrigger::OnPhaseChange);

	UE_LOG(LogTemp, Log, TEXT("Quest %s advanced to phase %d"), *QuestId.ToString(), NextPhase);
	return true;
}

bool UHarmoniaQuestComponent::GetPhaseObjectives(FHarmoniaID QuestId, int32 PhaseNumber, TArray<FQuestObjective>& OutObjectives) const
{
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return false;
	}

	if (!QuestData.Phases.IsValidIndex(PhaseNumber))
	{
		return false;
	}

	OutObjectives = QuestData.Phases[PhaseNumber].PhaseObjectives;
	return true;
}

bool UHarmoniaQuestComponent::ArePhaseObjectivesComplete(FHarmoniaID QuestId, int32 PhaseNumber) const
{
	const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (!Progress)
	{
		return false;
	}

	// If not on the requested phase, return false
	if (Progress->CurrentPhase != PhaseNumber)
	{
		return false;
	}

	// Check if all objectives are complete
	return Progress->AreObjectivesCompleted();
}

void UHarmoniaQuestComponent::TriggerPhaseEvents(FHarmoniaID QuestId, int32 PhaseNumber)
{
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return;
	}

	if (!QuestData.Phases.IsValidIndex(PhaseNumber))
	{
		return;
	}

	// Trigger phase-specific events
	const FQuestPhase& Phase = QuestData.Phases[PhaseNumber];
	for (const FQuestEvent& Event : Phase.PhaseEvents)
	{
		ExecuteQuestEvent(Event, QuestId);
	}
}

//~==============================================
//~ Quest Marker System
//~==============================================

TArray<FQuestMarker> UHarmoniaQuestComponent::GetActiveMarkers(FHarmoniaID QuestId) const
{
	TArray<FQuestMarker> Markers;

	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return Markers;
	}

	const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (!Progress)
	{
		return Markers;
	}

	// If using phases, get markers from current phase
	if (QuestData.Phases.Num() > 0 && QuestData.Phases.IsValidIndex(Progress->CurrentPhase))
	{
		Markers = QuestData.Phases[Progress->CurrentPhase].PhaseMarkers;
	}
	else
	{
		// Otherwise use global markers
		Markers = QuestData.Markers;
	}

	return Markers;
}

TArray<FQuestMarker> UHarmoniaQuestComponent::GetAllActiveMarkers() const
{
	TArray<FQuestMarker> AllMarkers;

	for (const FActiveQuestProgress& Progress : ActiveQuests)
	{
		TArray<FQuestMarker> QuestMarkers = GetActiveMarkers(Progress.QuestId);
		AllMarkers.Append(QuestMarkers);
	}

	return AllMarkers;
}

void UHarmoniaQuestComponent::UpdateMarkerActor(FHarmoniaID QuestId, int32 MarkerIndex, AActor* NewActor)
{
	// This would require storing marker runtime data in ActiveQuestProgress
	// For now, this is a placeholder for runtime marker updates
	UE_LOG(LogTemp, Log, TEXT("Marker actor updated for quest %s, marker %d"), *QuestId.ToString(), MarkerIndex);
}

//~==============================================
//~ Quest Hint System
//~==============================================

TArray<FQuestHint> UHarmoniaQuestComponent::GetAvailableHints(FHarmoniaID QuestId)
{
	TArray<FQuestHint> AvailableHints;

	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return AvailableHints;
	}

	const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
	if (!Progress)
	{
		return AvailableHints;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	float QuestTime = CurrentTime - Progress->StartTime;

	// Check each hint if it should be shown
	for (const FQuestHint& Hint : QuestData.Hints)
	{
		// Check if already shown
		if (Hint.bShown)
		{
			continue;
		}

		// Check time delay
		if (QuestTime < Hint.ShowAfterSeconds)
		{
			continue;
		}

		// Check if quest is stuck (no progress for a while)
		if (Hint.bShowIfStuck && Progress->ElapsedTime < Hint.ShowAfterSeconds)
		{
			continue;
		}

		AvailableHints.Add(Hint);
	}

	return AvailableHints;
}

void UHarmoniaQuestComponent::MarkHintShown(FHarmoniaID QuestId, int32 HintIndex)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return;
	}

	// Note: This modifies the data table, which is not ideal for runtime
	// In production, you'd want to store shown hints in ActiveQuestProgress
	if (QuestData.Hints.IsValidIndex(HintIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("Hint %d marked as shown for quest %s"), HintIndex, *QuestId.ToString());

		// Show notification
		ShowNotification(EQuestNotificationType::HintShown, QuestId, QuestData.Hints[HintIndex].HintText);
	}
}

void UHarmoniaQuestComponent::UpdateHintSystem(float DeltaTime)
{
	// Check hints for all active quests
	for (const FActiveQuestProgress& Progress : ActiveQuests)
	{
		TArray<FQuestHint> Hints = GetAvailableHints(Progress.QuestId);

		// Auto-show hints if any are available
		for (int32 i = 0; i < Hints.Num(); ++i)
		{
			if (!Hints[i].bShown)
			{
				// In a real implementation, you'd notify UI to show the hint
				UE_LOG(LogTemp, Verbose, TEXT("Hint available for quest %s: %s"),
					*Progress.QuestId.ToString(), *Hints[i].HintText.ToString());
			}
		}
	}
}

//~==============================================
//~ Quest Statistics
//~==============================================

bool UHarmoniaQuestComponent::GetQuestLogEntry(FHarmoniaID QuestId, FQuestLogEntry& OutEntry) const
{
	const FQuestLogEntry* Entry = QuestLog.FindByPredicate([QuestId](const FQuestLogEntry& Item) {
		return Item.QuestId == QuestId;
	});

	if (Entry)
	{
		OutEntry = *Entry;
		return true;
	}

	return false;
}

void UHarmoniaQuestComponent::AddPlayerNote(FHarmoniaID QuestId, const FString& Note)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	FQuestLogEntry* Entry = QuestLog.FindByPredicate([QuestId](FQuestLogEntry& Item) {
		return Item.QuestId == QuestId;
	});

	if (Entry)
	{
		Entry->PlayerNotes = Note;
		UE_LOG(LogTemp, Log, TEXT("Player note added to quest %s"), *QuestId.ToString());
	}
}

void UHarmoniaQuestComponent::UpdateQuestStatistics(FHarmoniaID QuestId, const FQuestData& QuestData, float CompletionTime)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	// Update total completed
	QuestStatistics.TotalQuestsCompleted++;

	// Update by type
	int32& TypeCount = QuestStatistics.CompletedByType.FindOrAdd(QuestData.QuestType, 0);
	TypeCount++;

	// Update streak
	FDateTime CurrentDate = FDateTime::Now();
	if (QuestStatistics.LastCompletionDate.GetDate() == CurrentDate.GetDate())
	{
		// Same day, increment streak
		QuestStatistics.CurrentStreak++;
	}
	else if ((CurrentDate - QuestStatistics.LastCompletionDate).GetDays() == 1)
	{
		// Next day, continue streak
		QuestStatistics.CurrentStreak++;
	}
	else
	{
		// Streak broken, reset
		QuestStatistics.CurrentStreak = 1;
	}

	QuestStatistics.LastCompletionDate = CurrentDate;

	// Update fastest times
	if (!QuestStatistics.FastestCompletionTimes.Contains(QuestId) ||
		CompletionTime < QuestStatistics.FastestCompletionTimes[QuestId])
	{
		QuestStatistics.FastestCompletionTimes.Add(QuestId, CompletionTime);
	}

	UE_LOG(LogTemp, Log, TEXT("Quest statistics updated: %d total, streak: %d"),
		QuestStatistics.TotalQuestsCompleted, QuestStatistics.CurrentStreak);
}

void UHarmoniaQuestComponent::UpdateQuestLog(FHarmoniaID QuestId, const FQuestData& QuestData, bool bCompleted)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	FQuestLogEntry* ExistingEntry = QuestLog.FindByPredicate([QuestId](FQuestLogEntry& Item) {
		return Item.QuestId == QuestId;
	});

	if (ExistingEntry)
	{
		// Update existing entry
		ExistingEntry->bCompleted = bCompleted;
		ExistingEntry->LastUpdated = FDateTime::Now();
	}
	else
	{
		// Create new entry
		FQuestLogEntry NewEntry;
		NewEntry.QuestId = QuestId;
		NewEntry.QuestName = QuestData.QuestName;
		NewEntry.bCompleted = bCompleted;
		NewEntry.Priority = QuestData.Priority;
		NewEntry.LastUpdated = FDateTime::Now();
		QuestLog.Add(NewEntry);
	}
}

//~==============================================
//~ Quest Party System
//~==============================================

bool UHarmoniaQuestComponent::ShareQuestWithParty(FHarmoniaID QuestId)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return false;
	}

	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return false;
	}

	// Check if quest can be shared
	if (!QuestData.bCanShare)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest %s cannot be shared"), *QuestId.ToString());
		return false;
	}

	// Get party members
	TArray<APlayerController*> PartyMembers = GetPartyMembers();

	// Check party size requirements
	int32 PartySize = GetPartySize();
	if (PartySize < QuestData.MinPartySize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Party too small: %d (min: %d)"), PartySize, QuestData.MinPartySize);
		return false;
	}

	if (PartySize > QuestData.MaxPartySize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Party too large: %d (max: %d)"), PartySize, QuestData.MaxPartySize);
		return false;
	}

	// Share quest with all party members
	for (APlayerController* Member : PartyMembers)
	{
		if (Member && Member->GetPawn())
		{
			UHarmoniaQuestComponent* MemberQuestComp = Member->GetPawn()->FindComponentByClass<UHarmoniaQuestComponent>();
			if (MemberQuestComp)
			{
				MemberQuestComp->StartQuest(QuestId);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Quest %s shared with party (%d members)"), *QuestId.ToString(), PartyMembers.Num());
	return true;
}

int32 UHarmoniaQuestComponent::GetPartySize() const
{
	// Override in subclass or game-specific implementation
	// Default: single player
	return 1;
}

TArray<APlayerController*> UHarmoniaQuestComponent::GetPartyMembers() const
{
	// Override in subclass or game-specific implementation
	// Default: return owner's controller
	TArray<APlayerController*> Members;

	AActor* Owner = GetOwner();
	if (Owner)
	{
		APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController());
		if (PC)
		{
			Members.Add(PC);
		}
	}

	return Members;
}

//~==============================================
//~ Quest Event System
//~==============================================

void UHarmoniaQuestComponent::TriggerQuestEvents(FHarmoniaID QuestId, EQuestEventTrigger TriggerType)
{
	FQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return;
	}

	// Trigger events matching the trigger type
	for (const FQuestEvent& Event : QuestData.Events)
	{
		if (Event.TriggerType == TriggerType)
		{
			ExecuteQuestEvent(Event, QuestId);
		}
	}
}

void UHarmoniaQuestComponent::ExecuteQuestEvent(const FQuestEvent& Event, FHarmoniaID QuestId)
{
	switch (Event.EventType)
	{
		case EQuestEventType::SpawnActor:
		{
			if (Event.ActorToSpawn && Event.SpawnLocation != FVector::ZeroVector)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				GetWorld()->SpawnActor<AActor>(Event.ActorToSpawn, Event.SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				UE_LOG(LogTemp, Log, TEXT("Quest event: Spawned actor for quest %s"), *QuestId.ToString());
			}
			break;
		}

		case EQuestEventType::PlayCutscene:
		{
			// Trigger cutscene (implement based on your cutscene system)
			UE_LOG(LogTemp, Log, TEXT("Quest event: Play cutscene for quest %s"), *QuestId.ToString());
			break;
		}

		case EQuestEventType::GrantReward:
		{
			// Grant additional reward
			if (Event.BonusReward.RewardType != EQuestRewardType::None)
			{
				GrantReward(Event.BonusReward);
				UE_LOG(LogTemp, Log, TEXT("Quest event: Granted bonus reward for quest %s"), *QuestId.ToString());
			}
			break;
		}

		case EQuestEventType::ModifyWorld:
		{
			// Trigger world state change (implement based on your world system)
			UE_LOG(LogTemp, Log, TEXT("Quest event: Modify world for quest %s"), *QuestId.ToString());
			break;
		}

		case EQuestEventType::StartQuest:
		{
			if (Event.TargetQuestId.IsValid())
			{
				StartQuest(Event.TargetQuestId);
				UE_LOG(LogTemp, Log, TEXT("Quest event: Started quest %s"), *Event.TargetQuestId.ToString());
			}
			break;
		}

		case EQuestEventType::Custom:
		{
			// Custom event handling (can be extended)
			UE_LOG(LogTemp, Log, TEXT("Quest event: Custom event for quest %s"), *QuestId.ToString());
			break;
		}

		default:
			break;
	}
}

//~==============================================
//~ Quest Fail Conditions
//~==============================================

void UHarmoniaQuestComponent::CheckFailConditions(float DeltaTime)
{
	// Server only
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}

	TArray<FHarmoniaID> QuestsToFail;

	for (const FActiveQuestProgress& Progress : ActiveQuests)
	{
		FQuestData QuestData;
		if (!GetQuestData(Progress.QuestId, QuestData))
		{
			continue;
		}

		// Check each fail condition
		for (const FQuestFailCondition& Condition : QuestData.FailConditions)
		{
			if (CheckFailCondition(Condition, Progress.QuestId))
			{
				QuestsToFail.AddUnique(Progress.QuestId);
				OnFailConditionTriggered(Progress.QuestId, Condition);
			}
		}
	}

	// Fail quests
	for (const FHarmoniaID& QuestId : QuestsToFail)
	{
		FailQuest(QuestId);
	}
}

bool UHarmoniaQuestComponent::CheckFailCondition(const FQuestFailCondition& Condition, FHarmoniaID QuestId)
{
	switch (Condition.ConditionType)
	{
		case EQuestFailConditionType::TimeLimit:
		{
			const FActiveQuestProgress* Progress = FindActiveQuest(QuestId);
			if (Progress)
			{
				FQuestData QuestData;
				if (GetQuestData(QuestId, QuestData))
				{
					return Progress->IsTimeUp(QuestData);
				}
			}
			return false;
		}

		case EQuestFailConditionType::NPCDied:
		{
			// Check if required NPC is dead (implement based on your NPC system)
			// For now, return false
			return false;
		}

		case EQuestFailConditionType::ItemLost:
		{
			// Check if required item was lost
			UHarmoniaInventoryComponent* Inventory = const_cast<UHarmoniaQuestComponent*>(this)->GetInventoryComponent();
			if (Inventory && Condition.RequiredItemId.IsValid())
			{
				// Check if player has the item
				// For now, return false (would need GetItemCount implementation)
				return false;
			}
			return false;
		}

		case EQuestFailConditionType::LocationLeft:
		{
			// Check if player left required location
			if (Condition.RequiredLocation != FVector::ZeroVector)
			{
				AActor* Owner = GetOwner();
				if (Owner)
				{
					float Distance = FVector::Dist(Owner->GetActorLocation(), Condition.RequiredLocation);
					return Distance > Condition.MaxDistance;
				}
			}
			return false;
		}

		case EQuestFailConditionType::PlayerDied:
		{
			// Check if player died (implement based on your health/death system)
			// This would typically be called from the death event
			return false;
		}

		case EQuestFailConditionType::Custom:
		{
			// Custom fail conditions (can be overridden)
			return false;
		}

		default:
			return false;
	}
}

void UHarmoniaQuestComponent::OnFailConditionTriggered(FHarmoniaID QuestId, const FQuestFailCondition& Condition)
{
	UE_LOG(LogTemp, Warning, TEXT("Fail condition triggered for quest %s: %s"),
		*QuestId.ToString(), *Condition.FailureMessage.ToString());

	// Show notification
	ShowNotification(EQuestNotificationType::QuestFailed, QuestId, Condition.FailureMessage);
}

//~==============================================
//~ Quest Notifications
//~==============================================

void UHarmoniaQuestComponent::ShowNotification(EQuestNotificationType Type, FHarmoniaID QuestId, const FText& Message)
{
	FQuestNotification Notification = CreateNotification(Type, QuestId, Message);
	OnNotification.Broadcast(Notification);
}

FQuestNotification UHarmoniaQuestComponent::CreateNotification(EQuestNotificationType Type, FHarmoniaID QuestId, const FText& Message)
{
	FQuestNotification Notification;
	Notification.NotificationType = Type;
	Notification.QuestId = QuestId;
	Notification.Message = Message;
	Notification.Timestamp = FDateTime::Now();
	Notification.bShown = false;

	return Notification;
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
