// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_RetreatFromTarget.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"

UBTTask_RetreatFromTarget::UBTTask_RetreatFromTarget()
{
	NodeName = "Retreat From Target";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_RetreatFromTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTRetreatMemory* Memory = reinterpret_cast<FBTRetreatMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.0f;
	Memory->bMoveRequestSent = false;

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

	// Get target from AI Controller
	AActor* Target = nullptr;
	AHarmoniaMonsterAIController* MonsterAI = Cast<AHarmoniaMonsterAIController>(AIController);
	if (MonsterAI)
	{
		Target = MonsterAI->GetCurrentTarget();
	}

	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	// Check if already far enough
	float CurrentDistance = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
	if (CurrentDistance >= TargetMinDistance)
	{
		return EBTNodeResult::Succeeded;
	}

	// Calculate retreat location
	Memory->RetreatLocation = CalculateRetreatLocation(Pawn, Target);

	// Move to retreat location
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(
		Memory->RetreatLocation,
		AcceptanceRadius,
		true, // bStopOnOverlap
		true, // bUsePathfinding
		false, // bProjectDestinationToNavigation
		true, // bCanStrafe
		nullptr, // FilterClass
		true // bAllowPartialPath
	);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	Memory->bMoveRequestSent = true;

	// Set focus to target if we want to face it while retreating
	if (bFaceTargetWhileRetreating)
	{
		AIController->SetFocus(Target);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_RetreatFromTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTRetreatMemory* Memory = reinterpret_cast<FBTRetreatMemory*>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;

	// Timeout check
	if (Memory->ElapsedTime >= MaxRetreatTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Get target
	AActor* Target = nullptr;
	AHarmoniaMonsterAIController* MonsterAI = Cast<AHarmoniaMonsterAIController>(AIController);
	if (MonsterAI)
	{
		Target = MonsterAI->GetCurrentTarget();
	}

	if (!Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if we've reached target distance
	float CurrentDistance = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
	if (CurrentDistance >= TargetMinDistance)
	{
		AIController->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if movement is still in progress
	UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent();
	if (PathComp && PathComp->GetStatus() != EPathFollowingStatus::Moving)
	{
		// Movement finished (reached destination or stopped)
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_RetreatFromTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	return EBTNodeResult::Aborted;
}

FVector UBTTask_RetreatFromTarget::CalculateRetreatLocation(APawn* Pawn, AActor* Target) const
{
	FVector MyLocation = Pawn->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// Calculate direction away from target
	FVector RetreatDirection = (MyLocation - TargetLocation).GetSafeNormal2D();

	// If too close to determine direction, use pawn's backward direction
	if (RetreatDirection.IsNearlyZero())
	{
		RetreatDirection = -Pawn->GetActorForwardVector();
	}

	// Apply random lateral angle for more natural movement
	if (LateralAngleRange > 0.0f)
	{
		float RandomAngle = FMath::FRandRange(-LateralAngleRange, LateralAngleRange);
		FRotator Rotation(0.0f, RandomAngle, 0.0f);
		RetreatDirection = Rotation.RotateVector(RetreatDirection);
	}

	// Calculate retreat destination
	FVector RetreatLocation = MyLocation + RetreatDirection * RetreatDistance;

	// Try to find a navigable point
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(Pawn->GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->GetRandomReachablePointInRadius(RetreatLocation, 200.0f, NavLocation))
		{
			return NavLocation.Location;
		}
	}

	return RetreatLocation;
}

FString UBTTask_RetreatFromTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Retreat From Target\nDistance: %.0f\nMin Distance: %.0f\nFace Target: %s"),
		RetreatDistance, TargetMinDistance, bFaceTargetWhileRetreating ? TEXT("Yes") : TEXT("No"));
}
