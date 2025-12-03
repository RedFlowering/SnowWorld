// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Components/HarmoniaLearningAIComponent.h"
#include "HarmoniaLearningAISaveGame.generated.h"

/**
 * Serializable player pattern data
 */
USTRUCT(BlueprintType)
struct FHarmoniaSerializedPlayerPattern
{
	GENERATED_BODY()

	/** Player unique ID */
	UPROPERTY(SaveGame)
	FString PlayerID;

	/** Player name for display */
	UPROPERTY(SaveGame)
	FString PlayerName;

	/** Preferred dodge direction */
	UPROPERTY(SaveGame)
	FVector PreferredDodgeDirection = FVector::ZeroVector;

	/** Average attack interval */
	UPROPERTY(SaveGame)
	float AverageAttackInterval = 2.0f;

	/** Skill usage counts */
	UPROPERTY(SaveGame)
	TMap<FString, int32> SkillUsageCount;

	/** Encounter count */
	UPROPERTY(SaveGame)
	int32 EncounterCount = 0;

	/** Player deaths */
	UPROPERTY(SaveGame)
	int32 PlayerDeathCount = 0;

	/** Monster deaths */
	UPROPERTY(SaveGame)
	int32 MonsterDeathCount = 0;

	/** Average combat duration */
	UPROPERTY(SaveGame)
	float AverageCombatDuration = 30.0f;

	/** Pattern confidence */
	UPROPERTY(SaveGame)
	float Confidence = 0.0f;

	/** Last updated timestamp */
	UPROPERTY(SaveGame)
	FDateTime LastUpdated;
};

/**
 * SaveGame for Learning AI data
 * Persists player pattern data across sessions
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaLearningAISaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHarmoniaLearningAISaveGame();

	/** Slot name for saving */
	UPROPERTY(VisibleAnywhere, Category = "Save")
	FString SlotName;

	/** User index for saving */
	UPROPERTY(VisibleAnywhere, Category = "Save")
	int32 UserIndex;

	/** All learned patterns */
	UPROPERTY(SaveGame)
	TArray<FHarmoniaSerializedPlayerPattern> LearnedPatterns;

	/** Save version for migration */
	UPROPERTY(SaveGame)
	int32 SaveVersion;

	/** Add or update pattern */
	void AddOrUpdatePattern(const FString& PlayerID, const FHarmoniaPlayerPattern& Pattern, const FString& PlayerName);

	/** Get pattern by player ID */
	FHarmoniaSerializedPlayerPattern GetPattern(const FString& PlayerID) const;

	/** Remove old patterns (older than specified days) */
	void CleanupOldPatterns(int32 DaysOld = 30);

	/** Get all patterns sorted by encounter count */
	TArray<FHarmoniaSerializedPlayerPattern> GetTopPatterns(int32 Count = 10) const;
};
