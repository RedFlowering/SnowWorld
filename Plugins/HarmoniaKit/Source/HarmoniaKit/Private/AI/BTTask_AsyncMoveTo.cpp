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

	// TODO: LOD-based radius adaptation cannot be implemented directly here because:
	// - AcceptableRadius is FValueOrBBKey_Float with protected DefaultValue
	// - GetValue() requires UBlackboardComponent* parameter
	// - Cannot modify parent class's protected member in a clean way
	//
	// Potential solutions for future:
	// 1. Implement custom MoveTo task from scratch (not inheriting from UBTTask_MoveTo)
	// 2. Use decorator node to modify acceptable radius via blackboard
	// 3. Wait for Epic to expose this functionality in future engine versions
	//
	// For now, bAdaptRadiusToLOD property exists but has no effect.
	// Developers should manually adjust AcceptableRadius in the behavior tree editor
	// based on their AI's typical LOD levels.

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

float UBTTask_AsyncMoveTo::GetLODAdaptedRadius(AActor* Owner) const
{
	// Note: This function cannot be properly implemented due to UBTTask_MoveTo limitations
	// - AcceptableRadius is FValueOrBBKey_Float with protected DefaultValue member
	// - GetValue() requires UBlackboardComponent* parameter which isn't available here
	// - Cannot cleanly modify parent class's protected property
	//
	// This function is kept for future use if Epic exposes the necessary API
	// or if we reimplement this task without inheriting from UBTTask_MoveTo
	return 0.0f;

	// Desired implementation (currently not possible):
	// if (!Owner || !bAdaptRadiusToLOD)
	// {
	// 	return base acceptable radius;
	// }
	//
	// UHarmoniaAILODComponent* LODComponent = Owner->FindComponentByClass<UHarmoniaAILODComponent>();
	// if (!LODComponent)
	// {
	// 	return base acceptable radius;
	// }
	//
	// EHarmoniaAILODLevel LODLevel = LODComponent->GetCurrentLODLevel();
	// float Multiplier = 1.0f;
	//
	// switch (LODLevel)
	// {
	// case EHarmoniaAILODLevel::VeryHigh:
	// 	Multiplier = 1.0f;
	// 	break;
	// case EHarmoniaAILODLevel::High:
	// 	Multiplier = 1.2f;
	// 	break;
	// case EHarmoniaAILODLevel::Medium:
	// 	Multiplier = 1.5f;
	// 	break;
	// case EHarmoniaAILODLevel::Low:
	// 	Multiplier = LowLODRadiusMultiplier;
	// 	break;
	// case EHarmoniaAILODLevel::VeryLow:
	// 	Multiplier = LowLODRadiusMultiplier * 1.5f;
	// 	break;
	// default:
	// 	Multiplier = 1.0f;
	// 	break;
	// }
	//
	// return base acceptable radius * Multiplier;
}

bool UBTTask_AsyncMoveTo::ShouldUpdatePath(float TimeSinceLastUpdate) const
{
	return TimeSinceLastUpdate >= MinPathUpdateInterval;
}
