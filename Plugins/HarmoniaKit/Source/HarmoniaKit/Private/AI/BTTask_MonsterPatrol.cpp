// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_MonsterPatrol.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "NavigationSystem.h"

UBTTask_MonsterPatrol::UBTTask_MonsterPatrol()
{
	NodeName = "Monster Patrol";
	bNotifyTaskFinished = true;

	HomeLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterPatrol, HomeLocationKey));
	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterPatrol, PatrolLocationKey));
}

EBTNodeResult::Type UBTTask_MonsterPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Check if task is enabled
	if (!bTaskEnabled)
	{
		return EBTNodeResult::Succeeded;
	}

	AHarmoniaMonsterAIController* AIController = Cast<AHarmoniaMonsterAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	// Get home location
	FVector HomeLocation = BlackboardComp->GetValueAsVector(HomeLocationKey.SelectedKeyName);
	if (HomeLocation.IsZero())
	{
		HomeLocation = AIController->GetHomeLocation();
	}

	// Determine patrol radius
	float PatrolRadius = PatrolRadiusOverride;
	if (PatrolRadius <= 0.0f)
	{
		UHarmoniaMonsterData* MonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(Monster);
		if (MonsterData)
		{
			PatrolRadius = MonsterData->PatrolRadius;
		}
		else
		{
			PatrolRadius = 500.0f; // Default
		}
	}

	// If no patrol radius, just stay at home
	if (PatrolRadius <= 0.0f)
	{
		return EBTNodeResult::Succeeded;
	}

	// Get random patrol location
	FVector PatrolLocation = FVector::ZeroVector;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Monster->GetWorld());

	if (NavSys)
	{
		FNavLocation NavLocation;
		bool bFound = NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, NavLocation);

		if (bFound)
		{
			PatrolLocation = NavLocation.Location;
		}
	}

	// Fallback: random offset from home
	if (PatrolLocation.IsZero())
	{
		FVector RandomOffset = FMath::VRand() * FMath::FRandRange(0.0f, PatrolRadius);
		RandomOffset.Z = 0.0f;
		PatrolLocation = HomeLocation + RandomOffset;
	}

	// Store patrol location in blackboard
	BlackboardComp->SetValueAsVector(PatrolLocationKey.SelectedKeyName, PatrolLocation);

	// Move to patrol location
	bool bSuccess = AIController->MoveToPatrolLocation(PatrolLocation);

	return bSuccess ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_MonsterPatrol::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_MonsterPatrol::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// Optional: Wait at patrol point
	// This would typically be handled by a separate Wait task in the BT
}

FString UBTTask_MonsterPatrol::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();

	if (!bTaskEnabled)
	{
		Description += TEXT("\n[DISABLED]");
		return Description;
	}

	if (PatrolRadiusOverride > 0.0f)
	{
		Description += FString::Printf(TEXT("\nRadius: %.0f"), PatrolRadiusOverride);
	}
	else
	{
		Description += TEXT("\nRadius: From Monster Data");
	}

	if (bWaitAtPatrolPoint)
	{
		Description += FString::Printf(TEXT("\nWait: %.1f-%.1fs"), MinWaitTime, MaxWaitTime);
	}

	return Description;
}
