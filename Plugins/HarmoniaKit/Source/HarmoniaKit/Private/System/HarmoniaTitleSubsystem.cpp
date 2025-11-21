// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaTitleSubsystem.h"
#include "HarmoniaKit.h"

void UHarmoniaTitleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaTitleSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaTitleSubsystem::UnlockTitle(const FHarmoniaID& TitleId)
{
	FTitleState& State = TitleStates.FindOrAdd(TitleId);
	if (!State.bUnlocked)
	{
		State.TitleId = TitleId;
		State.bUnlocked = true;
		State.UnlockedDate = FDateTime::Now();
		OnTitleUnlocked.Broadcast(TitleId);
	}
}

void UHarmoniaTitleSubsystem::EquipTitle(const FHarmoniaID& TitleId)
{
	if (IsTitleUnlocked(TitleId) || !TitleId.IsValid()) // Allow unequipping by passing invalid ID
	{
		EquippedTitleId = TitleId;
		OnTitleEquipped.Broadcast(TitleId);
	}
}

FHarmoniaID UHarmoniaTitleSubsystem::GetEquippedTitle() const
{
	return EquippedTitleId;
}

bool UHarmoniaTitleSubsystem::IsTitleUnlocked(const FHarmoniaID& TitleId) const
{
	if (const FTitleState* State = TitleStates.Find(TitleId))
	{
		return State->bUnlocked;
	}
	return false;
}
