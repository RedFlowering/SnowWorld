// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_CircleAroundTarget.generated.h"

/**
 * ECircleDirection
 * Circle direction enum
 */
UENUM(BlueprintType)
enum class ECircleDirection : uint8
{
	Clockwise UMETA(DisplayName = "Clockwise"),
	CounterClockwise UMETA(DisplayName = "Counter-Clockwise"),
	Random UMETA(DisplayName = "Random")
};

/**
 * UBTTask_CircleAroundTarget
 *
 * BT Task that circles around a target.
 * Gets target automatically from HarmoniaMonsterAIController.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Circle Around Target"))
class HARMONIAKIT_API UBTTask_CircleAroundTarget : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_CircleAroundTarget();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

	/** Helper to get target from AIController */
	AActor* GetTargetActor(UBehaviorTreeComponent& OwnerComp) const;

protected:
	/** Task enabled */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/** Circle radius */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "100.0"))
	float CircleRadius = 500.0f;

	/** Move speed (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "50.0"))
	float MoveSpeed = 400.0f;

	/** Circle direction */
	UPROPERTY(EditAnywhere, Category = "Circle")
	ECircleDirection CircleDirection = ECircleDirection::Random;

	/** Duration (0 = infinite) */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float Duration = 5.0f;

	/** Face target while circling */
	UPROPERTY(EditAnywhere, Category = "Circle")
	bool bFaceTarget = true;

	/** Radius tolerance */
	UPROPERTY(EditAnywhere, Category = "Circle")
	float RadiusTolerance = 100.0f;

	/** Avoid obstacles */
	UPROPERTY(EditAnywhere, Category = "Circle|Avoidance")
	bool bAvoidObstacles = true;

	/** Obstacle detection range */
	UPROPERTY(EditAnywhere, Category = "Circle|Avoidance", meta = (EditCondition = "bAvoidObstacles"))
	float ObstacleDetectionRange = 200.0f;

private:
	/** Get actual direction (1 = CW, -1 = CCW) */
	int32 GetActualDirection() const;
};

/**
 * Task node memory
 */
struct FBTCircleAroundTargetMemory
{
	/** Elapsed time */
	float ElapsedTime = 0.0f;

	/** Current angle (radians) */
	float CurrentAngle = 0.0f;

	/** Selected direction (for Random) */
	int32 SelectedDirection = 1;

	/** Initialized flag */
	bool bInitialized = false;
};
