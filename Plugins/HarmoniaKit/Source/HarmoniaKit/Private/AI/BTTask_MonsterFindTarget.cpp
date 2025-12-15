// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_MonsterFindTarget.h"
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

	// Determine search method - use ThreatComponent
	AActor* BestTarget = nullptr;

	// Use ThreatComponent for target selection
	UHarmoniaThreatComponent* ThreatComp = Monster->FindComponentByClass<UHarmoniaThreatComponent>();
	if (ThreatComp)
	{
		BestTarget = ThreatComp->GetHighestThreatActor();
	}

	// If no threat-based target, use current target from monster
	if (!BestTarget)
	{
		BestTarget = IHarmoniaMonsterInterface::Execute_GetCurrentTarget(Monster);
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

	Description += TEXT("\nUsing Threat System");

	if (bOnlySearchIfNoTarget)
	{
		Description += TEXT("\nOnly if no target");
	}

	return Description;
}
