// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_AsyncMoveTo.h"
#include "AIController.h"
#include "Components/HarmoniaAILODComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

// Memory structure for this task
struct FBTAsyncMoveToMemory
{
	float TimeSinceLastPathUpdate = 0.0f;
	float PathfindingStartTime = 0.0f;
	bool bPathfindingInProgress = false;
};

UBTTask_AsyncMoveTo::UBTTask_AsyncMoveTo()
{
	NodeName = "Async Move To";

	bUseAsyncPathfinding = true;
	bAdaptRadiusToLOD = true;
	LowLODRadiusMultiplier = 2.0f;
	PathfindingTimeout = 2.0f;
	MinPathUpdateInterval = 0.5f;

	// Enable ticking for path updates
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_AsyncMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get memory
	FBTAsyncMoveToMemory* Memory = CastInstanceNodeMemory<FBTAsyncMoveToMemory>(NodeMemory);
	if (Memory)
	{
		Memory->TimeSinceLastPathUpdate = 0.0f;
		Memory->bPathfindingInProgress = false;
	}

	// Apply LOD-based radius adaptation
	if (bAdaptRadiusToLOD && ControlledPawn)
	{
		float AdaptedRadius = GetLODAdaptedRadius(OwnerComp, ControlledPawn);
		if (AdaptedRadius > 0.0f)
		{
			// Replace AcceptableRadius with a new instance containing the adapted value
			AcceptableRadius = FValueOrBBKey_Float(AdaptedRadius);
		}
	}

	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UBTTask_AsyncMoveTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Get memory
	FBTAsyncMoveToMemory* Memory = CastInstanceNodeMemory<FBTAsyncMoveToMemory>(NodeMemory);
	if (!Memory)
	{
		Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
		return;
	}

	Memory->TimeSinceLastPathUpdate += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check pathfinding timeout
	if (Memory->bPathfindingInProgress)
	{
		float PathfindingDuration = Memory->TimeSinceLastPathUpdate - Memory->PathfindingStartTime;
		if (PathfindingDuration > PathfindingTimeout)
		{
			UE_LOG(LogTemp, Warning, TEXT("Async pathfinding timed out"));
			Memory->bPathfindingInProgress = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}

	// Parent tick handles movement completion
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}

FString UBTTask_AsyncMoveTo::GetStaticDescription() const
{
	FString ParentDesc = Super::GetStaticDescription();
	return FString::Printf(TEXT("%s\nAsync: %s, LOD Adapt: %s"),
		*ParentDesc,
		bUseAsyncPathfinding ? TEXT("On") : TEXT("Off"),
		bAdaptRadiusToLOD ? TEXT("On") : TEXT("Off"));
}

// ============================================================================
// Private Functions
// ============================================================================

float UBTTask_AsyncMoveTo::GetLODAdaptedRadius(const UBehaviorTreeComponent& OwnerComp, AActor* Owner) const
{
	if (!Owner || !bAdaptRadiusToLOD)
	{
		return AcceptableRadius.GetValue(OwnerComp);
	}

	UHarmoniaAILODComponent* LODComponent = Owner->FindComponentByClass<UHarmoniaAILODComponent>();
	if (!LODComponent)
	{
		return AcceptableRadius.GetValue(OwnerComp);
	}

	EHarmoniaAILODLevel LODLevel = LODComponent->GetCurrentLODLevel();
	float Multiplier = 1.0f;

	switch (LODLevel)
	{
	case EHarmoniaAILODLevel::VeryHigh:
		Multiplier = 1.0f;
		break;
	case EHarmoniaAILODLevel::High:
		Multiplier = 1.2f;
		break;
	case EHarmoniaAILODLevel::Medium:
		Multiplier = 1.5f;
		break;
	case EHarmoniaAILODLevel::Low:
		Multiplier = LowLODRadiusMultiplier;
		break;
	case EHarmoniaAILODLevel::VeryLow:
		Multiplier = LowLODRadiusMultiplier * 1.5f;
		break;
	default:
		Multiplier = 1.0f;
		break;
	}

	return AcceptableRadius.GetValue(OwnerComp) * Multiplier;
}

bool UBTTask_AsyncMoveTo::ShouldUpdatePath(float TimeSinceLastUpdate) const
{
	return TimeSinceLastUpdate >= MinPathUpdateInterval;
}
