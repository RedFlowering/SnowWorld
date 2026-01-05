// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/BTTask_FleeFromTarget.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"
#include "NavigationSystem.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Components/HarmoniaTerritoryDisputeComponent.h"

UBTTask_FleeFromTarget::UBTTask_FleeFromTarget()
{
	NodeName = "Flee From Target";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_FleeFromTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	FVector FleeLocation;

	// Use Territory Dispute Component if available
	if (bUseTerritoryDisputeComponent)
	{
		UHarmoniaTerritoryDisputeComponent* DisputeComp = Monster->FindComponentByClass<UHarmoniaTerritoryDisputeComponent>();
		if (DisputeComp)
		{
			FleeLocation = DisputeComp->GetFleeLocation(Target);
			DisputeComp->FleeFromOpponent(Target);
		}
		else
		{
			// Fallback: calculate flee direction manually
			FVector MyLocation = Monster->GetActorLocation();
			FVector TargetLocation = Target->GetActorLocation();
			FVector FleeDirection = (MyLocation - TargetLocation).GetSafeNormal();
			FleeLocation = MyLocation + FleeDirection * FleeDistance;
		}
	}
	else
	{
		// Manual flee calculation
		FVector MyLocation = Monster->GetActorLocation();
		FVector TargetLocation = Target->GetActorLocation();
		FVector FleeDirection = (MyLocation - TargetLocation).GetSafeNormal();
		FleeLocation = MyLocation + FleeDirection * FleeDistance;
	}

	// Try to find navigable point
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(Monster->GetWorld());
	if (NavSys)
	{
		FNavLocation NavLocation;
		if (NavSys->GetRandomReachablePointInRadius(FleeLocation, 500.0f, NavLocation))
		{
			FleeLocation = NavLocation.Location;
		}
	}

	// Move to flee location
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(FleeLocation, 50.0f);

	// Check if move request was accepted (not failed)
	if (MoveResult != 0)  // 0 = Failed
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_FleeFromTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Flee from Target (Distance: %.0f)"), FleeDistance);
}
