// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "HarmoniaTerritoryDisputeComponent.generated.h"

class AHarmoniaMonsterBase;

/**
 * Combat advantage evaluation result
 */
UENUM(BlueprintType)
enum class EHarmoniaCombatAdvantage : uint8
{
	/** Significant disadvantage - should flee immediately */
	SignificantDisadvantage		UMETA(DisplayName = "Significant Disadvantage"),

	/** Slight disadvantage - consider fleeing or fighting defensively */
	SlightDisadvantage			UMETA(DisplayName = "Slight Disadvantage"),

	/** Even match */
	Even						UMETA(DisplayName = "Even"),

	/** Slight advantage - can fight or pursue */
	SlightAdvantage				UMETA(DisplayName = "Slight Advantage"),

	/** Significant advantage - pursue and eliminate */
	SignificantAdvantage		UMETA(DisplayName = "Significant Advantage")
};

/**
 * Territory dispute state
 */
UENUM(BlueprintType)
enum class EHarmoniaTerritoryDisputeState : uint8
{
	/** No dispute active */
	None						UMETA(DisplayName = "None"),

	/** Challenging another monster/pack for territory */
	Challenging					UMETA(DisplayName = "Challenging"),

	/** Defending territory from challenger */
	Defending					UMETA(DisplayName = "Defending"),

	/** Fleeing from stronger opponent */
	Fleeing						UMETA(DisplayName = "Fleeing"),

	/** Pursuing fleeing opponent */
	Pursuing					UMETA(DisplayName = "Pursuing"),

	/** Player interrupted, preparing to flee */
	PlayerInterrupted			UMETA(DisplayName = "Player Interrupted")
};

/**
 * Opponent info for combat evaluation
 */
USTRUCT(BlueprintType)
struct FHarmoniaOpponentInfo
{
	GENERATED_BODY()

	/** The opponent actor */
	UPROPERTY(BlueprintReadWrite, Category = "Opponent")
	AActor* OpponentActor = nullptr;

	/** Monster opponent (if applicable) */
	UPROPERTY(BlueprintReadWrite, Category = "Opponent")
	AHarmoniaMonsterBase* OpponentMonster = nullptr;

	/** Is opponent a player */
	UPROPERTY(BlueprintReadWrite, Category = "Opponent")
	bool bIsPlayer = false;

	/** Opponent faction */
	UPROPERTY(BlueprintReadWrite, Category = "Opponent")
	EHarmoniaMonsterFaction OpponentFaction = EHarmoniaMonsterFaction::Neutral;

	/** Estimated combat power */
	UPROPERTY(BlueprintReadWrite, Category = "Opponent")
	float CombatPower = 0.0f;

	/** Number in opponent's group */
	UPROPERTY(BlueprintReadWrite, Category = "Opponent")
	int32 GroupSize = 1;
};

/**
 * Delegates for territory dispute events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryDisputeStarted, AHarmoniaMonsterBase*, Challenger, AHarmoniaMonsterBase*, Defender);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerritoryDisputeEnded, AHarmoniaMonsterBase*, Winner, AHarmoniaMonsterBase*, Loser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFleeingFromOpponent, AActor*, Opponent, EHarmoniaCombatAdvantage, Advantage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuingOpponent, AActor*, Opponent, EHarmoniaCombatAdvantage, Advantage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInterruptedDispute, AActor*, Player);

/**
 * Component for managing territory disputes between monster packs
 * Handles inter-faction combat, fleeing/pursuing behavior, and player interruption
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaTerritoryDisputeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaTerritoryDisputeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== Configuration ====================

	/** Enable territory dispute system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	bool bEnableTerritoryDispute = true;

	/** Detection radius for potential opponents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	float DetectionRadius = 2000.0f;

	/** How often to scan for opponents (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	float ScanInterval = 2.0f;

	/** Threshold for significant advantage (combat power ratio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float SignificantAdvantageThreshold = 2.0f;

	/** Threshold for slight advantage (combat power ratio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float SlightAdvantageThreshold = 1.3f;

	/** Chance to pursue fleeing opponent (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PursuitChance = 0.7f;

	/** Maximum pursuit distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	float MaxPursuitDistance = 5000.0f;

	/** Duration to flee after player interrupts dispute (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	float PlayerInterruptFleeTime = 3.0f;

	/** Minimum health percentage to engage in dispute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinHealthToEngage = 0.3f;

	/** Can this monster initiate disputes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	bool bCanInitiateDispute = true;

	/** Can defend when challenged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory Dispute")
	bool bCanDefendTerritory = true;

	/** Show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

	// ==================== Events ====================

	UPROPERTY(BlueprintAssignable, Category = "Territory Dispute")
	FOnTerritoryDisputeStarted OnDisputeStarted;

	UPROPERTY(BlueprintAssignable, Category = "Territory Dispute")
	FOnTerritoryDisputeEnded OnDisputeEnded;

	UPROPERTY(BlueprintAssignable, Category = "Territory Dispute")
	FOnFleeingFromOpponent OnFleeingFromOpponent;

	UPROPERTY(BlueprintAssignable, Category = "Territory Dispute")
	FOnPursuingOpponent OnPursuingOpponent;

	UPROPERTY(BlueprintAssignable, Category = "Territory Dispute")
	FOnPlayerInterruptedDispute OnPlayerInterruptedDispute;

	// ==================== Public API ====================

	/** Get current dispute state */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	EHarmoniaTerritoryDisputeState GetDisputeState() const { return DisputeState; }

	/** Get current opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	AActor* GetCurrentOpponent() const { return CurrentOpponent; }

	/** Evaluate combat advantage against specific opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	EHarmoniaCombatAdvantage EvaluateCombatAdvantage(AActor* Opponent);

	/** Start territory dispute with target */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	void StartDispute(AActor* Target);

	/** End current dispute */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	void EndDispute(bool bVictorious);

	/** Flee from opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	void FleeFromOpponent(AActor* Opponent);

	/** Pursue fleeing opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	void PursueOpponent(AActor* Opponent);

	/** Handle player interruption */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	void HandlePlayerInterruption(AActor* Player);

	/** Check if should flee from current opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	bool ShouldFleeFromOpponent(AActor* Opponent);

	/** Check if should pursue opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	bool ShouldPursueOpponent(AActor* Opponent);

	/** Get flee location away from opponent */
	UFUNCTION(BlueprintCallable, Category = "Territory Dispute")
	FVector GetFleeLocation(AActor* Opponent);

protected:
	// ==================== Internal Functions ====================

	/** Scan for potential dispute targets */
	void ScanForOpponents();

	/** Evaluate whether to start dispute with detected opponent */
	bool ShouldEngageOpponent(AHarmoniaMonsterBase* Opponent);

	/** Calculate combat power of actor */
	float CalculateCombatPower(AActor* Actor);

	/** Get opponent info */
	FHarmoniaOpponentInfo GetOpponentInfo(AActor* Opponent);

	/** Update current dispute state */
	void UpdateDisputeState(float DeltaTime);

	/** Check for player interruption */
	void CheckPlayerInterruption();

	/** Draw debug information */
	void DrawDebugInfo();

	// ==================== State ====================

	/** Current dispute state */
	UPROPERTY(BlueprintReadOnly, Category = "Territory Dispute", meta = (AllowPrivateAccess = "true"))
	EHarmoniaTerritoryDisputeState DisputeState;

	/** Current opponent in dispute */
	UPROPERTY(BlueprintReadOnly, Category = "Territory Dispute", meta = (AllowPrivateAccess = "true"))
	AActor* CurrentOpponent;

	/** Scan timer */
	float ScanTimer;

	/** Player interrupt flee timer */
	float FleeTimer;

	/** Initial dispute location */
	FVector DisputeStartLocation;

	/** Cached owner monster */
	UPROPERTY()
	AHarmoniaMonsterBase* OwnerMonster;

	/** Detected opponents in range */
	UPROPERTY()
	TArray<AActor*> DetectedOpponents;
};
