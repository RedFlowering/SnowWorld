// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_MonsterDistance.generated.h"

/**
 * Distance comparison type
 */
UENUM(BlueprintType)
enum class EMonsterDistanceComparison : uint8
{
	LessThan UMETA(DisplayName = "<"),
	LessOrEqual UMETA(DisplayName = "<="),
	Equal UMETA(DisplayName = "="),
	GreaterOrEqual UMETA(DisplayName = ">="),
	GreaterThan UMETA(DisplayName = ">")
};

/**
 * UBTDecorator_MonsterDistance
 *
 * Checks distance between monster and target
 * Useful for attack range checks, retreat conditions, etc.
 */
UCLASS()
class HARMONIAKIT_API UBTDecorator_MonsterDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_MonsterDistance();

protected:
	//~UBTDecorator interface
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTDecorator interface

	/**
	 * Blackboard key for target actor
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * Distance to compare against
	 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	float Distance = 500.0f;

	/**
	 * Comparison operator
	 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	EMonsterDistanceComparison Comparison = EMonsterDistanceComparison::LessThan;

	/**
	 * Use 2D distance (ignore Z axis)
	 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool b2DDistance = true;
};
