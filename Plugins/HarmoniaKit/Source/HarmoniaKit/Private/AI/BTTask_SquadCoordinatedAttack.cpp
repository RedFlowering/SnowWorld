// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_SquadCoordinatedAttack.h"
#include "AIController.h"
#include "Components/HarmoniaSquadComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SquadCoordinatedAttack::UBTTask_SquadCoordinatedAttack()
{
	NodeName = "Squad Coordinated Attack";
	bNotifyTaskFinished = true;

	bWaitForSquad = true;
	MaxSquadDistance = 1000.0f;
	MinimumSquadReadyPercent = 0.5f;
}

EBTNodeResult::Type UBTTask_SquadCoordinatedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get target from blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Target = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("No target for coordinated attack"));
		return EBTNodeResult::Failed;
	}

	// Get squad component
	UHarmoniaSquadComponent* SquadComponent = ControlledPawn->FindComponentByClass<UHarmoniaSquadComponent>();

	// If no squad or squad disabled, perform normal attack
	if (!SquadComponent || !SquadComponent->bSquadEnabled)
	{
		// Perform individual attack
		AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(ControlledPawn);
		if (Monster)
		{
			FHarmoniaMonsterAttackPattern AttackPattern = Monster->SelectRandomAttack();
			if (AttackPattern.AttackID != NAME_None)
			{
				Monster->ActivateAttackAbility(AttackPattern.AttackID);
				return EBTNodeResult::Succeeded;
			}
		}
		return EBTNodeResult::Failed;
	}

	// Check if squad is ready for coordinated attack
	if (bWaitForSquad && !IsSquadReady(SquadComponent, Target))
	{
		UE_LOG(LogTemp, Log, TEXT("Squad not ready for coordinated attack, performing individual attack"));

		// Perform individual attack anyway
		AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(ControlledPawn);
		if (Monster)
		{
			FHarmoniaMonsterAttackPattern AttackPattern = Monster->SelectRandomAttack();
			if (AttackPattern.AttackID != NAME_None)
			{
				Monster->ActivateAttackAbility(AttackPattern.AttackID);
				return EBTNodeResult::Succeeded;
			}
		}
		return EBTNodeResult::Failed;
	}

	// Set shared target for squad
	SquadComponent->SetSquadTarget(Target);

	// Perform attack based on role
	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(ControlledPawn);
	if (!Monster)
	{
		return EBTNodeResult::Failed;
	}

	// Select attack based on role
	FHarmoniaMonsterAttackPattern AttackPattern = Monster->SelectRandomAttack();

	// Role-based attack modification
	switch (SquadComponent->MyRole)
	{
	case EHarmoniaSquadRole::Tank:
		// Tanks prioritize close-range attacks
		UE_LOG(LogTemp, Log, TEXT("Tank %s performing coordinated attack"), *ControlledPawn->GetName());
		break;

	case EHarmoniaSquadRole::DPS:
		// DPS performs standard attack
		UE_LOG(LogTemp, Log, TEXT("DPS %s performing coordinated attack"), *ControlledPawn->GetName());
		break;

	case EHarmoniaSquadRole::Support:
		// Support might buff allies instead
		UE_LOG(LogTemp, Log, TEXT("Support %s assisting squad"), *ControlledPawn->GetName());
		break;

	case EHarmoniaSquadRole::Leader:
		// Leaders coordinate the attack
		UE_LOG(LogTemp, Log, TEXT("Leader %s initiating coordinated attack"), *ControlledPawn->GetName());
		SquadComponent->RequestCoordinatedAttack(Target);
		break;

	default:
		break;
	}

	// Execute attack
	if (AttackPattern.AttackID != NAME_None)
	{
		bool bSuccess = Monster->ActivateAttackAbility(AttackPattern.AttackID);
		return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	}

	return EBTNodeResult::Failed;
}

FString UBTTask_SquadCoordinatedAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Coordinated squad attack on %s (Wait: %s)"),
		*TargetKey.SelectedKeyName.ToString(),
		bWaitForSquad ? TEXT("Yes") : TEXT("No"));
}

bool UBTTask_SquadCoordinatedAttack::IsSquadReady(UHarmoniaSquadComponent* SquadComponent, AActor* Target) const
{
	if (!SquadComponent || !Target)
	{
		return false;
	}

	// Get all alive squad members
	TArray<AActor*> AliveMembers = SquadComponent->GetAliveSquadMembers();

	if (AliveMembers.Num() == 0)
	{
		return false;
	}

	// Count how many members are in range of target
	int32 ReadyCount = 0;
	FVector TargetLocation = Target->GetActorLocation();

	for (AActor* Member : AliveMembers)
	{
		if (!Member)
		{
			continue;
		}

		float Distance = FVector::Dist(Member->GetActorLocation(), TargetLocation);
		if (Distance <= MaxSquadDistance)
		{
			ReadyCount++;
		}
	}

	// Check if minimum percentage is ready
	float ReadyPercent = (float)ReadyCount / (float)AliveMembers.Num();
	return ReadyPercent >= MinimumSquadReadyPercent;
}
