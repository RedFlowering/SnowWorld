// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_CircleAroundTarget.generated.h"

/**
 * ECircleDirection
 * ?í˜• ?´ë™ ë°©í–¥
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
 * ?€ê²?ì£¼ìœ„ë¥??í˜•?¼ë¡œ ?´ë™?˜ëŠ” BT Task?…ë‹ˆ??
 * ë³´ìŠ¤ê°€ ?ˆê°œ ?ì—???Œë ˆ?´ì–´ë¥??„í˜‘?ìœ¼ë¡??Œì•„?¤ë‹ˆë©??¼ë???ì¤????¬ìš©?©ë‹ˆ??
 *
 * ?¬ìš©ë²?
 * - TargetKey: ì¤‘ì‹¬?????€ê²?(?Œë ˆ?´ì–´)
 * - CircleRadius: ?ì˜ ë°˜ê²½
 * - Duration: ì§€???œê°„ (0 = ë¬´í•œ)
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

protected:
	// ==========================================================================
	// Configuration
	// ==========================================================================

	/** Task ?œì„±???¬ë? */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/** ì¤‘ì‹¬?????€ê²?(Blackboard ?? */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** ?ì˜ ë°˜ê²½ */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "100.0"))
	float CircleRadius = 500.0f;

	/** ?´ë™ ?ë„ (cm/s) */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "50.0"))
	float MoveSpeed = 400.0f;

	/** ?Œì „ ë°©í–¥ */
	UPROPERTY(EditAnywhere, Category = "Circle")
	ECircleDirection CircleDirection = ECircleDirection::Random;

	/** ì§€???œê°„ (0 = ë¬´í•œ, ?¸ë??ì„œ ì¤‘ë‹¨?´ì•¼ ?? */
	UPROPERTY(EditAnywhere, Category = "Circle", meta = (ClampMin = "0.0"))
	float Duration = 5.0f;

	/** ?€ê²?ë°©í–¥????ƒ ë°”ë¼ë³¼ì? ?¬ë? */
	UPROPERTY(EditAnywhere, Category = "Circle")
	bool bFaceTarget = true;

	/** ë°˜ê²½ ? ì? ?¤ì°¨ ?ˆìš© ë²”ìœ„ */
	UPROPERTY(EditAnywhere, Category = "Circle")
	float RadiusTolerance = 100.0f;

	/** ?¥ì• ë¬??Œí”¼ ?œì„±??*/
	UPROPERTY(EditAnywhere, Category = "Circle|Avoidance")
	bool bAvoidObstacles = true;

	/** ?¥ì• ë¬?ê°ì? ë²”ìœ„ */
	UPROPERTY(EditAnywhere, Category = "Circle|Avoidance", meta = (EditCondition = "bAvoidObstacles"))
	float ObstacleDetectionRange = 200.0f;

private:
	/** ?„ì¬ ë°©í–¥ (1 = ?œê³„, -1 = ë°˜ì‹œê³? */
	int32 GetActualDirection() const;
};

/**
 * Task ?¸ìŠ¤?´ìŠ¤ ë©”ëª¨ë¦?
 */
struct FBTCircleAroundTargetMemory
{
	/** ê²½ê³¼ ?œê°„ */
	float ElapsedTime = 0.0f;

	/** ?„ì¬ ê°ë„ (?¼ë””?? */
	float CurrentAngle = 0.0f;

	/** ? íƒ??ë°©í–¥ (Random????ê²°ì •?? */
	int32 SelectedDirection = 1;

	/** ?œì‘ ??ì´ˆê¸°?”ë¨ */
	bool bInitialized = false;
};


