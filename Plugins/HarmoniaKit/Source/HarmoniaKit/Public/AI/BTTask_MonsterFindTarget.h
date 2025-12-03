// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterFindTarget.generated.h"

/**
 * UBTTask_MonsterFindTarget
 *
 * Finds the best target for the monster to attack
 * Uses Sense System for detection and Threat System for prioritization
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
	 * Sensor tag to use for detection (empty = all sensors)
	 */
	UPROPERTY(EditAnywhere, Category = "Sense System")
	FName SensorTag = NAME_None;

	/**
	 * Maximum search radius (0 = use aggro range from MonsterData)
	 */
	UPROPERTY(EditAnywhere, Category = "Sense System")
	float SearchRadius = 0.0f;

	/**
	 * Only search if current target is invalid
	 */
	UPROPERTY(EditAnywhere, Category = "Search")
	bool bOnlySearchIfNoTarget = true;

	/**
	 * Use threat system for prioritization (if available)
	 * Otherwise uses closest target
	 */
	UPROPERTY(EditAnywhere, Category = "Search")
	bool bUseThreatSystem = true;
};
