// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PursueTarget.generated.h"

/**
 * Behavior Tree task for pursuing a fleeing target
 * More aggressive than standard MoveTo - ignores acceptance radius until very close
 */
UCLASS(Blueprintable, meta = (DisplayName = "Pursue Target"))
class HARMONIAKIT_API UBTTask_PursueTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PursueTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key for the target to pursue */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** Maximum pursuit distance */
	UPROPERTY(EditAnywhere, Category = "Pursuit")
	float MaxPursuitDistance = 5000.0f;

	/** Use Territory Dispute Component for pursuit logic */
	UPROPERTY(EditAnywhere, Category = "Pursuit")
	bool bUseTerritoryDisputeComponent = true;

	/** Acceptance radius when pursuing */
	UPROPERTY(EditAnywhere, Category = "Pursuit")
	float AcceptanceRadius = 100.0f;
};
