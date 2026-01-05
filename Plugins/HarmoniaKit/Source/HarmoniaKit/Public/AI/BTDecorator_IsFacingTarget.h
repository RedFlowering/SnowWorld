// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsFacingTarget.generated.h"

/**
 * UBTDecorator_IsFacingTarget
 *
 * Decorator that checks if AI is facing the target within angle tolerance.
 * Gets target automatically from HarmoniaMonsterAIController.
 */
UCLASS(meta = (DisplayName = "Is Facing Target"))
class HARMONIAKIT_API UBTDecorator_IsFacingTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsFacingTarget();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Angle tolerance in degrees (total arc, not half) */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float AngleTolerance = 90.0f;
};
