// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/BTTask_PursueTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Components/HarmoniaTerritoryDisputeComponent.h"

UBTTask_PursueTarget::UBTTask_PursueTarget()
{
	NodeName = "Pursue Target";
	bNotifyTick = false;
	bCreateNodeInstance = true; // Need instance for move callback
}

EBTNodeResult::Type UBTTask_PursueTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(AIController->GetPawn());
	if (!Monster)
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

	// Check distance
	float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	
	if (Distance > MaxPursuitDistance)
	{
		// Too far to pursue
		if (bUseTerritoryDisputeComponent)
		{
			UHarmoniaTerritoryDisputeComponent* DisputeComp = Monster->FindComponentByClass<UHarmoniaTerritoryDisputeComponent>();
			if (DisputeComp)
			{
				DisputeComp->EndDispute(true);
			}
		}
		return EBTNodeResult::Failed;
	}

	// Notify dispute component
	if (bUseTerritoryDisputeComponent)
	{
		UHarmoniaTerritoryDisputeComponent* DisputeComp = Monster->FindComponentByClass<UHarmoniaTerritoryDisputeComponent>();
		if (DisputeComp)
		{
			DisputeComp->PursueOpponent(Target);
		}
	}

	// Cache owner component for callback
	CachedOwnerComp = &OwnerComp;

	// Bind to move completed delegate
	AIController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_PursueTarget::OnMoveCompleted);

	// Move to target
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(Target, AcceptanceRadius);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_PursueTarget::OnMoveCompleted);
		return EBTNodeResult::Failed;
	}

	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_PursueTarget::OnMoveCompleted);
		return EBTNodeResult::Succeeded;
	}

	// Store request ID
	if (AIController->GetPathFollowingComponent())
	{
		CurrentMoveRequestID = AIController->GetPathFollowingComponent()->GetCurrentRequestId();
	}
	
	// Return InProgress - will complete when OnMoveCompleted is called
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_PursueTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		// Stop movement
		AIController->StopMovement();
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_PursueTarget::OnMoveCompleted);
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_PursueTarget::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (!CachedOwnerComp.IsValid())
	{
		return;
	}

	// Unbind delegate
	AAIController* AIController = CachedOwnerComp->GetAIOwner();
	if (AIController)
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_PursueTarget::OnMoveCompleted);
	}

	// Finish the task
	const bool bSuccess = (Result == EPathFollowingResult::Success);
	FinishLatentTask(*CachedOwnerComp.Get(), bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}

FString UBTTask_PursueTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Pursue Target (Max Distance: %.0f)"), MaxPursuitDistance);
}
