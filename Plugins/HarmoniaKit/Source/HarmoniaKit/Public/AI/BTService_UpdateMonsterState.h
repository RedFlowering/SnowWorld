// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_UpdateMonsterState.generated.h"

/**
 * UBTService_UpdateMonsterState
 *
 * Continuously updates monster state and blackboard values
 * - Checks target validity
 * - Updates distance to target
 * - Updates monster state based on situation
 */
UCLASS(Blueprintable, meta = (DisplayName = "Update Monster State"))
class HARMONIAKIT_API UBTService_UpdateMonsterState : public UBTService_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTService_UpdateMonsterState();

protected:
	//~UBTService interface
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTService interface

	/**
	 * Enable/Disable this service
	 */
	UPROPERTY(EditAnywhere, Category = "Service Control")
	bool bServiceEnabled = true;

	/**
	 * Blackboard key for target actor
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Blackboard key for distance to target
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceToTargetKey;

	/**
	 * Blackboard key for monster state
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MonsterStateKey;

	/**
	 * Whether to clear invalid targets automatically
	 */
	UPROPERTY(EditAnywhere, Category = "Service")
	bool bClearInvalidTargets = true;
};


