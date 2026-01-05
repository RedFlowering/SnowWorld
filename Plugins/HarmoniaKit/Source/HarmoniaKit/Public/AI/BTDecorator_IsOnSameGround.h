// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTDecorator_IsOnSameGround.generated.h"

/**
 * UBTDecorator_IsOnSameGround
 *
 * Decorator that checks if AI and target are on the same ground level.
 */
UCLASS(meta = (DisplayName = "Is On Same Ground"))
class HARMONIAKIT_API UBTDecorator_IsOnSameGround : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsOnSameGround();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Height tolerance to consider "same ground" */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0"))
	float HeightTolerance = 100.0f;

	/** Blackboard key for target actor */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;
};
