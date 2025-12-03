// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_MonsterMeleeAttack.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UBTTask_MonsterMeleeAttack::UBTTask_MonsterMeleeAttack()
{
	NodeName = "Monster Melee Attack";
	bNotifyTick = false;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;

	// Accept only actors for target
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterMeleeAttack, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MonsterMeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}

	// Get melee combat component
	UHarmoniaMeleeCombatComponent* MeleeComp = Character->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
	if (!MeleeComp)
	{
		return EBTNodeResult::Failed;
	}

	// Check if can attack
	if (!MeleeComp->CanAttack())
	{
		return EBTNodeResult::Failed;
	}

	// Check stamina
	const bool bIsHeavy = bRandomizeAttackType ? (FMath::FRand() < HeavyAttackChance) : bUseHeavyAttack;
	const float RequiredStamina = bIsHeavy ? MeleeComp->GetHeavyAttackStaminaCost() : MeleeComp->GetLightAttackStaminaCost();

	if (!MeleeComp->HasEnoughStamina(FMath::Max(RequiredStamina, MinimumStamina)))
	{
		return EBTNodeResult::Failed;
	}

	// Perform attack
	bool bAttackStarted = false;
	if (bIsHeavy)
	{
		bAttackStarted = MeleeComp->RequestHeavyAttack();
	}
	else
	{
		bAttackStarted = MeleeComp->RequestLightAttack();
	}

	if (!bAttackStarted)
	{
		return EBTNodeResult::Failed;
	}

	// Wait for completion if requested
	if (bWaitForCompletion)
	{
		UWorld* World = Character->GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				AttackTimerHandle,
				FTimerDelegate::CreateUObject(this, &UBTTask_MonsterMeleeAttack::OnAttackComplete, &OwnerComp),
				MaxWaitTime,
				false
			);
		}

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_MonsterMeleeAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
		if (Character)
		{
			UWorld* World = Character->GetWorld();
			if (World)
			{
				World->GetTimerManager().ClearTimer(AttackTimerHandle);
			}
		}
	}

	return EBTNodeResult::Aborted;
}

FString UBTTask_MonsterMeleeAttack::GetStaticDescription() const
{
	FString Description = FString::Printf(
		TEXT("Monster Melee Attack\nAttack Type: %s\n"),
		bRandomizeAttackType ? TEXT("Random") : (bUseHeavyAttack ? TEXT("Heavy") : TEXT("Light"))
	);

	if (bContinueCombo)
	{
		Description += FString::Printf(TEXT("Combo Chance: %.0f%%\n"), ComboContinuationChance * 100.0f);
	}

	return Description;
}

void UBTTask_MonsterMeleeAttack::OnAttackComplete(UBehaviorTreeComponent* OwnerComp)
{
	if (!OwnerComp)
	{
		return;
	}

	AAIController* AIController = OwnerComp->GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UHarmoniaMeleeCombatComponent* MeleeComp = Character->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
	if (!MeleeComp)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if should continue combo
	if (bContinueCombo && ShouldContinueCombo(MeleeComp))
	{
		MeleeComp->QueueNextCombo();

		// Restart timer for next attack
		UWorld* World = Character->GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				AttackTimerHandle,
				FTimerDelegate::CreateUObject(this, &UBTTask_MonsterMeleeAttack::OnAttackComplete, OwnerComp),
				MaxWaitTime,
				false
			);
		}
		return;
	}

	FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
}

bool UBTTask_MonsterMeleeAttack::ShouldContinueCombo(UHarmoniaMeleeCombatComponent* MeleeComp) const
{
	if (!MeleeComp)
	{
		return false;
	}

	// Check if in combo window
	if (!MeleeComp->IsInComboWindow())
	{
		return false;
	}

	// Check if has more combos available
	if (MeleeComp->GetCurrentComboIndex() >= MeleeComp->GetMaxComboCount() - 1)
	{
		return false;
	}

	// Random chance
	return FMath::FRand() < ComboContinuationChance;
}
