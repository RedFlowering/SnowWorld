// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ContextualAttack.generated.h"

/**
 * UBTTask_ContextualAttack
 *
 * Behavior tree task for context-aware attack selection
 * Uses UHarmoniaAdvancedAIComponent to select attacks based on:
 * - Target health
 * - Tactical position
 * - Emotional state
 * - Combo chains
 *
 * Replaces simple random attack selection with intelligent decision-making
 */
UCLASS()
class HARMONIAKIT_API UBTTask_ContextualAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ContextualAttack();

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
	 * Whether to prioritize combo continuations
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bPrioritizeCombos = true;

	/**
	 * Whether to consider tactical positioning when selecting attacks
	 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bUseTacticalContext = true;
};
