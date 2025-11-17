// Copyright 2025 Snow Game Studio.

#include "AI/BehaviorTree/BTTask_MonsterFindTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "Components/HarmoniaThreatComponent.h"

UBTTask_MonsterFindTarget::UBTTask_MonsterFindTarget()
{
	NodeName = "Find Target (Sense System)";
	bCreateNodeInstance = false;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterFindTarget, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MonsterFindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Check if task is enabled
	if (!bTaskEnabled)
	{
		return EBTNodeResult::Succeeded;
	}

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

	// Determine search method
	AActor* BestTarget = nullptr;

	if (bUseThreatSystem)
	{
		// Use SelectBestTarget which checks threat system first
		BestTarget = Monster->SelectBestTarget();
	}
	else
	{
		// Use Sense System directly
		TArray<AActor*> SensedTargets = Monster->GetSensedTargets(SensorTag);

		// Filter by distance if specified
		float MaxDistance = SearchRadius;
		if (MaxDistance <= 0.0f)
		{
			// Use aggro range from monster data
			UHarmoniaMonsterData* MonsterData = IHarmoniaMonsterInterface::Execute_GetMonsterData(Monster);
			if (MonsterData)
			{
				MaxDistance = MonsterData->AggroRange;
			}
			else
			{
				MaxDistance = 2000.0f; // Default
			}
		}

		// Find closest valid target within range
		float ClosestDistance = MAX_FLT;
		for (AActor* Target : SensedTargets)
		{
			if (!Target || Target->IsPendingKillPending())
			{
				continue;
			}

			// Don't target other monsters
			if (Target->Implements<UHarmoniaMonsterInterface>())
			{
				continue;
			}

			// Check if it's a valid pawn
			APawn* TargetPawn = Cast<APawn>(Target);
			if (!TargetPawn || !TargetPawn->GetController())
			{
				continue;
			}

			// Check distance
			float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
			if (Distance <= MaxDistance && Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				BestTarget = Target;
			}
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

	if (!bTaskEnabled)
	{
		Description += TEXT(" [DISABLED]");
		return Description;
	}

	if (SensorTag != NAME_None)
	{
		Description += FString::Printf(TEXT("\nSensor: %s"), *SensorTag.ToString());
	}
	else
	{
		Description += TEXT("\nSensor: All");
	}

	if (SearchRadius > 0.0f)
	{
		Description += FString::Printf(TEXT("\nRadius: %.0f"), SearchRadius);
	}
	else
	{
		Description += TEXT("\nRadius: From MonsterData");
	}

	if (bOnlySearchIfNoTarget)
	{
		Description += TEXT("\nOnly if no target");
	}

	if (bUseThreatSystem)
	{
		Description += TEXT("\nPriority: Threat System");
	}
	else
	{
		Description += TEXT("\nPriority: Closest");
	}

	return Description;
}
