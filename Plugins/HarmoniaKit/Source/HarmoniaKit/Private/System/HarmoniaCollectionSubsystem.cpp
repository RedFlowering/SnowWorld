// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaCollectionSubsystem.h"
#include "HarmoniaKit.h"

void UHarmoniaCollectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaCollectionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaCollectionSubsystem::MarkAsSeen(const FHarmoniaID& CollectionId)
{
	FCollectionState& State = CollectionStates.FindOrAdd(CollectionId);
	if (State.State == ECollectionState::Unknown)
	{
		State.CollectionId = CollectionId;
		State.State = ECollectionState::Seen;
		OnCollectionUpdated.Broadcast(CollectionId);
	}
}

void UHarmoniaCollectionSubsystem::MarkAsCollected(const FHarmoniaID& CollectionId)
{
	FCollectionState& State = CollectionStates.FindOrAdd(CollectionId);
	
	bool bChanged = false;
	if (State.State != ECollectionState::Collected)
	{
		State.CollectionId = CollectionId;
		State.State = ECollectionState::Collected;
		State.FirstCollectedDate = FDateTime::Now();
		bChanged = true;
	}
	
	State.Count++;
	
	if (bChanged || State.Count > 1)
	{
		OnCollectionUpdated.Broadcast(CollectionId);
	}
}

ECollectionState UHarmoniaCollectionSubsystem::GetCollectionState(const FHarmoniaID& CollectionId) const
{
	if (const FCollectionState* State = CollectionStates.Find(CollectionId))
	{
		return State->State;
	}
	return ECollectionState::Unknown;
}

int32 UHarmoniaCollectionSubsystem::GetCollectionCount(const FHarmoniaID& CollectionId) const
{
	if (const FCollectionState* State = CollectionStates.Find(CollectionId))
	{
		return State->Count;
	}
	return 0;
}
