// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Components/HarmoniaTerritoryDisputeComponent.h"
#include "BTDecorator_EvaluateCombatAdvantage.generated.h"

/**
 * Decorator that evaluates combat advantage against a target
 * Uses TerritoryDisputeComponent to determine if monster should engage, flee, or pursue
 */
UCLASS()
class HARMONIAKIT_API UBTDecorator_EvaluateCombatAdvantage : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_EvaluateCombatAdvantage();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key for target to evaluate */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** Required combat advantage for this decorator to pass */
	UPROPERTY(EditAnywhere, Category = "Combat")
	EHarmoniaCombatAdvantage RequiredAdvantage = EHarmoniaCombatAdvantage::Even;

	/** If true, passes when advantage is >= RequiredAdvantage. If false, passes when exactly equal */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bCheckGreaterOrEqual = true;

	/** Invert the result */
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bInvertResult = false;
};
