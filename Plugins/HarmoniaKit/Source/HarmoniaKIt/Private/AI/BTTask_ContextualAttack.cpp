// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_ContextualAttack.h"
#include "AIController.h"
#include "Components/HarmoniaAdvancedAIComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterData.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ContextualAttack::UBTTask_ContextualAttack()
{
	NodeName = "Contextual Attack";
	bNotifyTaskFinished = true;

	bPrioritizeCombos = true;
	bUseTacticalContext = true;
}

EBTNodeResult::Type UBTTask_ContextualAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(ControlledPawn);
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	// Get target from blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Target = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("No target for contextual attack"));
		return EBTNodeResult::Failed;
	}

	// Get advanced AI component
	UHarmoniaAdvancedAIComponent* AdvancedAI = ControlledPawn->FindComponentByClass<UHarmoniaAdvancedAIComponent>();

	// Check for combo continuation first
	if (AdvancedAI && bPrioritizeCombos && AdvancedAI->IsComboActive())
	{
		FName NextAttackID;
		if (AdvancedAI->TryContinueCombo(NextAttackID))
		{
			// Execute combo attack
			bool bSuccess = Monster->ActivateAttackAbility(NextAttackID);
			return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
		}
	}

	// Get available attacks
	if (!Monster->MonsterData || Monster->MonsterData->AttackPatterns.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	FHarmoniaMonsterAttackPattern SelectedAttack;

	// Use contextual selection if advanced AI is available
	if (AdvancedAI)
	{
		// Filter available attacks (not on cooldown, state requirements met, distance)
		TArray<FHarmoniaMonsterAttackPattern> AvailableAttacks;
		for (const FHarmoniaMonsterAttackPattern& Pattern : Monster->MonsterData->AttackPatterns)
		{
			// Check cooldown
			if (Monster->AttackCooldowns.Contains(Pattern.AttackID) && Monster->AttackCooldowns[Pattern.AttackID] > 0.0f)
			{
				continue;
			}

			// Check state
			if (Pattern.RequiredState != EHarmoniaMonsterState::Combat && Monster->CurrentState != Pattern.RequiredState)
			{
				continue;
			}

			// Check distance
			float DistanceToTarget = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
			if (DistanceToTarget < Pattern.MinRange || DistanceToTarget > Pattern.MaxRange)
			{
				continue;
			}

			AvailableAttacks.Add(Pattern);
		}

		if (AvailableAttacks.Num() > 0)
		{
			// Use contextual selection
			SelectedAttack = AdvancedAI->SelectContextualAttack(Target, AvailableAttacks);
		}
		else
		{
			// No available attacks
			return EBTNodeResult::Failed;
		}
	}
	else
	{
		// Fallback to simple random selection
		SelectedAttack = Monster->SelectRandomAttack();
	}

	if (SelectedAttack.AttackID == NAME_None)
	{
		return EBTNodeResult::Failed;
	}

	// Execute attack
	bool bSuccess = Monster->ActivateAttackAbility(SelectedAttack.AttackID);

	// Start combo if applicable
	if (bSuccess && AdvancedAI && SelectedAttack.bCanStartCombo)
	{
		AdvancedAI->StartCombo(SelectedAttack);
	}

	return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

FString UBTTask_ContextualAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Context-aware attack on %s (Combos: %s, Tactical: %s)"),
		*TargetKey.SelectedKeyName.ToString(),
		bPrioritizeCombos ? TEXT("Yes") : TEXT("No"),
		bUseTacticalContext ? TEXT("Yes") : TEXT("No"));
}
