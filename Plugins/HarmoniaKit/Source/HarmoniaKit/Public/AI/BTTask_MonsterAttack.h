// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MonsterAttack.generated.h"

/**
 * UBTTask_MonsterAttack
 *
 * Behavior Tree task for monster attacks
 * Executes Gameplay Ability configured in MonsterData
 *
 * Usage:
 * - Add to BT as a task node
 * - Specify Attack ID or leave empty for random
 * - Attack abilities are configured in MonsterData asset
 */
UCLASS()
class HARMONIAKIT_API UBTTask_MonsterAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterAttack();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Enable/Disable this task
	 */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/**
	 * Specific attack ID to use (leave empty for random selection)
	 * Must match AttackID in MonsterData->AttackPatterns
	 */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FName AttackID = NAME_None;

	/**
	 * Blackboard key for target actor
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Whether to rotate to face target before attacking
	 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bRotateToTarget = true;

	/**
	 * Maximum rotation speed when rotating to target
	 */
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (EditCondition = "bRotateToTarget"))
	float RotationSpeed = 360.0f;

	/**
	 * Acceptable rotation tolerance (degrees)
	 */
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (EditCondition = "bRotateToTarget"))
	float RotationTolerance = 15.0f;

	/**
	 * Wait for attack animation to complete
	 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bWaitForAnimationComplete = true;

	/**
	 * Maximum time to wait for attack (prevents infinite wait)
	 */
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (EditCondition = "bWaitForAnimationComplete"))
	float MaxAttackDuration = 3.0f;

private:
	/**
	 * Time when attack started
	 */
	float AttackStartTime = 0.0f;

	/**
	 * Whether we're currently rotating to target
	 */
	bool bIsRotating = false;
};
