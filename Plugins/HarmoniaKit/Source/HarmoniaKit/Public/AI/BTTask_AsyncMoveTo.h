// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_AsyncMoveTo.generated.h"

/**
 * UBTTask_AsyncMoveTo
 *
 * Optimized movement task with async pathfinding support
 * Extends BTTask_MoveTo with performance improvements:
 * - Async pathfinding (non-blocking)
 * - LOD-aware acceptable radius
 * - Distance-based path update frequency
 * - Early exit for unreachable destinations
 *
 * Performance benefits:
 * - Doesn't block AI tick while finding path
 * - Reduces pathfinding frequency for distant targets
 * - Adapts acceptable radius based on LOD level
 */
UCLASS()
class HARMONIAKIT_API UBTTask_AsyncMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_AsyncMoveTo();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Use async pathfinding
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization")
	bool bUseAsyncPathfinding = true;

	/**
	 * Adapt acceptable radius based on LOD level
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization")
	bool bAdaptRadiusToLOD = true;

	/**
	 * Multiplier for acceptable radius at low LOD
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float LowLODRadiusMultiplier = 2.0f;

	/**
	 * Maximum time to wait for pathfinding result
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float PathfindingTimeout = 2.0f;

	/**
	 * Minimum time between path updates (in seconds)
	 */
	UPROPERTY(EditAnywhere, Category = "Optimization", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MinPathUpdateInterval = 0.5f;

private:
	/**
	 * Get acceptable radius based on LOD level
	 */
	float GetLODAdaptedRadius(const UBehaviorTreeComponent& OwnerComp, AActor* Owner) const;

	/**
	 * Check if should update path this tick
	 */
	bool ShouldUpdatePath(float TimeSinceLastUpdate) const;
};
