// Copyright Epic Games, Inc. All Rights Reserved.

#include "SaveGame/HarmoniaLearningAISaveGame.h"

UHarmoniaLearningAISaveGame::UHarmoniaLearningAISaveGame()
{
	SlotName = TEXT("LearningAIData");
	UserIndex = 0;
	SaveVersion = 1;
}

void UHarmoniaLearningAISaveGame::AddOrUpdatePattern(const FString& PlayerID, const FHarmoniaPlayerPattern& Pattern, const FString& PlayerName)
{
	// Find existing pattern
	FHarmoniaSerializedPlayerPattern* ExistingPattern = LearnedPatterns.FindByPredicate([&PlayerID](const FHarmoniaSerializedPlayerPattern& P)
	{
		return P.PlayerID == PlayerID;
	});

	if (ExistingPattern)
	{
		// Update existing
		ExistingPattern->PreferredDodgeDirection = Pattern.PreferredDodgeDirection;
		ExistingPattern->AverageAttackInterval = Pattern.AverageAttackInterval;

		// Convert skill usage array to map
		ExistingPattern->SkillUsageCount.Empty();
		for (const FHarmoniaSkillUsageEntry& Entry : Pattern.SkillUsageData)
		{
			ExistingPattern->SkillUsageCount.Add(Entry.SkillName.ToString(), Entry.UsageCount);
		}

		ExistingPattern->EncounterCount = Pattern.EncounterCount;
		ExistingPattern->PlayerDeathCount = Pattern.PlayerDeathCount;
		ExistingPattern->MonsterDeathCount = Pattern.MonsterDeathCount;
		ExistingPattern->AverageCombatDuration = Pattern.AverageCombatDuration;
		ExistingPattern->Confidence = Pattern.Confidence;
		ExistingPattern->LastUpdated = FDateTime::Now();
	}
	else
	{
		// Create new
		FHarmoniaSerializedPlayerPattern NewPattern;
		NewPattern.PlayerID = PlayerID;
		NewPattern.PlayerName = PlayerName;
		NewPattern.PreferredDodgeDirection = Pattern.PreferredDodgeDirection;
		NewPattern.AverageAttackInterval = Pattern.AverageAttackInterval;

		// Convert skill usage array to map
		for (const FHarmoniaSkillUsageEntry& Entry : Pattern.SkillUsageData)
		{
			NewPattern.SkillUsageCount.Add(Entry.SkillName.ToString(), Entry.UsageCount);
		}

		NewPattern.EncounterCount = Pattern.EncounterCount;
		NewPattern.PlayerDeathCount = Pattern.PlayerDeathCount;
		NewPattern.MonsterDeathCount = Pattern.MonsterDeathCount;
		NewPattern.AverageCombatDuration = Pattern.AverageCombatDuration;
		NewPattern.Confidence = Pattern.Confidence;
		NewPattern.LastUpdated = FDateTime::Now();

		LearnedPatterns.Add(NewPattern);
	}
}

FHarmoniaSerializedPlayerPattern UHarmoniaLearningAISaveGame::GetPattern(const FString& PlayerID) const
{
	const FHarmoniaSerializedPlayerPattern* Found = LearnedPatterns.FindByPredicate([&PlayerID](const FHarmoniaSerializedPlayerPattern& P)
	{
		return P.PlayerID == PlayerID;
	});

	if (Found)
	{
		return *Found;
	}

	return FHarmoniaSerializedPlayerPattern();
}

void UHarmoniaLearningAISaveGame::CleanupOldPatterns(int32 DaysOld)
{
	FDateTime Cutoff = FDateTime::Now() - FTimespan::FromDays(DaysOld);

	LearnedPatterns.RemoveAll([&Cutoff](const FHarmoniaSerializedPlayerPattern& P)
	{
		return P.LastUpdated < Cutoff;
	});

	UE_LOG(LogTemp, Log, TEXT("Cleaned up old learning patterns. Remaining: %d"), LearnedPatterns.Num());
}

TArray<FHarmoniaSerializedPlayerPattern> UHarmoniaLearningAISaveGame::GetTopPatterns(int32 Count) const
{
	TArray<FHarmoniaSerializedPlayerPattern> Sorted = LearnedPatterns;

	// Sort by encounter count descending
	Sorted.Sort([](const FHarmoniaSerializedPlayerPattern& A, const FHarmoniaSerializedPlayerPattern& B)
	{
		return A.EncounterCount > B.EncounterCount;
	});

	if (Sorted.Num() > Count)
	{
		Sorted.SetNum(Count);
	}

	return Sorted;
}
