// Copyright 2025 Snow Game Studio.

#include "AI/BehaviorTree/BTService_UpdateMonsterState.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"

UBTService_UpdateMonsterState::UBTService_UpdateMonsterState()
{
	NodeName = "Update Monster State";
	Interval = 0.5f;
	RandomDeviation = 0.1f;

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateMonsterState, TargetKey), AActor::StaticClass());
	DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateMonsterState, DistanceToTargetKey));
	MonsterStateKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_UpdateMonsterState, MonsterStateKey), StaticEnum<EHarmoniaMonsterState>());
}

void UBTService_UpdateMonsterState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	// Update monster state in blackboard
	EHarmoniaMonsterState CurrentState = IHarmoniaMonsterInterface::Execute_GetMonsterState(Monster);
	BlackboardComp->SetValueAsEnum(MonsterStateKey.SelectedKeyName, static_cast<uint8>(CurrentState));

	// Get and validate target
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

	if (Target)
	{
		// Check if target is valid
		bool bTargetValid = true;

		if (Target->IsPendingKillPending())
		{
			bTargetValid = false;
		}
		else if (Target->Implements<UHarmoniaMonsterInterface>())
		{
			if (IHarmoniaMonsterInterface::Execute_IsDead(Target))
			{
				bTargetValid = false;
			}
		}
		else if (APawn* TargetPawn = Cast<APawn>(Target))
		{
			if (!TargetPawn->GetController())
			{
				bTargetValid = false;
			}
		}

		if (bTargetValid)
		{
			// Update distance to target
			float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
			BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
		}
		else if (bClearInvalidTargets)
		{
			// Clear invalid target
			BlackboardComp->ClearValue(TargetKey.SelectedKeyName);
			BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, -1.0f);
			IHarmoniaMonsterInterface::Execute_SetCurrentTarget(Monster, nullptr);
		}
	}
	else
	{
		// No target, set distance to -1
		BlackboardComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, -1.0f);
	}
}

FString UBTService_UpdateMonsterState::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();
	Description += TEXT("\nUpdates: State, Target, Distance");

	if (bClearInvalidTargets)
	{
		Description += TEXT("\nAuto-clear invalid targets");
	}

	return Description;
}
