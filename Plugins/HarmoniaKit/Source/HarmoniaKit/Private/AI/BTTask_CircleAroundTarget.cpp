// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_CircleAroundTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

UBTTask_CircleAroundTarget::UBTTask_CircleAroundTarget()
{
	NodeName = "Circle Around Target";
	bNotifyTick = true;
	bCreateNodeInstance = true;

	// Blackboard 키 필터
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_CircleAroundTarget, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_CircleAroundTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!bTaskEnabled)
	{
		return EBTNodeResult::Succeeded;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	// 메모리 초기화
	FBTCircleAroundTargetMemory* Memory = reinterpret_cast<FBTCircleAroundTargetMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.0f;
	Memory->bInitialized = true;

	// 현재 위치에서 시작 각도 계산
	FVector ToOwner = Pawn->GetActorLocation() - TargetActor->GetActorLocation();
	Memory->CurrentAngle = FMath::Atan2(ToOwner.Y, ToOwner.X);

	// 방향 결정
	if (CircleDirection == ECircleDirection::Random)
	{
		Memory->SelectedDirection = FMath::RandBool() ? 1 : -1;
	}
	else
	{
		Memory->SelectedDirection = (CircleDirection == ECircleDirection::Clockwise) ? 1 : -1;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_CircleAroundTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTCircleAroundTargetMemory* Memory = reinterpret_cast<FBTCircleAroundTargetMemory*>(NodeMemory);
	if (!Memory || !Memory->bInitialized)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 시간 체크
	Memory->ElapsedTime += DeltaSeconds;
	if (Duration > 0.0f && Memory->ElapsedTime >= Duration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 각도 업데이트 (초당 각속도 계산)
	float AngularVelocity = MoveSpeed / CircleRadius; // rad/s
	Memory->CurrentAngle += AngularVelocity * Memory->SelectedDirection * DeltaSeconds;

	// 목표 위치 계산
	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector DesiredLocation = TargetLocation + FVector(
		FMath::Cos(Memory->CurrentAngle) * CircleRadius,
		FMath::Sin(Memory->CurrentAngle) * CircleRadius,
		0.0f
	);

	// 현재 위치에서 목표까지 방향
	FVector CurrentLocation = Pawn->GetActorLocation();
	FVector MoveDirection = (DesiredLocation - CurrentLocation).GetSafeNormal2D();

	// 장애물 회피
	if (bAvoidObstacles)
	{
		FHitResult Hit;
		FVector TraceStart = CurrentLocation;
		FVector TraceEnd = CurrentLocation + MoveDirection * ObstacleDetectionRange;
		
		if (Pawn->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			// 장애물 감지 시 반대 방향으로
			Memory->SelectedDirection *= -1;
		}
	}

	// 이동 명령
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		Character->AddMovementInput(MoveDirection, 1.0f);

		// 타겟 방향 바라보기
		if (bFaceTarget)
		{
			FVector LookDirection = (TargetLocation - CurrentLocation).GetSafeNormal2D();
			FRotator DesiredRotation = LookDirection.Rotation();
			Character->SetActorRotation(FMath::RInterpTo(
				Character->GetActorRotation(),
				DesiredRotation,
				DeltaSeconds,
				10.0f
			));
		}
	}
	else
	{
		// 이동 명령 (MoveToLocation 사용)
		AIController->MoveToLocation(DesiredLocation, 50.0f, false);
	}
}

EBTNodeResult::Type UBTTask_CircleAroundTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
	}
	return EBTNodeResult::Aborted;
}

uint16 UBTTask_CircleAroundTarget::GetInstanceMemorySize() const
{
	return sizeof(FBTCircleAroundTargetMemory);
}

FString UBTTask_CircleAroundTarget::GetStaticDescription() const
{
	FString DirectionStr;
	switch (CircleDirection)
	{
	case ECircleDirection::Clockwise: DirectionStr = TEXT("CW"); break;
	case ECircleDirection::CounterClockwise: DirectionStr = TEXT("CCW"); break;
	default: DirectionStr = TEXT("Random"); break;
	}

	return FString::Printf(TEXT("Circle around %s\nRadius: %.0f, Speed: %.0f, Dir: %s"),
		*TargetKey.SelectedKeyName.ToString(),
		CircleRadius,
		MoveSpeed,
		*DirectionStr);
}

int32 UBTTask_CircleAroundTarget::GetActualDirection() const
{
	return (CircleDirection == ECircleDirection::Clockwise) ? 1 : -1;
}
