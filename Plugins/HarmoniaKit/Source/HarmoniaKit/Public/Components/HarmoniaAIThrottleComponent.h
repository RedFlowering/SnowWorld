// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/HarmoniaUpdateRateSubsystem.h"
#include "HarmoniaAIThrottleComponent.generated.h"

class UBehaviorTree;
class UBlackboardComponent;
class AAIController;

/**
 * AI throttle state
 */
UENUM(BlueprintType)
enum class EHarmoniaAIThrottleState : uint8
{
	FullUpdate,        // All AI systems active
	ReducedUpdate,     // Reduced perception, normal behavior
	MinimalUpdate,     // Minimal perception, simplified behavior
	Suspended          // AI suspended, only basic state checks
};

/**
 * AI throttle configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAIThrottleConfig
{
	GENERATED_BODY()

	// ============================================================================
	// Perception Settings
	// ============================================================================

	/** Perception update interval at full update (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float FullPerceptionInterval = 0.1f;

	/** Perception update interval at reduced update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float ReducedPerceptionInterval = 0.5f;

	/** Perception update interval at minimal update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float MinimalPerceptionInterval = 2.0f;

	/** Whether to disable sight perception when suspended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	bool bDisableSightWhenSuspended = true;

	/** Whether to disable hearing perception when suspended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	bool bDisableHearingWhenSuspended = true;

	// ============================================================================
	// Behavior Tree Settings
	// ============================================================================

	/** BT tick interval at full update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTree")
	float FullBTInterval = 0.0f; // Every frame

	/** BT tick interval at reduced update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTree")
	float ReducedBTInterval = 0.1f;

	/** BT tick interval at minimal update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTree")
	float MinimalBTInterval = 0.5f;

	/** Whether to pause BT when suspended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTree")
	bool bPauseBTWhenSuspended = true;

	// ============================================================================
	// Movement Settings
	// ============================================================================

	/** Path update interval at full update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FullPathUpdateInterval = 0.2f;

	/** Path update interval at reduced/minimal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ThrottledPathUpdateInterval = 1.0f;

	/** Whether to stop movement when suspended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bStopMovementWhenSuspended = false;

	// ============================================================================
	// Animation Settings
	// ============================================================================

	/** Whether to use reduced animation LOD when throttled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bReduceAnimationLOD = true;

	/** Whether to skip animation when suspended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bSkipAnimationWhenSuspended = false;

	// ============================================================================
	// Integration Settings
	// ============================================================================

	/** Whether to automatically register with UpdateRateSubsystem */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	bool bAutoRegisterWithUpdateRate = true;

	/** Update rate config (if auto-registering) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (EditCondition = "bAutoRegisterWithUpdateRate"))
	FHarmoniaUpdateRateConfig UpdateRateConfig;
};

/**
 * Delegate for throttle state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIThrottleStateChanged, EHarmoniaAIThrottleState, OldState, EHarmoniaAIThrottleState, NewState);

/**
 * Harmonia AI Throttle Component
 *
 * Manages AI update frequency based on distance and significance.
 * Reduces CPU usage by throttling perception, behavior trees,
 * pathfinding, and animation for distant AI agents.
 *
 * Features:
 * - Automatic integration with UpdateRateSubsystem
 * - Perception interval adjustment
 * - Behavior tree tick throttling
 * - Path update frequency control
 * - Animation LOD reduction
 * - Combat state awareness
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaAIThrottleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaAIThrottleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Throttle configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|AIThrottle")
	FHarmoniaAIThrottleConfig Config;

	// ============================================================================
	// State Queries
	// ============================================================================

	/** Get current throttle state */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AIThrottle")
	EHarmoniaAIThrottleState GetThrottleState() const { return CurrentState; }

	/** Check if AI is at full update rate */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AIThrottle")
	bool IsFullUpdate() const { return CurrentState == EHarmoniaAIThrottleState::FullUpdate; }

	/** Check if AI is suspended */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AIThrottle")
	bool IsSuspended() const { return CurrentState == EHarmoniaAIThrottleState::Suspended; }

	/** Get current perception interval */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AIThrottle")
	float GetCurrentPerceptionInterval() const;

	/** Get current BT interval */
	UFUNCTION(BlueprintPure, Category = "Harmonia|AIThrottle")
	float GetCurrentBTInterval() const;

	// ============================================================================
	// Manual Control
	// ============================================================================

	/** Force a specific throttle state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|AIThrottle")
	void ForceThrottleState(EHarmoniaAIThrottleState State);

	/** Clear forced state, return to automatic */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|AIThrottle")
	void ClearForcedState();

	/** Set combat state (combat = no throttling) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|AIThrottle")
	void SetInCombat(bool bInCombat);

	/** Temporarily boost to full update for duration */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|AIThrottle")
	void BoostUpdateRate(float Duration);

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|AIThrottle")
	FOnAIThrottleStateChanged OnThrottleStateChanged;

protected:
	/** Called when throttle state changes */
	virtual void OnStateChanged(EHarmoniaAIThrottleState OldState, EHarmoniaAIThrottleState NewState);

	/** Apply perception throttling */
	void ApplyPerceptionThrottling();

	/** Apply behavior tree throttling */
	void ApplyBehaviorTreeThrottling();

	/** Apply movement throttling */
	void ApplyMovementThrottling();

	/** Apply animation throttling */
	void ApplyAnimationThrottling();

	/** Convert update tier to throttle state */
	EHarmoniaAIThrottleState TierToThrottleState(EHarmoniaUpdateTier Tier) const;

	/** Handle update tier changed */
	UFUNCTION()
	void OnUpdateTierChanged(AActor* Actor, EHarmoniaUpdateTier OldTier, EHarmoniaUpdateTier NewTier);

private:
	/** Current throttle state */
	EHarmoniaAIThrottleState CurrentState = EHarmoniaAIThrottleState::FullUpdate;

	/** Whether state is forced */
	bool bStateForced = false;

	/** Forced state value */
	EHarmoniaAIThrottleState ForcedState = EHarmoniaAIThrottleState::FullUpdate;

	/** Whether in combat */
	bool bIsInCombat = false;

	/** Boost timer remaining */
	float BoostTimeRemaining = 0.0f;

	/** Cached AI controller */
	UPROPERTY()
	TWeakObjectPtr<AAIController> CachedAIController;

	/** Cached perception component */
	UPROPERTY()
	TWeakObjectPtr<UActorComponent> CachedPerceptionComponent;

	/** Original perception interval (for restoration) */
	float OriginalPerceptionInterval = 0.1f;
};
