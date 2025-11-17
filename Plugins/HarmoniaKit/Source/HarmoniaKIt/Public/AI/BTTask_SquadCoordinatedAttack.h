// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SquadCoordinatedAttack.generated.h"

class UHarmoniaSquadComponent;

/**
 * UBTTask_SquadCoordinatedAttack
 *
 * Behavior tree task for coordinated squad attacks
 * Synchronizes attack timing with other squad members
 *
 * Features:
 * - Waits for squad members to be in position
 * - Synchronizes attack timing
 * - Targets shared squad target
 * - Different attack strategies based on role
 */
UCLASS()
class HARMONIAKIT_API UBTTask_SquadCoordinatedAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SquadCoordinatedAttack();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Blackboard key for target actor
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Whether to wait for other squad members before attacking
	 */
	UPROPERTY(EditAnywhere, Category = "Squad")
	bool bWaitForSquad = true;

	/**
	 * Maximum distance squad members can be from target
	 */
	UPROPERTY(EditAnywhere, Category = "Squad", meta = (ClampMin = "100.0"))
	float MaxSquadDistance = 1000.0f;

	/**
	 * Minimum percentage of squad that must be ready (0.0 - 1.0)
	 */
	UPROPERTY(EditAnywhere, Category = "Squad", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumSquadReadyPercent = 0.5f;

private:
	/**
	 * Check if squad is ready for coordinated attack
	 */
	bool IsSquadReady(UHarmoniaSquadComponent* SquadComponent, AActor* Target) const;
};
