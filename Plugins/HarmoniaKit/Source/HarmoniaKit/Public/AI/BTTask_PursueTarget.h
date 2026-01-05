// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_PursueTarget.generated.h"

/**
 * Behavior Tree task for pursuing a fleeing target
 * Gets target automatically from HarmoniaMonsterAIController.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Pursue Target"))
class HARMONIAKIT_API UBTTask_PursueTarget : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_PursueTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Called when move request is completed */
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	/** Maximum pursuit distance */
	UPROPERTY(EditAnywhere, Category = "Pursuit")
	float MaxPursuitDistance = 5000.0f;

	/** Use Territory Dispute Component for pursuit logic */
	UPROPERTY(EditAnywhere, Category = "Pursuit")
	bool bUseTerritoryDisputeComponent = true;

	/** Acceptance radius when pursuing */
	UPROPERTY(EditAnywhere, Category = "Pursuit")
	float AcceptanceRadius = 100.0f;

private:
	/** Cached owner component for callback */
	UPROPERTY()
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	/** Cached move request ID */
	FAIRequestID CurrentMoveRequestID;
};
