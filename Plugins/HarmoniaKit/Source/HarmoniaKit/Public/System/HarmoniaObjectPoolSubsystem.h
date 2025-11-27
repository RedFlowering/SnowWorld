// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "HarmoniaObjectPoolSubsystem.generated.h"

class AActor;
class UNiagaraComponent;
class UAudioComponent;

/**
 * Pool Configuration for a specific actor class
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPoolConfig
{
	GENERATED_BODY()

	/** Actor class to pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	TSubclassOf<AActor> ActorClass;

	/** Initial pool size (pre-spawned at initialization) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool", meta = (ClampMin = "0", ClampMax = "1000"))
	int32 InitialSize = 10;

	/** Maximum pool size (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool", meta = (ClampMin = "0"))
	int32 MaxSize = 50;

	/** Whether to grow the pool when empty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	bool bCanGrow = true;

	/** Number of actors to spawn when pool needs to grow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool", meta = (ClampMin = "1", ClampMax = "100"))
	int32 GrowthAmount = 5;

	/** Time in seconds before inactive pooled actors are destroyed (0 = never) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool", meta = (ClampMin = "0"))
	float ShrinkTimeout = 60.0f;

	/** Identifier tag for this pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pool")
	FGameplayTag PoolTag;
};

/**
 * Statistics for a single pool
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaPoolStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 ActiveCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 InactiveCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalAcquired = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalReleased = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TotalCreated = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 PoolMisses = 0;  // Times we needed to create new because pool was empty
};

/**
 * Interface for poolable actors
 * Actors implementing this interface will receive pool lifecycle callbacks
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UHarmoniaPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

class HARMONIAKIT_API IHarmoniaPoolableInterface
{
	GENERATED_BODY()

public:
	/** Called when actor is acquired from pool */
	UFUNCTION(BlueprintNativeEvent, Category = "Pool")
	void OnAcquiredFromPool();

	/** Called when actor is released back to pool */
	UFUNCTION(BlueprintNativeEvent, Category = "Pool")
	void OnReleasedToPool();

	/** Called to reset actor state before returning to pool */
	UFUNCTION(BlueprintNativeEvent, Category = "Pool")
	void ResetPooledState();

	/** Returns true if this actor can be returned to pool (not destroyed) */
	UFUNCTION(BlueprintNativeEvent, Category = "Pool")
	bool CanReturnToPool() const;
};

/**
 * Internal pool data structure
 */
USTRUCT()
struct FHarmoniaActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	FHarmoniaPoolConfig Config;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> InactiveActors;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> ActiveActors;

	UPROPERTY()
	FHarmoniaPoolStats Stats;

	/** Timestamp of last acquisition (for shrink timeout) */
	double LastAcquireTime = 0.0;

	/** Timestamp of last shrink check */
	double LastShrinkCheckTime = 0.0;
};

/**
 * Delegate fired when pool statistics change significantly
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoolStatsChanged, TSubclassOf<AActor>, ActorClass, const FHarmoniaPoolStats&, Stats);

/**
 * Harmonia Object Pool Subsystem
 * 
 * High-performance object pooling system for frequently spawned/destroyed actors.
 * Reduces GC pressure and improves frame consistency.
 *
 * Features:
 * - Generic actor pooling with class-based or tag-based lookup
 * - Automatic pool growth and shrinking
 * - Blueprint and C++ support
 * - Pool statistics and debugging
 * - Interface-based lifecycle callbacks
 *
 * Ideal for:
 * - Projectiles (arrows, bullets, spells)
 * - Damage numbers / UI popups
 * - Hit effects (decals, particles)
 * - AI agents in spawn/despawn heavy scenarios
 *
 * Usage:
 * 1. Configure pools in Project Settings or call RegisterPool()
 * 2. Use AcquireActor() instead of SpawnActor()
 * 3. Use ReleaseActor() instead of DestroyActor()
 * 4. Implement IHarmoniaPoolableInterface for custom reset logic
 */
UCLASS(Config = Game)
class HARMONIAKIT_API UHarmoniaObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	// ============================================================================
	// Pool Registration
	// ============================================================================

	/**
	 * Register a new actor pool
	 * @param Config - Pool configuration
	 * @return True if pool was registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	bool RegisterPool(const FHarmoniaPoolConfig& Config);

	/**
	 * Register a pool with simple parameters
	 * @param ActorClass - Actor class to pool
	 * @param InitialSize - Initial pool size
	 * @param MaxSize - Maximum pool size (0 = unlimited)
	 * @return True if pool was registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	bool RegisterPoolSimple(TSubclassOf<AActor> ActorClass, int32 InitialSize = 10, int32 MaxSize = 50);

	/**
	 * Unregister and destroy a pool
	 * @param ActorClass - Actor class of the pool to remove
	 * @param bDestroyActors - If true, destroys all pooled actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void UnregisterPool(TSubclassOf<AActor> ActorClass, bool bDestroyActors = true);

	/**
	 * Check if a pool exists for the given class
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|ObjectPool")
	bool HasPool(TSubclassOf<AActor> ActorClass) const;

	// ============================================================================
	// Actor Acquisition & Release
	// ============================================================================

	/**
	 * Acquire an actor from the pool
	 * @param ActorClass - Class of actor to acquire
	 * @param Transform - Transform to apply to the actor
	 * @param bAutoActivate - If true, actor is activated immediately
	 * @return Pooled actor, or nullptr if pool is empty and cannot grow
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool", meta = (DeterminesOutputType = "ActorClass"))
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform, bool bAutoActivate = true);

	/**
	 * Acquire an actor from pool by tag
	 * @param PoolTag - Tag identifying the pool
	 * @param Transform - Transform to apply
	 * @param bAutoActivate - If true, actor is activated immediately
	 * @return Pooled actor, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	AActor* AcquireActorByTag(FGameplayTag PoolTag, const FTransform& Transform, bool bAutoActivate = true);

	/**
	 * Release an actor back to the pool
	 * @param Actor - Actor to release
	 * @return True if actor was returned to pool, false if destroyed
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	bool ReleaseActor(AActor* Actor);

	/**
	 * Release an actor back to pool after a delay
	 * @param Actor - Actor to release
	 * @param Delay - Delay in seconds before releasing
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void ReleaseActorDelayed(AActor* Actor, float Delay);

	/**
	 * Release all active actors of a class back to pool
	 * @param ActorClass - Class of actors to release
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void ReleaseAllActorsOfClass(TSubclassOf<AActor> ActorClass);

	// ============================================================================
	// Pool Management
	// ============================================================================

	/**
	 * Pre-warm a pool by spawning actors up to the initial size
	 * Call this during loading screens for smoother gameplay
	 * @param ActorClass - Class of pool to warm
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void WarmPool(TSubclassOf<AActor> ActorClass);

	/**
	 * Pre-warm all registered pools
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void WarmAllPools();

	/**
	 * Shrink a pool to its initial size, destroying excess actors
	 * @param ActorClass - Class of pool to shrink
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void ShrinkPool(TSubclassOf<AActor> ActorClass);

	/**
	 * Clear all pools, destroying all pooled actors
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool")
	void ClearAllPools();

	// ============================================================================
	// Statistics & Debugging
	// ============================================================================

	/**
	 * Get statistics for a specific pool
	 * @param ActorClass - Class of pool to query
	 * @param OutStats - Output statistics
	 * @return True if pool exists
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|ObjectPool")
	bool GetPoolStats(TSubclassOf<AActor> ActorClass, FHarmoniaPoolStats& OutStats) const;

	/**
	 * Get all registered pool classes
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|ObjectPool")
	TArray<TSubclassOf<AActor>> GetAllPooledClasses() const;

	/**
	 * Get total number of pooled actors (active + inactive)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|ObjectPool")
	int32 GetTotalPooledActorCount() const;

	/**
	 * Debug: Draw pool statistics on screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|ObjectPool|Debug")
	void DebugDrawPoolStats(bool bEnabled);

	// ============================================================================
	// Events
	// ============================================================================

	/** Fired when pool statistics change significantly */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|ObjectPool")
	FOnPoolStatsChanged OnPoolStatsChanged;

protected:
	/** Tick function for pool maintenance */
	void TickPoolMaintenance(float DeltaTime);

	/** Create a new actor for the pool */
	AActor* CreatePooledActor(const FHarmoniaPoolConfig& Config);

	/** Deactivate an actor (hide, disable collision, etc.) */
	void DeactivateActor(AActor* Actor);

	/** Activate an actor (show, enable collision, etc.) */
	void ActivateActor(AActor* Actor, const FTransform& Transform);

	/** Find pool by actor class */
	FHarmoniaActorPool* FindPool(TSubclassOf<AActor> ActorClass);
	const FHarmoniaActorPool* FindPool(TSubclassOf<AActor> ActorClass) const;

	/** Find pool by tag */
	FHarmoniaActorPool* FindPoolByTag(FGameplayTag Tag);

	/** Handle automatic pool shrinking */
	void ProcessPoolShrinking(float DeltaTime);

	/** Clean up null references in pools */
	void CleanupNullReferences();

private:
	/** Map of actor class to pool data */
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FHarmoniaActorPool> Pools;

	/** Map of tags to actor classes for fast tag lookup */
	UPROPERTY()
	TMap<FGameplayTag, TSubclassOf<AActor>> TagToClassMap;

	/** Timer handle for pool maintenance */
	FTimerHandle MaintenanceTimerHandle;

	/** Timer handles for delayed releases */
	TMap<AActor*, FTimerHandle> DelayedReleaseHandles;

	/** Whether debug drawing is enabled */
	bool bDebugDrawEnabled = false;

	/** Interval for pool maintenance tick (seconds) */
	UPROPERTY(Config)
	float MaintenanceInterval = 5.0f;

	/** Default configs loaded from ini */
	UPROPERTY(Config)
	TArray<FHarmoniaPoolConfig> DefaultPoolConfigs;
};
