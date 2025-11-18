// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterPatrol.generated.h"

/**
 * UBTTask_MonsterPatrol
 *
 * Moves monster to a random patrol point within patrol radius
 */
UCLASS()
class HARMONIAKIT_API UBTTask_MonsterPatrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterPatrol();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Enable/Disable this task
	 */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/**
	 * Blackboard key for home location (patrol center)
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HomeLocationKey;

	/**
	 * Blackboard key to store patrol destination
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey;

	/**
	 * Override patrol radius (0 = use monster data)
	 */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolRadiusOverride = 0.0f;

	/**
	 * Acceptable radius for reaching patrol point
	 */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	float AcceptableRadius = 100.0f;

	/**
	 * Whether to wait at patrol point before selecting next
	 */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	bool bWaitAtPatrolPoint = true;

	/**
	 * Wait time at patrol point (randomized between min-max)
	 */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (EditCondition = "bWaitAtPatrolPoint"))
	float MinWaitTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (EditCondition = "bWaitAtPatrolPoint"))
	float MaxWaitTime = 5.0f;
};
