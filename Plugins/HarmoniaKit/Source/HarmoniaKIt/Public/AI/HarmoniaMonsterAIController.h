// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "HarmoniaMonsterAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UBehaviorTree;
class AHarmoniaMonsterBase;

/**
 * AHarmoniaMonsterAIController
 *
 * AI Controller for monster characters
 * Handles perception, behavior tree execution, and combat targeting
 *
 * Features:
 * - AI Perception (sight, hearing, damage)
 * - Behavior tree integration
 * - Target selection and tracking
 * - Aggro management
 * - Compatible with all monster types (humanoid, creature, mechanical, etc.)
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHarmoniaMonsterAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~AAIController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;
	//~End of AAIController interface

	// ============================================================================
	// AI Configuration
	// ============================================================================

	/**
	 * Sight perception configuration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig = nullptr;

	/**
	 * Hearing perception configuration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig = nullptr;

	/**
	 * Damage perception configuration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig = nullptr;

	// ============================================================================
	// Target Management
	// ============================================================================

	/**
	 * Get the current target actor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	AActor* GetCurrentTarget() const;

	/**
	 * Set the current target actor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	void SetCurrentTarget(AActor* NewTarget);

	/**
	 * Clear the current target
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	void ClearTarget();

	/**
	 * Check if target is valid and alive
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	bool IsTargetValid() const;

	/**
	 * Get distance to current target
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Target")
	float GetDistanceToTarget() const;

	// ============================================================================
	// Combat
	// ============================================================================

	/**
	 * Request monster to perform attack
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool RequestAttack(FName AttackID = NAME_None);

	/**
	 * Select best attack for current situation
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	FName SelectBestAttack() const;

	// ============================================================================
	// Patrol & Movement
	// ============================================================================

	/**
	 * Get random patrol location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	FVector GetRandomPatrolLocation() const;

	/**
	 * Move to patrol location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	bool MoveToPatrolLocation(const FVector& PatrolLocation);

	/**
	 * Get spawn/home location
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	FVector GetHomeLocation() const { return HomeLocation; }

protected:
	// ============================================================================
	// AI Perception
	// ============================================================================

	/**
	 * Called when perception is updated
	 */
	UFUNCTION()
	virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	/**
	 * Called when target is perceived
	 */
	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/**
	 * Check if actor should be considered as target
	 */
	virtual bool ShouldTargetActor(AActor* Actor) const;

	/**
	 * Select best target from perceived actors
	 */
	virtual AActor* SelectBestTarget(const TArray<AActor*>& PotentialTargets) const;

	// ============================================================================
	// Behavior Tree
	// ============================================================================

	/**
	 * Start behavior tree
	 */
	virtual void StartBehaviorTree();

	/**
	 * Stop behavior tree
	 */
	virtual void StopBehaviorTree();

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Cached reference to controlled monster
	 */
	UPROPERTY()
	TObjectPtr<AHarmoniaMonsterBase> ControlledMonster = nullptr;

	/**
	 * Home/spawn location for patrol
	 */
	FVector HomeLocation = FVector::ZeroVector;

	/**
	 * Current behavior tree
	 */
	UPROPERTY()
	TObjectPtr<UBehaviorTree> CurrentBehaviorTree = nullptr;
};
