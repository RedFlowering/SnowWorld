// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterFindTarget.generated.h"

/**
 * UBTTask_MonsterFindTarget
 *
 * Finds the best target for the monster to attack
 * Uses perception system and distance/threat calculations
 */
UCLASS()
class HARMONIAKIT_API UBTTask_MonsterFindTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterFindTarget();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Blackboard key to store found target
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Maximum search radius
	 */
	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchRadius = 2000.0f;

	/**
	 * Only search if current target is invalid
	 */
	UPROPERTY(EditAnywhere, Category = "Search")
	bool bOnlySearchIfNoTarget = true;

	/**
	 * Prioritize closest target (otherwise uses threat/perception)
	 */
	UPROPERTY(EditAnywhere, Category = "Search")
	bool bPrioritizeClosest = true;
};
