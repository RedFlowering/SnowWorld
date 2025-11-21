// Copyright 2025 Snow Game Studio.

#include "Journal/HarmoniaJournalSubsystem.h"
#include "HarmoniaStoryLog.h"

void UHarmoniaJournalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaJournalSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaJournalSubsystem::UnlockEntry(FHarmoniaID EntryId)
{
	if (!EntryId.IsValid())
	{
		return;
	}

	if (!UnlockedEntries.Contains(EntryId))
	{
		UnlockedEntries.Add(EntryId);
		OnJournalEntryUnlocked.Broadcast(EntryId);
		UE_LOG(LogHarmoniaStory, Log, TEXT("Journal Entry Unlocked: %s"), *EntryId.ToString());
	}
}

bool UHarmoniaJournalSubsystem::IsEntryUnlocked(FHarmoniaID EntryId) const
{
	return UnlockedEntries.Contains(EntryId);
}

TArray<FHarmoniaID> UHarmoniaJournalSubsystem::GetUnlockedEntries() const
{
	return UnlockedEntries.Array();
}
