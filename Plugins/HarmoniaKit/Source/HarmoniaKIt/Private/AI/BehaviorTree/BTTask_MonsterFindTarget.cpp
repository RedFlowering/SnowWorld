// Copyright 2025 Snow Game Studio.

#include "AI/BehaviorTree/BTTask_MonsterFindTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTTask_MonsterFindTarget::UBTTask_MonsterFindTarget()
{
	NodeName = "Find Target";
	bCreateNodeInstance = false;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterFindTarget, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MonsterFindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	// Check if we already have a valid target and should keep it
	if (bOnlySearchIfNoTarget)
	{
		AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
		if (CurrentTarget && !CurrentTarget->IsPendingKillPending())
		{
			// Check if target implements monster interface and is alive
			if (CurrentTarget->Implements<UHarmoniaMonsterInterface>())
			{
				if (!IHarmoniaMonsterInterface::Execute_IsDead(CurrentTarget))
				{
					return EBTNodeResult::Succeeded;
				}
			}
			else if (APawn* TargetPawn = Cast<APawn>(CurrentTarget))
			{
				if (TargetPawn->GetController())
				{
					return EBTNodeResult::Succeeded;
				}
			}
		}
	}

	// Use perception component to find targets
	UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent();
	if (!PerceptionComp)
	{
		return EBTNodeResult::Failed;
	}

	// Get all perceived actors
	TArray<AActor*> PerceivedActors;
	PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

	// Filter valid targets
	TArray<AActor*> ValidTargets;
	for (AActor* Actor : PerceivedActors)
	{
		if (!Actor || Actor == Monster || Actor->IsPendingKillPending())
		{
			continue;
		}

		// Don't target other monsters
		if (Actor->Implements<UHarmoniaMonsterInterface>())
		{
			continue;
		}

		// Check if it's a valid pawn
		APawn* TargetPawn = Cast<APawn>(Actor);
		if (!TargetPawn || !TargetPawn->GetController())
		{
			continue;
		}

		// Check distance
		float Distance = FVector::Dist(Monster->GetActorLocation(), Actor->GetActorLocation());
		if (Distance <= SearchRadius)
		{
			ValidTargets.Add(Actor);
		}
	}

	// Select best target
	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (AActor* Target : ValidTargets)
	{
		float Score = 0.0f;

		if (bPrioritizeClosest)
		{
			// Score based on distance (closer = higher score)
			float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
			Score = SearchRadius - Distance;
		}
		else
		{
			// Score based on threat/perception
			// This could be extended with a threat table system
			Score = 100.0f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	// Set target in blackboard
	if (BestTarget)
	{
		BlackboardComp->SetValueAsObject(TargetKey.SelectedKeyName, BestTarget);
		IHarmoniaMonsterInterface::Execute_SetCurrentTarget(Monster, BestTarget);
		return EBTNodeResult::Succeeded;
	}

	// No target found, clear blackboard
	BlackboardComp->ClearValue(TargetKey.SelectedKeyName);
	IHarmoniaMonsterInterface::Execute_SetCurrentTarget(Monster, nullptr);
	return EBTNodeResult::Failed;
}

FString UBTTask_MonsterFindTarget::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();
	Description += FString::Printf(TEXT("\nRadius: %.0f"), SearchRadius);

	if (bOnlySearchIfNoTarget)
	{
		Description += TEXT("\nOnly if no target");
	}

	if (bPrioritizeClosest)
	{
		Description += TEXT("\nPriority: Closest");
	}
	else
	{
		Description += TEXT("\nPriority: Threat");
	}

	return Description;
}
