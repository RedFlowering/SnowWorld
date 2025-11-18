// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_FindTacticalPosition.h"
#include "AIController.h"
#include "Components/HarmoniaAdvancedAIComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindTacticalPosition::UBTTask_FindTacticalPosition()
{
	NodeName = "Find Tactical Position";
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	AcceptableRadius = 150.0f;
	MaxExecutionTime = 10.0f;
}

EBTNodeResult::Type UBTTask_FindTacticalPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get advanced AI component
	UHarmoniaAdvancedAIComponent* AdvancedAI = ControlledPawn->FindComponentByClass<UHarmoniaAdvancedAIComponent>();
	if (!AdvancedAI || !AdvancedAI->bEnableTacticalPositioning)
	{
		// No advanced AI or tactical positioning disabled
		return EBTNodeResult::Failed;
	}

	// Get target from blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Target = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;

	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	// Find tactical position
	bool bFoundPosition = AdvancedAI->FindTacticalPosition(Target, PreferredPosition);
	if (!bFoundPosition)
	{
		return EBTNodeResult::Failed;
	}

	// Get position
	FVector TacticalPosition = AdvancedAI->GetTacticalPositionLocation();

	// Check if already at position
	float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), TacticalPosition);
	if (Distance <= AcceptableRadius)
	{
		return EBTNodeResult::Succeeded;
	}

	// Move to position
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(
		TacticalPosition,
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

	ExecutionTime = 0.0f;
	return EBTNodeResult::InProgress;
}

void UBTTask_FindTacticalPosition::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	// Check timeout
	if (ExecutionTime >= MaxExecutionTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tactical position task timed out for %s"), *ControlledPawn->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Get advanced AI component
	UHarmoniaAdvancedAIComponent* AdvancedAI = ControlledPawn->FindComponentByClass<UHarmoniaAdvancedAIComponent>();
	if (!AdvancedAI)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Get tactical position
	FVector TacticalPosition = AdvancedAI->GetTacticalPositionLocation();
	FVector CurrentPosition = ControlledPawn->GetActorLocation();

	// Check if reached position
	float Distance = FVector::Dist(CurrentPosition, TacticalPosition);
	if (Distance <= AcceptableRadius)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if pathfinding completed
	EPathFollowingStatus::Type Status = AIController->GetMoveStatus();
	if (Status == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

FString UBTTask_FindTacticalPosition::GetStaticDescription() const
{
	FString PositionName = UEnum::GetValueAsString(PreferredPosition);
	return FString::Printf(TEXT("Find and move to %s (Radius: %.0f)"), *PositionName, AcceptableRadius);
}
