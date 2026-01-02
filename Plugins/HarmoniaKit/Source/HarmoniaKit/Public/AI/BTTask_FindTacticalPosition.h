// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "BTTask_FindTacticalPosition.generated.h"

/**
 * UBTTask_FindTacticalPosition
 *
 * Behavior tree task that finds and moves to tactical positions
 * Works with UHarmoniaAdvancedAIComponent to find cover, high ground, etc.
 *
 * Usage:
 * - Add to behavior tree for monsters with advanced AI component
 * - Task will find optimal tactical position and move there
 * - Completes when monster reaches tactical position
 */
UCLASS(Blueprintable, meta = (DisplayName = "Find Tactical Position"))
class HARMONIAKIT_API UBTTask_FindTacticalPosition : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_FindTacticalPosition();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Blackboard key for target actor
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Preferred tactical position type
	 */
	UPROPERTY(EditAnywhere, Category = "Tactical")
	EHarmoniaTacticalPosition PreferredPosition = EHarmoniaTacticalPosition::None;

	/**
	 * Acceptable distance to tactical position
	 */
	UPROPERTY(EditAnywhere, Category = "Tactical", meta = (ClampMin = "50.0"))
	float AcceptableRadius = 150.0f;

	/**
	 * Maximum time to try reaching position
	 */
	UPROPERTY(EditAnywhere, Category = "Tactical", meta = (ClampMin = "1.0"))
	float MaxExecutionTime = 10.0f;

private:
	/**
	 * Time spent executing this task
	 */
	float ExecutionTime = 0.0f;
};


