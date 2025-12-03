// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "BTTask_MonsterMeleeAttack.generated.h"

/**
 * Monster Melee Attack Task
 * Makes monster perform melee attack using MeleeCombatComponent
 *
 * Features:
 * - Automatic weapon selection based on distance
 * - Light/Heavy attack selection
 * - Combo support
 * - Stamina management
 */
UCLASS()
class HARMONIAKIT_API UBTTask_MonsterMeleeAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterMeleeAttack();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/** Attack type selection */
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bUseHeavyAttack = false;

	/** Randomly choose attack type */
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bRandomizeAttackType = true;

	/** Heavy attack chance when randomizing (0-1) */
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (EditCondition = "bRandomizeAttackType", ClampMin = "0.0", ClampMax = "1.0"))
	float HeavyAttackChance = 0.3f;

	/** Minimum stamina required */
	UPROPERTY(EditAnywhere, Category = "Attack")
	float MinimumStamina = 10.0f;

	/** Wait for attack animation to complete */
	UPROPERTY(EditAnywhere, Category = "Attack")
	bool bWaitForCompletion = true;

	/** Maximum wait time */
	UPROPERTY(EditAnywhere, Category = "Attack", meta = (EditCondition = "bWaitForCompletion"))
	float MaxWaitTime = 2.0f;

	/** Continue combo if available */
	UPROPERTY(EditAnywhere, Category = "Attack|Combo")
	bool bContinueCombo = false;

	/** Combo continuation chance (0-1) */
	UPROPERTY(EditAnywhere, Category = "Attack|Combo", meta = (EditCondition = "bContinueCombo", ClampMin = "0.0", ClampMax = "1.0"))
	float ComboContinuationChance = 0.5f;

	/** Blackboard key for target actor */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

private:
	/** Attack completion timer */
	FTimerHandle AttackTimerHandle;

	/** Called when attack completes */
	void OnAttackComplete(UBehaviorTreeComponent* OwnerComp);

	/** Check if should continue combo */
	bool ShouldContinueCombo(class UHarmoniaMeleeCombatComponent* MeleeComp) const;
};
