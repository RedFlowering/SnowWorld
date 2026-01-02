// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterFindTarget.generated.h"

/**
 * UBTTask_MonsterFindTarget
 *
 * Finds the best target for the monster to attack
 * Uses Threat System for prioritization
 */
UCLASS(Blueprintable, meta = (DisplayName = "Monster Find Target"))
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
	 * Enable/Disable this task
	 */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/**
	 * Blackboard key to store found target
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Only search if current target is invalid
	 */
	UPROPERTY(EditAnywhere, Category = "Search")
	bool bOnlySearchIfNoTarget = true;
};
