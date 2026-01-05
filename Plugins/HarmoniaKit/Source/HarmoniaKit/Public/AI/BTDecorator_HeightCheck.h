// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HeightCheck.generated.h"

/**
 * UBTDecorator_HeightCheck
 *
 * Decorator that checks if target is within specified height difference range.
 * Positive = target above, Negative = target below
 * Gets target automatically from HarmoniaMonsterAIController.
 */
UCLASS(meta = (DisplayName = "Height Difference Check"))
class HARMONIAKIT_API UBTDecorator_HeightCheck : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HeightCheck();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Minimum height difference (negative = target below) */
	UPROPERTY(EditAnywhere, Category = "Condition")
	float MinHeightDifference = -10000.0f;

	/** Maximum height difference (positive = target above) */
	UPROPERTY(EditAnywhere, Category = "Condition")
	float MaxHeightDifference = 10000.0f;
};
