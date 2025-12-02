// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaQuestSubsystem.h"
#include "Engine/DataTable.h"

UHarmoniaQuestSubsystem::UHarmoniaQuestSubsystem()
{
	QuestDataTable = nullptr;
	DailyResetHour = 0; // Reset at midnight
	WeeklyResetDay = 0; // Reset on Sunday
	WeeklyResetHour = 0; // Reset at midnight
}

void UHarmoniaQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize reset times
	LastDailyResetTime = FDateTime::Now();
	LastWeeklyResetTime = FDateTime::Now();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaQuestSubsystem initialized"));
}

void UHarmoniaQuestSubsystem::Deinitialize()
{
	ClearQuestDataCache();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaQuestSubsystem deinitialized"));
}

//~==============================================
//~ Quest Data Access
//~==============================================

void UHarmoniaQuestSubsystem::SetQuestDataTable(UDataTable* DataTable)
{
	if (DataTable != QuestDataTable)
	{
		QuestDataTable = DataTable;
		CacheQuestData();
	}
}

bool UHarmoniaQuestSubsystem::GetQuestData(FHarmoniaID QuestId, FHarmoniaQuestData& OutQuestData) const
{
	if (!QuestId.IsValid())
	{
		return false;
	}

	// Try cache first
	if (const FHarmoniaQuestData* CachedData = QuestDataCache.Find(QuestId))
	{
		OutQuestData = *CachedData;
		return true;
	}

	// Try data table
	if (QuestDataTable)
	{
		FHarmoniaQuestData* QuestData = QuestDataTable->FindRow<FHarmoniaQuestData>(QuestId.Id, TEXT("GetQuestData"));
		if (QuestData)
		{
			OutQuestData = *QuestData;
			return true;
		}
	}

	return false;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::GetQuestsByType(EQuestType QuestType) const
{
	TArray<FHarmoniaQuestData> Result;

	for (const auto& Pair : QuestDataCache)
	{
		if (Pair.Value.QuestType == QuestType)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::GetQuestsByCategory(FGameplayTag CategoryTag) const
{
	TArray<FHarmoniaQuestData> Result;

	if (!CategoryTag.IsValid())
	{
		return Result;
	}

	for (const auto& Pair : QuestDataCache)
	{
		if (Pair.Value.CategoryTags.HasTag(CategoryTag))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::GetAllQuests() const
{
	TArray<FHarmoniaQuestData> Result;
	QuestDataCache.GenerateValueArray(Result);
	return Result;
}

bool UHarmoniaQuestSubsystem::DoesQuestExist(FHarmoniaID QuestId) const
{
	if (!QuestId.IsValid())
	{
		return false;
	}

	return QuestDataCache.Contains(QuestId);
}

//~==============================================
//~ Quest Chain Validation
//~==============================================

bool UHarmoniaQuestSubsystem::GetPrerequisiteQuests(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutPrerequisites) const
{
	OutPrerequisites.Empty();

	FHarmoniaQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return false;
	}

	// Extract prerequisite quests from unlock conditions
	for (const FQuestCondition& Condition : QuestData.UnlockConditions)
	{
		if (Condition.ConditionType == EQuestConditionType::QuestCompleted &&
			Condition.TargetQuestId.IsValid())
		{
			OutPrerequisites.AddUnique(Condition.TargetQuestId);
		}
	}

	return OutPrerequisites.Num() > 0;
}

TArray<FHarmoniaID> UHarmoniaQuestSubsystem::GetUnlockedQuests(FHarmoniaID QuestId) const
{
	TArray<FHarmoniaID> Result;

	FHarmoniaQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return Result;
	}

	// Get directly linked next quests
	Result = QuestData.NextQuests;

	// Also find quests that have this quest as a prerequisite
	for (const auto& Pair : QuestDataCache)
	{
		TArray<FHarmoniaID> Prerequisites;
		if (GetPrerequisiteQuests(Pair.Key, Prerequisites))
		{
			if (Prerequisites.Contains(QuestId))
			{
				Result.AddUnique(Pair.Key);
			}
		}
	}

	return Result;
}

void UHarmoniaQuestSubsystem::GetQuestChain(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutQuestChain, bool bIncludeStartQuest) const
{
	OutQuestChain.Empty();

	if (!QuestId.IsValid())
	{
		return;
	}

	TSet<FHarmoniaID> VisitedQuests;

	if (bIncludeStartQuest)
	{
		OutQuestChain.Add(QuestId);
		VisitedQuests.Add(QuestId);
	}

	GetQuestChainRecursive(QuestId, OutQuestChain, VisitedQuests);
}

bool UHarmoniaQuestSubsystem::ValidateQuestChain(FHarmoniaID QuestId) const
{
	TSet<FHarmoniaID> VisitedQuests;
	TSet<FHarmoniaID> RecursionStack;
	return ValidateQuestChainRecursive(QuestId, VisitedQuests, RecursionStack);
}

//~==============================================
//~ Daily/Weekly Quest Reset
//~==============================================

float UHarmoniaQuestSubsystem::GetTimeUntilDailyReset() const
{
	FDateTime NextResetTime = GetNextDailyResetTime();
	FTimespan TimeUntilReset = NextResetTime - FDateTime::Now();
	return static_cast<float>(TimeUntilReset.GetTotalSeconds());
}

float UHarmoniaQuestSubsystem::GetTimeUntilWeeklyReset() const
{
	FDateTime NextResetTime = GetNextWeeklyResetTime();
	FTimespan TimeUntilReset = NextResetTime - FDateTime::Now();
	return static_cast<float>(TimeUntilReset.GetTotalSeconds());
}

bool UHarmoniaQuestSubsystem::ShouldResetDailyQuests() const
{
	FDateTime Now = FDateTime::Now();
	FDateTime NextResetTime = GetNextDailyResetTime();

	// Check if we've passed the reset time since last reset
	return Now >= NextResetTime && LastDailyResetTime < NextResetTime;
}

bool UHarmoniaQuestSubsystem::ShouldResetWeeklyQuests() const
{
	FDateTime Now = FDateTime::Now();
	FDateTime NextResetTime = GetNextWeeklyResetTime();

	// Check if we've passed the reset time since last reset
	return Now >= NextResetTime && LastWeeklyResetTime < NextResetTime;
}

void UHarmoniaQuestSubsystem::MarkDailyResetCompleted()
{
	LastDailyResetTime = FDateTime::Now();
	UE_LOG(LogTemp, Log, TEXT("Daily quest reset completed"));
}

void UHarmoniaQuestSubsystem::MarkWeeklyResetCompleted()
{
	LastWeeklyResetTime = FDateTime::Now();
	UE_LOG(LogTemp, Log, TEXT("Weekly quest reset completed"));
}

//~==============================================
//~ Quest Statistics
//~==============================================

int32 UHarmoniaQuestSubsystem::GetTotalQuestCount() const
{
	return QuestDataCache.Num();
}

int32 UHarmoniaQuestSubsystem::GetQuestCountByType(EQuestType QuestType) const
{
	int32 Count = 0;

	for (const auto& Pair : QuestDataCache)
	{
		if (Pair.Value.QuestType == QuestType)
		{
			Count++;
		}
	}

	return Count;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::GetRecommendedQuestsForLevel(int32 PlayerLevel, int32 LevelRange) const
{
	TArray<FHarmoniaQuestData> Result;

	int32 MinLevel = FMath::Max(1, PlayerLevel - LevelRange);
	int32 MaxLevel = PlayerLevel + LevelRange;

	for (const auto& Pair : QuestDataCache)
	{
		if (Pair.Value.RecommendedLevel >= MinLevel && Pair.Value.RecommendedLevel <= MaxLevel)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

//~==============================================
//~ Quest Search and Filtering
//~==============================================

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::SearchQuestsByName(const FString& SearchText) const
{
	TArray<FHarmoniaQuestData> Result;

	if (SearchText.IsEmpty())
	{
		return Result;
	}

	FString LowerSearchText = SearchText.ToLower();

	for (const auto& Pair : QuestDataCache)
	{
		FString QuestName = Pair.Value.QuestName.ToString().ToLower();
		if (QuestName.Contains(LowerSearchText))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::FindQuestsWithObjectiveType(EQuestObjectiveType ObjectiveType) const
{
	TArray<FHarmoniaQuestData> Result;

	for (const auto& Pair : QuestDataCache)
	{
		for (const FQuestObjective& Objective : Pair.Value.Objectives)
		{
			if (Objective.ObjectiveType == ObjectiveType)
			{
				Result.Add(Pair.Value);
				break;
			}
		}
	}

	return Result;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::FindQuestsRewardingItem(FHarmoniaID ItemId) const
{
	TArray<FHarmoniaQuestData> Result;

	if (!ItemId.IsValid())
	{
		return Result;
	}

	for (const auto& Pair : QuestDataCache)
	{
		// Check rewards
		for (const FQuestReward& Reward : Pair.Value.Rewards)
		{
			if (Reward.RewardType == EQuestRewardType::Item && Reward.ItemId == ItemId)
			{
				Result.Add(Pair.Value);
				break;
			}
		}

		// Check optional rewards
		for (const FQuestReward& Reward : Pair.Value.OptionalRewards)
		{
			if (Reward.RewardType == EQuestRewardType::Item && Reward.ItemId == ItemId)
			{
				Result.Add(Pair.Value);
				break;
			}
		}
	}

	return Result;
}

TArray<FHarmoniaQuestData> UHarmoniaQuestSubsystem::FindQuestsRequiringItem(FHarmoniaID ItemId) const
{
	TArray<FHarmoniaQuestData> Result;

	if (!ItemId.IsValid())
	{
		return Result;
	}

	for (const auto& Pair : QuestDataCache)
	{
		// Check objectives
		for (const FQuestObjective& Objective : Pair.Value.Objectives)
		{
			if ((Objective.ObjectiveType == EQuestObjectiveType::Collect ||
				 Objective.ObjectiveType == EQuestObjectiveType::Deliver ||
				 Objective.ObjectiveType == EQuestObjectiveType::Use) &&
				Objective.TargetId == ItemId)
			{
				Result.Add(Pair.Value);
				break;
			}
		}

		// Check unlock conditions
		for (const FQuestCondition& Condition : Pair.Value.UnlockConditions)
		{
			if (Condition.ConditionType == EQuestConditionType::HasItem &&
				Condition.RequiredItemId == ItemId)
			{
				Result.Add(Pair.Value);
				break;
			}
		}
	}

	return Result;
}

//~==============================================
//~ Debug and Development
//~==============================================

#if WITH_EDITOR
bool UHarmoniaQuestSubsystem::ValidateAllQuests(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();
	bool bAllValid = true;

	for (const auto& Pair : QuestDataCache)
	{
		const FHarmoniaID& QuestId = Pair.Key;
		const FHarmoniaQuestData& QuestData = Pair.Value;

		// Check for circular dependencies
		if (!ValidateQuestChain(QuestId))
		{
			OutErrors.Add(FString::Printf(TEXT("Quest '%s' has circular dependency in quest chain"),
				*QuestId.ToString()));
			bAllValid = false;
		}

		// Validate prerequisite quests exist
		TArray<FHarmoniaID> Prerequisites;
		if (GetPrerequisiteQuests(QuestId, Prerequisites))
		{
			for (const FHarmoniaID& PrereqId : Prerequisites)
			{
				if (!DoesQuestExist(PrereqId))
				{
					OutErrors.Add(FString::Printf(TEXT("Quest '%s' has invalid prerequisite '%s'"),
						*QuestId.ToString(), *PrereqId.ToString()));
					bAllValid = false;
				}
			}
		}

		// Validate next quests exist
		for (const FHarmoniaID& NextQuestId : QuestData.NextQuests)
		{
			if (!DoesQuestExist(NextQuestId))
			{
				OutErrors.Add(FString::Printf(TEXT("Quest '%s' has invalid next quest '%s'"),
					*QuestId.ToString(), *NextQuestId.ToString()));
				bAllValid = false;
			}
		}

		// Validate quest has at least one objective
		if (QuestData.Objectives.Num() == 0)
		{
			OutErrors.Add(FString::Printf(TEXT("Quest '%s' has no objectives"),
				*QuestId.ToString()));
			bAllValid = false;
		}

		// Validate optional reward selection count
		if (QuestData.MaxOptionalRewardChoices > QuestData.OptionalRewards.Num())
		{
			OutErrors.Add(FString::Printf(TEXT("Quest '%s' has MaxOptionalRewardChoices (%d) greater than OptionalRewards count (%d)"),
				*QuestId.ToString(), QuestData.MaxOptionalRewardChoices, QuestData.OptionalRewards.Num()));
			bAllValid = false;
		}
	}

	return bAllValid;
}

void UHarmoniaQuestSubsystem::PrintQuestDependencyGraph() const
{
	UE_LOG(LogTemp, Log, TEXT("=== Quest Dependency Graph ==="));

	for (const auto& Pair : QuestDataCache)
	{
		const FHarmoniaID& QuestId = Pair.Key;
		const FHarmoniaQuestData& QuestData = Pair.Value;

		UE_LOG(LogTemp, Log, TEXT("Quest: %s (%s)"), *QuestData.QuestName.ToString(), *QuestId.ToString());

		// Print prerequisites
		TArray<FHarmoniaID> Prerequisites;
		if (GetPrerequisiteQuests(QuestId, Prerequisites))
		{
			UE_LOG(LogTemp, Log, TEXT("  Prerequisites:"));
			for (const FHarmoniaID& PrereqId : Prerequisites)
			{
				FHarmoniaQuestData PrereqData;
				if (GetQuestData(PrereqId, PrereqData))
				{
					UE_LOG(LogTemp, Log, TEXT("    - %s (%s)"), *PrereqData.QuestName.ToString(), *PrereqId.ToString());
				}
			}
		}

		// Print next quests
		if (QuestData.NextQuests.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("  Next Quests:"));
			for (const FHarmoniaID& NextQuestId : QuestData.NextQuests)
			{
				FHarmoniaQuestData NextQuestData;
				if (GetQuestData(NextQuestId, NextQuestData))
				{
					UE_LOG(LogTemp, Log, TEXT("    - %s (%s)"), *NextQuestData.QuestName.ToString(), *NextQuestId.ToString());
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("=== End Quest Dependency Graph ==="));
}

FString UHarmoniaQuestSubsystem::GetQuestStatisticsReport() const
{
	FString Report = TEXT("=== Quest Statistics ===\n\n");

	Report += FString::Printf(TEXT("Total Quests: %d\n\n"), GetTotalQuestCount());

	// Quest type breakdown
	Report += TEXT("Quest Types:\n");
	for (int32 i = 0; i < (int32)EQuestType::MAX; ++i)
	{
		EQuestType Type = (EQuestType)i;
		int32 Count = GetQuestCountByType(Type);
		if (Count > 0)
		{
			FString TypeName = UEnum::GetDisplayValueAsText(Type).ToString();
			Report += FString::Printf(TEXT("  %s: %d\n"), *TypeName, Count);
		}
	}

	Report += TEXT("\n");

	// Objective type breakdown
	Report += TEXT("Objective Types:\n");
	TMap<EQuestObjectiveType, int32> ObjectiveTypeCounts;
	for (const auto& Pair : QuestDataCache)
	{
		for (const FQuestObjective& Objective : Pair.Value.Objectives)
		{
			int32& Count = ObjectiveTypeCounts.FindOrAdd(Objective.ObjectiveType, 0);
			Count++;
		}
	}
	for (const auto& TypeCount : ObjectiveTypeCounts)
	{
		FString TypeName = UEnum::GetDisplayValueAsText(TypeCount.Key).ToString();
		Report += FString::Printf(TEXT("  %s: %d\n"), *TypeName, TypeCount.Value);
	}

	Report += TEXT("\n=== End Quest Statistics ===");

	return Report;
}
#endif

//~==============================================
//~ Internal Functions
//~==============================================

void UHarmoniaQuestSubsystem::CacheQuestData()
{
	ClearQuestDataCache();

	if (!QuestDataTable)
	{
		return;
	}

	// Get all quest rows
	TArray<FHarmoniaQuestData*> AllQuests;
	QuestDataTable->GetAllRows<FHarmoniaQuestData>(TEXT("CacheQuestData"), AllQuests);

	// Cache each quest
	for (FHarmoniaQuestData* QuestData : AllQuests)
	{
		if (QuestData && QuestData->QuestId.IsValid())
		{
			QuestDataCache.Add(QuestData->QuestId, *QuestData);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Cached %d quests"), QuestDataCache.Num());
}

void UHarmoniaQuestSubsystem::ClearQuestDataCache()
{
	QuestDataCache.Empty();
}

FDateTime UHarmoniaQuestSubsystem::GetNextDailyResetTime() const
{
	FDateTime Now = FDateTime::Now();
	FDateTime NextReset(Now.GetYear(), Now.GetMonth(), Now.GetDay(), DailyResetHour, 0, 0);

	// If reset time has already passed today, schedule for tomorrow
	if (Now >= NextReset)
	{
		NextReset += FTimespan::FromDays(1);
	}

	return NextReset;
}

FDateTime UHarmoniaQuestSubsystem::GetNextWeeklyResetTime() const
{
	FDateTime Now = FDateTime::Now();

	// Get current day of week (0=Sunday)
	int32 CurrentDayOfWeek = (int32)Now.GetDayOfWeek();

	// Calculate days until next reset day
	int32 DaysUntilReset = (WeeklyResetDay - CurrentDayOfWeek + 7) % 7;

	// If reset day is today, check if reset time has passed
	if (DaysUntilReset == 0)
	{
		FDateTime TodayResetTime(Now.GetYear(), Now.GetMonth(), Now.GetDay(), WeeklyResetHour, 0, 0);
		if (Now >= TodayResetTime)
		{
			// Reset time has passed, schedule for next week
			DaysUntilReset = 7;
		}
	}

	FDateTime NextReset = Now + FTimespan::FromDays(DaysUntilReset);
	NextReset = FDateTime(NextReset.GetYear(), NextReset.GetMonth(), NextReset.GetDay(), WeeklyResetHour, 0, 0);

	return NextReset;
}

void UHarmoniaQuestSubsystem::GetQuestChainRecursive(FHarmoniaID QuestId, TArray<FHarmoniaID>& OutQuestChain, TSet<FHarmoniaID>& VisitedQuests) const
{
	FHarmoniaQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		return;
	}

	// Add next quests
	for (const FHarmoniaID& NextQuestId : QuestData.NextQuests)
	{
		if (!VisitedQuests.Contains(NextQuestId))
		{
			OutQuestChain.Add(NextQuestId);
			VisitedQuests.Add(NextQuestId);
			GetQuestChainRecursive(NextQuestId, OutQuestChain, VisitedQuests);
		}
	}
}

bool UHarmoniaQuestSubsystem::ValidateQuestChainRecursive(FHarmoniaID QuestId, TSet<FHarmoniaID>& VisitedQuests, TSet<FHarmoniaID>& RecursionStack) const
{
	if (!QuestId.IsValid())
	{
		return true;
	}

	// If we're already in the recursion stack, we have a circular dependency
	if (RecursionStack.Contains(QuestId))
	{
		return false;
	}

	// If we've already validated this quest, skip it
	if (VisitedQuests.Contains(QuestId))
	{
		return true;
	}

	// Add to recursion stack
	RecursionStack.Add(QuestId);
	VisitedQuests.Add(QuestId);

	// Get quest data
	FHarmoniaQuestData QuestData;
	if (!GetQuestData(QuestId, QuestData))
	{
		RecursionStack.Remove(QuestId);
		return true; // Quest doesn't exist, not a validation error
	}

	// Validate next quests
	for (const FHarmoniaID& NextQuestId : QuestData.NextQuests)
	{
		if (!ValidateQuestChainRecursive(NextQuestId, VisitedQuests, RecursionStack))
		{
			return false;
		}
	}

	// Remove from recursion stack
	RecursionStack.Remove(QuestId);

	return true;
}
