// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlueprintBase.h"
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
UCLASS(Blueprintable, meta = (DisplayName = "Monster Distance"))
class HARMONIAKIT_API UBTDecorator_MonsterDistance : public UBTDecorator_BlueprintBase
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
	 * Enable/Disable this decorator
	 * When disabled, always returns true (allows execution)
	 */
	UPROPERTY(EditAnywhere, Category = "Decorator Control")
	bool bDecoratorEnabled = true;

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


