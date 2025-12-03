// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MaintainSquadFormation.generated.h"

/**
 * UBTTask_MaintainSquadFormation
 *
 * Behavior tree task that moves monster to its squad formation position
 * Works with UHarmoniaSquadComponent to coordinate group movement
 *
 * Usage:
 * - Add to behavior tree for monsters with squad component
 * - Task will move monster to formation position relative to squad leader
 * - Completes when monster reaches formation position (or has no squad)
 */
UCLASS()
class HARMONIAKIT_API UBTTask_MaintainSquadFormation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MaintainSquadFormation();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Acceptable distance to formation position
	 */
	UPROPERTY(EditAnywhere, Category = "Squad", meta = (ClampMin = "50.0"))
	float AcceptableRadius = 100.0f;

	/**
	 * Whether to use pathfinding (vs direct movement)
	 */
	UPROPERTY(EditAnywhere, Category = "Squad")
	bool bUsePathfinding = true;

	/**
	 * Maximum time to try reaching formation position
	 */
	UPROPERTY(EditAnywhere, Category = "Squad", meta = (ClampMin = "1.0"))
	float MaxExecutionTime = 5.0f;

private:
	/**
	 * Time spent executing this task
	 */
	float ExecutionTime = 0.0f;
};
