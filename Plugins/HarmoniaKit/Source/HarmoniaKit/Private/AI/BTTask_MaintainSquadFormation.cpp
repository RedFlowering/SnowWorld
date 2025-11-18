// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_MaintainSquadFormation.h"
#include "AIController.h"
#include "Components/HarmoniaSquadComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MaintainSquadFormation::UBTTask_MaintainSquadFormation()
{
	NodeName = "Maintain Squad Formation";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	AcceptableRadius = 100.0f;
	bUsePathfinding = true;
	MaxExecutionTime = 5.0f;
}

EBTNodeResult::Type UBTTask_MaintainSquadFormation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	// Get squad component
	UHarmoniaSquadComponent* SquadComponent = ControlledPawn->FindComponentByClass<UHarmoniaSquadComponent>();
	if (!SquadComponent || !SquadComponent->bSquadEnabled)
	{
		// No squad component or squad disabled, succeed immediately
		return EBTNodeResult::Succeeded;
	}

	// If this is the squad leader, no need to move
	if (SquadComponent->IsSquadLeader())
	{
		return EBTNodeResult::Succeeded;
	}

	// Get formation position
	FVector FormationPosition = SquadComponent->GetFormationPosition();
	FVector CurrentPosition = ControlledPawn->GetActorLocation();

	// Check if already at formation position
	float Distance = FVector::Dist(CurrentPosition, FormationPosition);
	if (Distance <= AcceptableRadius)
	{
		return EBTNodeResult::Succeeded;
	}

	// Move to formation position
	if (bUsePathfinding)
	{
		EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(
			FormationPosition,
			AcceptableRadius,
			true,  // bStopOnOverlap
			true,  // bUsePathfinding
			false, // bProjectDestinationToNavigation
			true,  // bCanStrafe
			nullptr
		);

		if (Result == EPathFollowingRequestResult::Failed)
		{
			return EBTNodeResult::Failed;
		}
	}
	else
	{
		// Direct movement
		FVector Direction = (FormationPosition - CurrentPosition).GetSafeNormal();
		AIController->SetFocalPoint(FormationPosition);
	}

	ExecutionTime = 0.0f;
	return EBTNodeResult::InProgress;
}

void UBTTask_MaintainSquadFormation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ExecutionTime += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Get squad component
	UHarmoniaSquadComponent* SquadComponent = ControlledPawn->FindComponentByClass<UHarmoniaSquadComponent>();
	if (!SquadComponent)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check timeout
	if (ExecutionTime >= MaxExecutionTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("Squad formation task timed out for %s"), *ControlledPawn->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Get formation position
	FVector FormationPosition = SquadComponent->GetFormationPosition();
	FVector CurrentPosition = ControlledPawn->GetActorLocation();

	// Check if reached formation position
	float Distance = FVector::Dist(CurrentPosition, FormationPosition);
	if (Distance <= AcceptableRadius)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if pathfinding completed
	if (bUsePathfinding)
	{
		EPathFollowingStatus::Type Status = AIController->GetMoveStatus();
		if (Status == EPathFollowingStatus::Idle)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
}

FString UBTTask_MaintainSquadFormation::GetStaticDescription() const
{
	return FString::Printf(TEXT("Move to squad formation position (Radius: %.0f)"), AcceptableRadius);
}
