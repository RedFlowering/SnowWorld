// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_CircleAroundTarget.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"

UBTTask_CircleAroundTarget::UBTTask_CircleAroundTarget()
{
	NodeName = "Circle Around Target";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

AActor* UBTTask_CircleAroundTarget::GetTargetActor(UBehaviorTreeComponent& OwnerComp) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return nullptr;
	}

	// Get target from AI Controller
	AHarmoniaMonsterAIController* MonsterAI = Cast<AHarmoniaMonsterAIController>(AIController);
	if (MonsterAI)
	{
		return MonsterAI->GetCurrentTarget();
	}

	return nullptr;
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

	AActor* TargetActor = GetTargetActor(OwnerComp);
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	// Initialize memory
	FBTCircleAroundTargetMemory* Memory = reinterpret_cast<FBTCircleAroundTargetMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.0f;
	Memory->bInitialized = true;

	// Calculate starting angle from current position
	FVector ToOwner = Pawn->GetActorLocation() - TargetActor->GetActorLocation();
	Memory->CurrentAngle = FMath::Atan2(ToOwner.Y, ToOwner.X);

	// Determine direction
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

	AActor* TargetActor = GetTargetActor(OwnerComp);
	if (!TargetActor)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check time
	Memory->ElapsedTime += DeltaSeconds;
	if (Duration > 0.0f && Memory->ElapsedTime >= Duration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Update angle (angular velocity calculation)
	float AngularVelocity = MoveSpeed / CircleRadius; // rad/s
	Memory->CurrentAngle += AngularVelocity * Memory->SelectedDirection * DeltaSeconds;

	// Calculate desired location
	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector DesiredLocation = TargetLocation + FVector(
		FMath::Cos(Memory->CurrentAngle) * CircleRadius,
		FMath::Sin(Memory->CurrentAngle) * CircleRadius,
		0.0f
	);

	// Direction from current location to target
	FVector CurrentLocation = Pawn->GetActorLocation();
	FVector MoveDirection = (DesiredLocation - CurrentLocation).GetSafeNormal2D();

	// Obstacle avoidance
	if (bAvoidObstacles)
	{
		FHitResult Hit;
		FVector TraceStart = CurrentLocation;
		FVector TraceEnd = CurrentLocation + MoveDirection * ObstacleDetectionRange;
		
		if (Pawn->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			// Reverse direction on obstacle detection
			Memory->SelectedDirection *= -1;
		}
	}

	// Movement command
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		Character->AddMovementInput(MoveDirection, 1.0f);

		// Face target
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
		// Use MoveToLocation for non-characters
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

	return FString::Printf(TEXT("Circle around Target\nRadius: %.0f, Speed: %.0f, Dir: %s"),
		CircleRadius,
		MoveSpeed,
		*DirectionStr);
}

int32 UBTTask_CircleAroundTarget::GetActualDirection() const
{
	return (CircleDirection == ECircleDirection::Clockwise) ? 1 : -1;
}
