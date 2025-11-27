// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "HarmoniaUpdateRateSubsystem.generated.h"

class AActor;
class UActorComponent;

/**
 * Update rate tier based on significance/distance
 */
UENUM(BlueprintType)
enum class EHarmoniaUpdateTier : uint8
{
	Critical,      // Every frame (player, nearby enemies in combat)
	High,          // Every 2-3 frames
	Medium,        // Every 5-10 frames  
	Low,           // Every 15-30 frames
	Minimal,       // Every 60+ frames (far away, off-screen)
	Dormant        // No updates (completely inactive)
};

/**
 * Update rate configuration for an actor/component
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaUpdateRateConfig
{
	GENERATED_BODY()

	/** Distance thresholds for tier transitions (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float CriticalDistance = 1000.0f;  // 10m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float HighDistance = 3000.0f;      // 30m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float MediumDistance = 7000.0f;    // 70m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float LowDistance = 15000.0f;      // 150m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance")
	float MinimalDistance = 30000.0f;  // 300m

	/** Update intervals per tier (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intervals")
	float CriticalInterval = 0.0f;     // Every tick

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intervals")
	float HighInterval = 0.033f;       // ~30Hz

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intervals")
	float MediumInterval = 0.1f;       // 10Hz

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intervals")
	float LowInterval = 0.25f;         // 4Hz

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intervals")
	float MinimalInterval = 1.0f;      // 1Hz

	/** Whether to consider visibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
	bool bConsiderVisibility = true;

	/** Whether off-screen actors should be demoted one tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
	bool bDemoteOffScreen = true;

	/** Whether to consider combat state (combat = higher tier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bConsiderCombatState = true;

	/** Tag that indicates actor is in combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FGameplayTag InCombatTag;
};

/**
 * Tracked actor data
 */
USTRUCT()
struct FHarmoniaTrackedActorData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Actor;

	UPROPERTY()
	EHarmoniaUpdateTier CurrentTier = EHarmoniaUpdateTier::Medium;

	UPROPERTY()
	float LastUpdateTime = 0.0f;

	UPROPERTY()
	float DistanceToPlayer = 0.0f;

	UPROPERTY()
	bool bIsVisible = true;

	UPROPERTY()
	bool bIsInCombat = false;

	UPROPERTY()
	FHarmoniaUpdateRateConfig Config;
};

/**
 * Update rate statistics
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaUpdateRateStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalTrackedActors = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 CriticalCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 HighCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 MediumCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 LowCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 MinimalCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 DormantCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 UpdatesThisFrame = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 UpdatesSkippedThisFrame = 0;
};

/**
 * Delegate for update rate changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnUpdateTierChanged, AActor*, Actor, EHarmoniaUpdateTier, OldTier, EHarmoniaUpdateTier, NewTier);

/**
 * Harmonia Update Rate Subsystem
 *
 * Manages tick/update frequency for actors based on distance, visibility,
 * and significance. Reduces CPU usage by throttling updates for distant
 * or less important actors.
 *
 * Features:
 * - Distance-based update tier assignment
 * - Visibility consideration (off-screen demotion)
 * - Combat state awareness
 * - Per-actor configuration
 * - Statistics tracking
 * - Blueprint integration
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaUpdateRateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// Tick (returns bool for FTSTicker compatibility)
	bool Tick(float DeltaTime);

	// ============================================================================
	// Actor Registration
	// ============================================================================

	/** Register an actor for update rate management */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void RegisterActor(AActor* Actor, const FHarmoniaUpdateRateConfig& Config);

	/** Register with default config */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void RegisterActorWithDefaults(AActor* Actor);

	/** Unregister an actor */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void UnregisterActor(AActor* Actor);

	/** Check if actor is registered */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate")
	bool IsActorRegistered(AActor* Actor) const;

	// ============================================================================
	// Update Queries
	// ============================================================================

	/** Check if an actor should update this frame */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate")
	bool ShouldActorUpdate(AActor* Actor) const;

	/** Get actor's current update tier */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate")
	EHarmoniaUpdateTier GetActorUpdateTier(AActor* Actor) const;

	/** Get actor's update interval (seconds) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate")
	float GetActorUpdateInterval(AActor* Actor) const;

	/** Get time since last update for actor */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate")
	float GetTimeSinceLastUpdate(AActor* Actor) const;

	// ============================================================================
	// Manual Control
	// ============================================================================

	/** Force an actor to a specific tier (overrides automatic calculation) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void ForceActorTier(AActor* Actor, EHarmoniaUpdateTier Tier);

	/** Clear forced tier, return to automatic calculation */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void ClearForcedTier(AActor* Actor);

	/** Set actor's combat state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void SetActorInCombat(AActor* Actor, bool bInCombat);

	/** Mark actor as updated (call after processing) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate")
	void MarkActorUpdated(AActor* Actor);

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Set default configuration */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate|Config")
	void SetDefaultConfig(const FHarmoniaUpdateRateConfig& Config);

	/** Get default configuration */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate|Config")
	FHarmoniaUpdateRateConfig GetDefaultConfig() const { return DefaultConfig; }

	/** Set global update rate multiplier (1.0 = normal, 0.5 = half speed) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate|Config")
	void SetGlobalUpdateMultiplier(float Multiplier);

	// ============================================================================
	// Statistics
	// ============================================================================

	/** Get current statistics */
	UFUNCTION(BlueprintPure, Category = "Harmonia|UpdateRate|Stats")
	FHarmoniaUpdateRateStats GetStatistics() const { return CurrentStats; }

	/** Enable/disable statistics collection */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UpdateRate|Stats")
	void SetCollectStatistics(bool bEnable) { bCollectStatistics = bEnable; }

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UpdateRate")
	FOnUpdateTierChanged OnTierChanged;

protected:
	/** Update all tracked actors' tiers */
	void UpdateActorTiers();

	/** Calculate tier for a specific actor */
	EHarmoniaUpdateTier CalculateTier(const FHarmoniaTrackedActorData& Data) const;

	/** Get update interval for tier */
	float GetIntervalForTier(EHarmoniaUpdateTier Tier, const FHarmoniaUpdateRateConfig& Config) const;

	/** Get player location for distance calculations */
	FVector GetPlayerLocation() const;

	/** Check if actor is visible to player */
	bool IsActorVisible(AActor* Actor) const;

	/** Clean up invalid actors */
	void CleanupInvalidActors();

	/** Update statistics */
	void UpdateStatistics();

private:
	/** All tracked actors */
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, FHarmoniaTrackedActorData> TrackedActors;

	/** Actors with forced tiers */
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, EHarmoniaUpdateTier> ForcedTiers;

	/** Default configuration */
	UPROPERTY()
	FHarmoniaUpdateRateConfig DefaultConfig;

	/** Current statistics */
	FHarmoniaUpdateRateStats CurrentStats;

	/** Global update multiplier */
	float GlobalUpdateMultiplier = 1.0f;

	/** Whether to collect statistics */
	bool bCollectStatistics = true;

	/** Cached player location */
	FVector CachedPlayerLocation;

	/** Time accumulator for tier updates */
	float TierUpdateAccumulator = 0.0f;

	/** Tier update interval (how often to recalculate tiers) */
	float TierUpdateInterval = 0.5f;

	/** Delegate handle for tick */
	FTSTicker::FDelegateHandle TickDelegateHandle;
};
