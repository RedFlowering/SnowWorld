// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RetreatFromTarget.generated.h"

/**
 * UBTTask_RetreatFromTarget
 *
 * Behavior Tree task that moves the monster away from its target
 * to create distance. Unlike FleeFromTarget, this task:
 * - Maintains some awareness of the target direction
 * - Moves a configurable distance backward
 * - Completes when the target distance is reached
 */
UCLASS(meta = (DisplayName = "Retreat From Target"))
class HARMONIAKIT_API UBTTask_RetreatFromTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RetreatFromTarget();

	//~UBTAuxiliaryNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTRetreatMemory); }
	//~End of UBTAuxiliaryNode interface

protected:
	/** Distance to retreat from target */
	UPROPERTY(EditAnywhere, Category = "Retreat", meta = (ClampMin = "100.0"))
	float RetreatDistance = 400.0f;

	/** Target distance to maintain from target */
	UPROPERTY(EditAnywhere, Category = "Retreat", meta = (ClampMin = "100.0"))
	float TargetMinDistance = 500.0f;

	/** Acceptable radius to consider retreat complete */
	UPROPERTY(EditAnywhere, Category = "Retreat", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 100.0f;

	/** Maximum time to attempt retreat before giving up */
	UPROPERTY(EditAnywhere, Category = "Retreat", meta = (ClampMin = "0.5"))
	float MaxRetreatTime = 3.0f;

	/** Should face the target while retreating (walk backwards) */
	UPROPERTY(EditAnywhere, Category = "Retreat")
	bool bFaceTargetWhileRetreating = true;

	/** Random lateral angle range (degrees). Retreat direction will be rotated randomly within [-Angle, +Angle] */
	UPROPERTY(EditAnywhere, Category = "Retreat", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float LateralAngleRange = 30.0f;

	/** Helper function to calculate retreat location */
	FVector CalculateRetreatLocation(APawn* Pawn, AActor* Target) const;

	struct FBTRetreatMemory
	{
		float ElapsedTime = 0.0f;
		FVector RetreatLocation = FVector::ZeroVector;
		bool bMoveRequestSent = false;
	};
};
