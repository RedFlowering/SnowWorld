// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SenseStimulusComponent.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaSenseComponent.generated.h"

class UAbilitySystemComponent;
class USenseReceiverComponent;
class UGameplayEffect;
struct FSensedStimulus;
class UHarmoniaSenseInteractableComponent;
class UHarmoniaSenseInteractionComponent;

/**
 * Delegate for attack hit events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackHitDelegate, const FHarmoniaAttackHitResult&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackStartDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEndDelegate);

/**
 * UHarmoniaSenseComponent
 *
 * Central SenseSystem management component for characters.
 * Automatically ensures owner has required SenseSystem components and manages combat/interaction detection.
 *
 * Features:
 * - Auto-creates HarmoniaSenseInteractableComponent (Stimulus) if missing
 * - Auto-creates HarmoniaSenseInteractionComponent (Receiver) if missing
 * - Attack hit detection using multithreaded sense detection
 * - Multiple trace shapes (Box, Sphere, Capsule, Line)
 * - Continuous or single-shot detection
 * - Damage type support (Instant, Duration, Explosion)
 * - Gameplay Effect and Cue integration
 * - Critical hit calculation
 */
UCLASS(Blueprintable, ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSenseComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHarmoniaSenseComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Attack Configuration
	// ============================================================================

	/**
	 * Attack data configuration
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Configuration")
	FHarmoniaAttackData AttackData;

	/**
	 * Whether to auto-initialize sense stimulus on BeginPlay
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Configuration")
	bool bAutoInitializeSenseStimulus = true;

	/**
	 * Owner's ability system component (auto-found if null)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|Configuration")
	TObjectPtr<UAbilitySystemComponent> OwnerAbilitySystem = nullptr;

	// ============================================================================
	// Owner SenseSystem Component Management
	// ============================================================================

	/** Owner's interactable component (Stimulus) - created if missing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sense|Owner Components")
	TObjectPtr<UHarmoniaSenseInteractableComponent> OwnerInteractable = nullptr;

	/** Owner's interaction component (Receiver) - created if missing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sense|Owner Components")
	TObjectPtr<UHarmoniaSenseInteractionComponent> OwnerInteraction = nullptr;

	/** Sensor tag used for combat interactions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Configuration")
	FName CombatSensorTag = FName("Combat");

	/** Sense channel used for combat detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Configuration")
	int32 CombatSenseChannel = 1;

	// ============================================================================
	// Attack Control
	// ============================================================================

	/**
	 * Request to start attack detection (Client -> Server)
	 * @param InAttackData Optional attack data to override default configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void RequestStartAttack(const FHarmoniaAttackData& InAttackData);

	/**
	 * Request to start attack detection with default configuration (Client -> Server)
	 */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void RequestStartAttackDefault();

	/**
	 * Request to stop attack detection (Client -> Server)
	 */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void RequestStopAttack();

	/**
	 * Check if currently attacking
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attack")
	bool IsAttacking() const { return bIsAttacking; }

	/**
	 * Get all targets hit this attack
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attack")
	TArray<FHarmoniaAttackHitResult> GetHitTargets() const { return HitTargets; }

	/**
	 * Clear hit target history
	 */
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ClearHitTargets();

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when an attack hits a target
	 */
	UPROPERTY(BlueprintAssignable, Category = "Attack|Events")
	FOnAttackHitDelegate OnAttackHit;

	/**
	 * Called when attack starts
	 */
	UPROPERTY(BlueprintAssignable, Category = "Attack|Events")
	FOnAttackStartDelegate OnAttackStart;

	/**
	 * Called when attack ends
	 */
	UPROPERTY(BlueprintAssignable, Category = "Attack|Events")
	FOnAttackEndDelegate OnAttackEnd;

protected:
	// ============================================================================
	// Server RPCs
	// ============================================================================

	/**
	 * Server RPC for starting attack
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartAttack(const FHarmoniaAttackData& InAttackData);

	/**
	 * Server RPC for stopping attack
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopAttack();

	// ============================================================================
	// Owner SenseSystem Initialization
	// ============================================================================

	/**
	 * Initialize owner's SenseSystem components (Interactable + Interaction)
	 * Called during BeginPlay - ensures owner can be detected and can detect others
	 */
	virtual void InitializeOwnerSenseComponents();

	/**
	 * Ensure owner has HarmoniaSenseInteractableComponent (Stimulus)
	 * Creates one if not present and configures for combat detection
	 */
	virtual void EnsureOwnerInteractable();

	/**
	 * Ensure owner has HarmoniaSenseInteractionComponent (Receiver)
	 * Creates one if not present and configures for combat detection
	 */
	virtual void EnsureOwnerInteraction();

	// ============================================================================
	// Internal Attack Functions (Server-only)
	// ============================================================================

	/**
	 * Start attack detection (Server-only)
	 * @param InAttackData Optional attack data to override default configuration
	 */
	virtual void StartAttack(const FHarmoniaAttackData& InAttackData);

	/**
	 * Stop attack detection (Server-only)
	 */
	virtual void StopAttack();

	// ============================================================================
	// Sense System Integration
	// ============================================================================

	/**
	 * Initialize sense stimulus component
	 */
	virtual void InitializeSenseStimulus();

	/**
	 * Update sense stimulus based on attack config
	 */
	virtual void UpdateSenseStimulus();

	/**
	 * Setup sense receiver for hit detection
	 */
	virtual void SetupSenseReceiver();

	/**
	 * Cleanup sense receiver
	 */
	virtual void CleanupSenseReceiver();

	/**
	 * Process detected stimuli and check for hits
	 */
	UFUNCTION()
	virtual void OnSenseDetected(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli);

	/**
	 * Process a single detected target
	 * @return true if hit was processed
	 */
	virtual bool ProcessHitTarget(const FSensedStimulus& Stimulus);

	/**
	 * Check if target should be hit
	 * @return true if target passes all checks
	 */
	virtual bool ShouldHitTarget(AActor* TargetActor, const FSensedStimulus& Stimulus) const;

	/**
	 * Calculate if hit is critical
	 */
	virtual bool CalculateCriticalHit(float CritChance) const;

	/**
	 * Apply damage to target using Gameplay Effect
	 * @return The actual damage dealt (after all calculations)
	 */
	virtual float ApplyDamageToTarget(
		AActor* TargetActor,
		const FHarmoniaDamageEffectConfig& DamageConfig,
		bool bWasCritical,
		const FVector& HitLocation,
		const FVector& HitNormal);

	/**
	 * Apply hit reaction to target using Gameplay Cue
	 */
	virtual void ApplyHitReaction(
		AActor* TargetActor,
		const FHarmoniaHitReactionConfig& ReactionConfig,
		bool bWasCritical,
		const FVector& HitLocation,
		const FVector& HitDirection);

	/**
	 * Create Gameplay Effect Context for damage application
	 */
	virtual FGameplayEffectContextHandle CreateDamageEffectContext(
		AActor* TargetActor,
		const FVector& HitLocation,
		const FVector& HitNormal) const;

	/**
	 * Handle attack timer completion
	 */
	virtual void OnAttackTimerComplete();

	/**
	 * Get socket location for trace (or component location if no socket)
	 */
	virtual FVector GetTraceLocation() const;

	/**
	 * Get socket rotation for trace (or component rotation if no socket)
	 */
	virtual FRotator GetTraceRotation() const;

	/**
	 * Draw debug visualization for current attack
	 */
	virtual void DrawDebugAttackTrace() const;

	/**
	 * Draw SenseSystem debug visualization (sensor detection, stimulus, scores)
	 * Only available with ENABLE_DRAW_DEBUG macro
	 */
	virtual void DrawSenseSystemDebug() const;

	/**
	 * Minimum interval between attacks (anti-spam)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack|AntiCheat")
	float MinAttackInterval = 0.1f;

	/**
	 * Last attack request time (for rate limiting)
	 */
	float LastAttackRequestTime = 0.0f;

private:
	/**
	 * Sense stimulus component for attack detection
	 */
	UPROPERTY()
	TObjectPtr<USenseStimulusComponent> SenseStimulus = nullptr;

	/**
	 * Temporary sense receiver for detecting hits
	 * Created dynamically during attacks
	 */
	UPROPERTY()
	TObjectPtr<USenseReceiverComponent> SenseReceiver = nullptr;

	/**
	 * Current attack state
	 */
	bool bIsAttacking = false;

	/**
	 * Current attack data being used
	 */
	FHarmoniaAttackData CurrentAttackData;

	/**
	 * Targets hit during current attack
	 */
	UPROPERTY(Transient)
	TArray<FHarmoniaAttackHitResult> HitTargets;

	/**
	 * Set of actors already hit (for hit-once tracking)
	 */
	TSet<TObjectPtr<AActor>> HitActors;

	/**
	 * Timer handle for continuous attack duration
	 */
	FTimerHandle AttackTimerHandle;

	/**
	 * Attack start time
	 */
	float AttackStartTime = 0.0f;
};

