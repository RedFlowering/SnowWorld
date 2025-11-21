// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaAchievementSubsystem.h"
#include "HarmoniaKit.h"

void UHarmoniaAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadAchievementData();
}

void UHarmoniaAchievementSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaAchievementSubsystem::UnlockAchievement(const FHarmoniaID& AchievementId)
{
	FAchievementState* State = FindAchievementState(AchievementId);
	if (State)
	{
		if (State->State != EAchievementState::Unlocked)
		{
			State->State = EAchievementState::Unlocked;
			State->UnlockDate = FDateTime::Now();
			
			// TODO: Integrate with Steamworks here
			// if (SteamAPI_Init()) { ... }

			OnAchievementUnlocked.Broadcast(AchievementId);
			UE_LOG(LogHarmoniaKit, Log, TEXT("Achievement Unlocked: %s"), *AchievementId.ToString());
		}
	}
	else
	{
		// Create new state if not exists (though it should exist if loaded from DT)
		FAchievementState NewState;
		NewState.AchievementId = AchievementId;
		NewState.State = EAchievementState::Unlocked;
		NewState.UnlockDate = FDateTime::Now();
		AchievementStates.Add(AchievementId, NewState);

		OnAchievementUnlocked.Broadcast(AchievementId);
	}
}

void UHarmoniaAchievementSubsystem::UpdateProgress(const FHarmoniaID& AchievementId, int32 Amount)
{
	FAchievementState* State = FindAchievementState(AchievementId);
	if (State)
	{
		if (State->State == EAchievementState::Unlocked)
		{
			return; // Already unlocked
		}

		State->CurrentProgress += Amount;
		
		// Check if we reached the target
		FHarmoniaAchievementDefinition Definition;
		if (GetAchievementDefinition(AchievementId, Definition))
		{
			if (Definition.Type == EAchievementType::Progressive)
			{
				if (State->CurrentProgress >= Definition.TargetProgress)
				{
					UnlockAchievement(AchievementId);
				}
				else
				{
					OnAchievementProgressUpdated.Broadcast(AchievementId, State->CurrentProgress);
				}
			}
		}
	}
}

EAchievementState UHarmoniaAchievementSubsystem::GetAchievementState(const FHarmoniaID& AchievementId) const
{
	if (const FAchievementState* State = AchievementStates.Find(AchievementId))
	{
		return State->State;
	}
	return EAchievementState::Locked;
}

int32 UHarmoniaAchievementSubsystem::GetAchievementProgress(const FHarmoniaID& AchievementId) const
{
	if (const FAchievementState* State = AchievementStates.Find(AchievementId))
	{
		return State->CurrentProgress;
	}
	return 0;
}

bool UHarmoniaAchievementSubsystem::GetAchievementDefinition(const FHarmoniaID& AchievementId, FHarmoniaAchievementDefinition& OutDefinition) const
{
	if (AchievementDataTable.IsValid())
	{
		// This is a simplified lookup. In reality, you'd cache this or look up in the DT.
		// Assuming DT row names match IDs or we iterate.
		// For performance, we should cache definitions on load.
		// Implementing simple iteration for now.
		
		UDataTable* DT = AchievementDataTable.Get();
		if (DT)
		{
			// Try to find by RowName first if ID matches RowName
			FName RowName = FName(*AchievementId.ToString());
			FHarmoniaAchievementDefinition* Row = DT->FindRow<FHarmoniaAchievementDefinition>(RowName, TEXT("GetAchievementDefinition"));
			if (Row)
			{
				OutDefinition = *Row;
				return true;
			}
		}
	}
	return false;
}

void UHarmoniaAchievementSubsystem::LoadAchievementData()
{
	// Load initial state from SaveGame or Server
	// For now, just initialize empty states or load from DT defaults
}

FAchievementState* UHarmoniaAchievementSubsystem::FindAchievementState(const FHarmoniaID& AchievementId)
{
	return AchievementStates.Find(AchievementId);
}
