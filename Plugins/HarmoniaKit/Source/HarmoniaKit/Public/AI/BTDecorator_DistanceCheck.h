// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_DistanceCheck.generated.h"

/**
 * UBTDecorator_DistanceCheck
 *
 * Decorator that checks if target is within specified distance range.
 * Gets target automatically from HarmoniaMonsterAIController.
 */
UCLASS(meta = (DisplayName = "Distance Check"))
class HARMONIAKIT_API UBTDecorator_DistanceCheck : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_DistanceCheck();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Minimum distance (0 = no minimum) */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0"))
	float MinDistance = 0.0f;

	/** Maximum distance (0 = no limit) */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", DisplayName = "Max Distance (0 = No Limit)"))
	float MaxDistance = 0.0f;
};
