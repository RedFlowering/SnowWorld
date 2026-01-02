// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CircleAroundTarget.generated.h"

/**
 * ECircleDirection
 * 원형 이동 방향
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
 * 타겟 주위를 원형으로 이동하는 BT Task입니다.
 * 보스가 안개 속에서 플레이어를 위협적으로 돌아다니며 혼란을 줄 때 사용합니다.
 *
 * 사용법:
 * - TargetKey: 중심이 될 타겟 (플레이어)
 * - CircleRadius: 원의 반경
 * - Duration: 지속 시간 (0 = 무한)
 */
UCLASS(Blueprintable, meta = (DisplayName = "Circle Around Target"))
class HARMONIAKIT_API UBTTask_CircleAroundTarget : public UBTTaskNode
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

protected:
	// ==========================================================================
	// Configuration
	// ==========================================================================

	/** Task 활성화 여부 */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/** 중심이 될 타겟 (Blackboard 키) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** 원의 반경 */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "100.0"))
	float CircleRadius = 500.0f;

	/** 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "50.0"))
	float MoveSpeed = 400.0f;

	/** 회전 방향 */
	UPROPERTY(EditAnywhere, Category = "Circle")
	ECircleDirection CircleDirection = ECircleDirection::Random;

	/** 지속 시간 (0 = 무한, 외부에서 중단해야 함) */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float Duration = 5.0f;

	/** 타겟 방향을 항상 바라볼지 여부 */
	UPROPERTY(EditAnywhere, Category = "Circle")
	bool bFaceTarget = true;

	/** 반경 유지 오차 허용 범위 */
	UPROPERTY(EditAnywhere, Category = "Circle")
	float RadiusTolerance = 100.0f;

	/** 장애물 회피 활성화 */
	UPROPERTY(EditAnywhere, Category = "Circle|Avoidance")
	bool bAvoidObstacles = true;

	/** 장애물 감지 범위 */
	UPROPERTY(EditAnywhere, Category = "Circle|Avoidance", meta = (EditCondition = "bAvoidObstacles"))
	float ObstacleDetectionRange = 200.0f;

private:
	/** 현재 방향 (1 = 시계, -1 = 반시계) */
	int32 GetActualDirection() const;
};

/**
 * Task 인스턴스 메모리
 */
struct FBTCircleAroundTargetMemory
{
	/** 경과 시간 */
	float ElapsedTime = 0.0f;

	/** 현재 각도 (라디안) */
	float CurrentAngle = 0.0f;

	/** 선택된 방향 (Random일 때 결정됨) */
	int32 SelectedDirection = 1;

	/** 시작 시 초기화됨 */
	bool bInitialized = false;
};
