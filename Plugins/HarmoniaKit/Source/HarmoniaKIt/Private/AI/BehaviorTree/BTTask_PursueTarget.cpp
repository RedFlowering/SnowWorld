// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/BehaviorTree/BTTask_PursueTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AIController.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Components/HarmoniaTerritoryDisputeComponent.h"

UBTTask_PursueTarget::UBTTask_PursueTarget()
{
	NodeName = "Pursue Target";
	bNotifyTick = false;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PursueTarget, TargetKey), AActor::StaticClass());
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

	// Get target to pursue
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

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

	// Move to target
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(Target, AcceptanceRadius);

	// Check if move request was accepted (not failed)
	using namespace EPathFollowingRequestResult;
	if (MoveResult == RequestSuccessful || MoveResult == AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_PursueTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Pursue %s (Max Distance: %.0f)"), *TargetKey.SelectedKeyName.ToString(), MaxPursuitDistance);
}
