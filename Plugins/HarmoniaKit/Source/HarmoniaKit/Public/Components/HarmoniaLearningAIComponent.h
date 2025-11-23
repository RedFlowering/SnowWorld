// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseAIComponent.h"
#include "HarmoniaLearningAIComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Skill usage tracking entry
 */
USTRUCT(BlueprintType)
struct FHarmoniaSkillUsageEntry
{
	GENERATED_BODY()

	/** Skill name */
	UPROPERTY()
	FName SkillName;

	/** Usage count */
	UPROPERTY()
	int32 UsageCount = 0;
};

/**
 * Player pattern data
 */
USTRUCT(BlueprintType)
struct FHarmoniaPlayerPattern
{
	GENERATED_BODY()

	/** Player reference */
	UPROPERTY()
	AActor* Player = nullptr;

	/** Most common dodge direction (normalized) */
	UPROPERTY()
	FVector PreferredDodgeDirection = FVector::ZeroVector;

	/** Average attack timing (seconds) */
	UPROPERTY()
	float AverageAttackInterval = 2.0f;

	/** Most frequently used abilities/skills */
	UPROPERTY()
	TArray<FHarmoniaSkillUsageEntry> SkillUsageData;

	/** Combat encounters count */
	UPROPERTY()
	int32 EncounterCount = 0;

	/** Player deaths count */
	UPROPERTY()
	int32 PlayerDeathCount = 0;

	/** Monster deaths count */
	UPROPERTY()
	int32 MonsterDeathCount = 0;

	/** Average combat duration */
	UPROPERTY()
	float AverageCombatDuration = 30.0f;

	/** Pattern confidence (0.0 - 1.0) */
	UPROPERTY()
	float Confidence = 0.0f;
};

/**
 * Player pattern entry for replication
 */
USTRUCT(BlueprintType)
struct FHarmoniaPlayerPatternEntry
{
	GENERATED_BODY()

	/** Player actor */
	UPROPERTY()
	AActor* Player = nullptr;

	/** Pattern data */
	UPROPERTY()
	FHarmoniaPlayerPattern Pattern;
};

/**
 * Adaptive difficulty state
 */
UENUM(BlueprintType)
enum class EHarmoniaAdaptiveDifficulty : uint8
{
	VeryEasy		UMETA(DisplayName = "Very Easy"),
	Easy			UMETA(DisplayName = "Easy"),
	Normal			UMETA(DisplayName = "Normal"),
	Hard			UMETA(DisplayName = "Hard"),
	VeryHard		UMETA(DisplayName = "Very Hard")
};

/**
 * Learning AI Component
 * Learns player patterns and adapts difficulty accordingly
 * Inherits common AI functionality from UHarmoniaBaseAIComponent
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaLearningAIComponent : public UHarmoniaBaseAIComponent
{
	GENERATED_BODY()

public:
	UHarmoniaLearningAIComponent();

protected:
	//~UHarmoniaBaseAIComponent interface
	virtual void InitializeAIComponent() override;
	virtual void CleanupAIComponent() override;
	virtual void UpdateAIComponent(float DeltaTime) override;
	//~End of UHarmoniaBaseAIComponent interface

public:
	//~UActorComponent interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UActorComponent interface

	// ==================== Configuration ====================

	/** Enable learning AI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning AI")
	bool bEnableLearning = true;

	/** How many samples needed before patterns are reliable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning AI")
	int32 MinSamplesForConfidence = 5;

	/** Pattern detection radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning AI")
	float DetectionRadius = 300.0f;

	/** How often to analyze patterns (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Learning AI")
	float AnalysisInterval = 2.0f;

	/** Enable adaptive difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Difficulty")
	bool bEnableAdaptiveDifficulty = true;

	/** Adjust difficulty after this many encounters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Difficulty")
	int32 EncountersBeforeAdjustment = 3;

	/** Difficulty adjustment rate (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Difficulty")
	float DifficultyAdjustmentRate = 0.1f;

	// ==================== Public API ====================

	/** Get pattern data for specific player */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	FHarmoniaPlayerPattern GetPlayerPattern(AActor* Player) const;

	/** Get current difficulty level */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	EHarmoniaAdaptiveDifficulty GetCurrentDifficulty() const { return CurrentDifficulty; }

	/** Record player dodge direction */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	void RecordPlayerDodge(AActor* Player, FVector DodgeDirection);

	/** Record player attack */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	void RecordPlayerAttack(AActor* Player, FName AbilityName);

	/** Record combat result */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	void RecordCombatResult(AActor* Player, bool bPlayerWon, float CombatDuration);

	/** Predict next player action based on learned patterns */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	FVector PredictPlayerDodgeDirection(AActor* Player) const;

	/** Get recommended attack timing based on player patterns */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	float GetOptimalAttackTiming(AActor* Player) const;

	/** Adjust difficulty based on player performance */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	void AdjustDifficulty();

	/** Get difficulty multiplier for stats */
	UFUNCTION(BlueprintCallable, Category = "Learning AI")
	float GetDifficultyMultiplier() const;

protected:
	// ==================== Internal Functions ====================

	/** Analyze player movement patterns */
	void AnalyzePlayerMovement(float DeltaTime);

	/** Update pattern confidence */
	void UpdatePatternConfidence(AActor* Player);

	/** Apply difficulty adjustments */
	void ApplyDifficultyAdjustments();

	/** Calculate win rate for player */
	float CalculatePlayerWinRate(AActor* Player) const;

	// ==================== State ====================

	/** Learned patterns per player */
	UPROPERTY(Replicated)
	TArray<FHarmoniaPlayerPatternEntry> LearnedPatterns;

	/** Current adaptive difficulty */
	UPROPERTY(Replicated)
	EHarmoniaAdaptiveDifficulty CurrentDifficulty = EHarmoniaAdaptiveDifficulty::Normal;

	/** Analysis timer */
	float AnalysisTimer = 0.0f;

	/** Last player position (for movement analysis) */
	TMap<AActor*, FVector> LastPlayerPositions;

	/** Last attack time per player */
	TMap<AActor*, float> LastAttackTimes;

	/** Encounter start time */
	float EncounterStartTime = 0.0f;

	/** Is currently in combat */
	bool bInCombat = false;

	/** Current combat target */
	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	/** Cached owner monster */
	UPROPERTY()
	AHarmoniaMonsterBase* OwnerMonster = nullptr;
};
