// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FleeFromTarget.generated.h"

/**
 * Behavior Tree task for fleeing from a target
 * Uses TerritoryDisputeComponent to calculate flee location
 */
UCLASS()
class HARMONIAKIT_API UBTTask_FleeFromTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FleeFromTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key for the target to flee from */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** Minimum distance to flee */
	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance = 2000.0f;

	/** Use Territory Dispute Component for flee calculation */
	UPROPERTY(EditAnywhere, Category = "Flee")
	bool bUseTerritoryDisputeComponent = true;
};
