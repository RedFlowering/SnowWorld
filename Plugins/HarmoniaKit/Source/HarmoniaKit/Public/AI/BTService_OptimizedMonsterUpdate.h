// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_OptimizedMonsterUpdate.generated.h"

/**
 * Cached Query Result
 * Stores results of expensive queries with timestamp
 */
USTRUCT()
struct FBTCachedQueryResult
{
	GENERATED_BODY()

	// Cached target actor
	UPROPERTY()
	TObjectPtr<AActor> CachedTarget = nullptr;

	// Distance to target
	UPROPERTY()
	float CachedDistance = 0.0f;

	// Whether target is in attack range
	UPROPERTY()
	bool bInAttackRange = false;

	// Timestamp when cached
	UPROPERTY()
	float CacheTime = 0.0f;

	// Whether cache is valid
	UPROPERTY()
	bool bIsValid = false;

	FBTCachedQueryResult()
	{
	}

	// Check if cache is still valid
	bool IsCacheValid(float CurrentTime, float MaxCacheAge) const
	{
		return bIsValid && (CurrentTime - CacheTime) < MaxCacheAge;
	}

	// Invalidate cache
	void Invalidate()
	{
		bIsValid = false;
	}
};

/**
 * UBTService_OptimizedMonsterUpdate
 *
 * Optimized version of monster state update service
 * Features:
 * - LOD-aware update frequency
 * - Query result caching
 * - Conditional activation (only in combat)
 * - Async pathfinding support
 * - Blackboard value caching
 *
 * Performance improvements:
 * - Reduces expensive queries
 * - Skips updates for distant monsters
 * - Caches blackboard reads/writes
 * - Only activates when needed
 */
UCLASS()
class HARMONIAKIT_API UBTService_OptimizedMonsterUpdate : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_OptimizedMonsterUpdate();

	//~UBTService interface
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTService interface

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Blackboard key for target actor
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Blackboard key for distance to target
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceKey;

	/**
	 * Blackboard key for "in attack range" bool
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InAttackRangeKey;

	/**
	 * Only update when in combat state
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization")
	bool bOnlyUpdateInCombat = true;

	/**
	 * Use LOD system for update frequency
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization")
	bool bUseLODSystem = true;

	/**
	 * Enable query result caching
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization|Caching")
	bool bEnableCaching = true;

	/**
	 * Maximum age of cached queries (in seconds)
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization|Caching", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MaxCacheAge = 0.5f;

	/**
	 * Distance threshold for "in attack range" check
	 */
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackRangeThreshold = 300.0f;

	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Check if should update this tick
	 */
	bool ShouldUpdateThisTick(UBehaviorTreeComponent& OwnerComp, float DeltaSeconds);

	/**
	 * Update cached query results
	 */
	void UpdateCachedQueries(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

	/**
	 * Get or create cache for this node
	 */
	FBTCachedQueryResult* GetCache(uint8* NodeMemory);

	/**
	 * Check if owner is in combat
	 */
	bool IsInCombat(AActor* Owner) const;
};
