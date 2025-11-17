// Copyright 2025 Snow Game Studio.

#include "AI/BehaviorTree/BTTask_MonsterAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "GameFramework/Character.h"

UBTTask_MonsterAttack::UBTTask_MonsterAttack()
{
	NodeName = "Monster Attack (GAS)";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;

	// Initialize blackboard key selector
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MonsterAttack, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	// Get target from blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = nullptr;

	if (BlackboardComp && TargetKey.SelectedKeyName != NAME_None)
	{
		TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	}

	// Check if we need to rotate to target
	if (bRotateToTarget && TargetActor)
	{
		FVector DirectionToTarget = TargetActor->GetActorLocation() - Monster->GetActorLocation();
		DirectionToTarget.Z = 0.0f;
		DirectionToTarget.Normalize();

		FVector CurrentForward = Monster->GetActorForwardVector();
		float DotProduct = FVector::DotProduct(CurrentForward, DirectionToTarget);
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		if (AngleDegrees > RotationTolerance)
		{
			bIsRotating = true;
			// Will continue rotating in TickTask
			return EBTNodeResult::InProgress;
		}
	}

	// Activate attack ability via GAS
	bool bAttackStarted = Monster->ActivateAttackAbility(AttackID);
	if (!bAttackStarted)
	{
		return EBTNodeResult::Failed;
	}

	AttackStartTime = OwnerComp.GetWorld()->GetTimeSeconds();

	// If we should wait for animation, return InProgress
	if (bWaitForAnimationComplete)
	{
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UBTTask_MonsterAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Handle rotation to target
	if (bIsRotating)
	{
		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
		AActor* TargetActor = nullptr;

		if (BlackboardComp && TargetKey.SelectedKeyName != NAME_None)
		{
			TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
		}

		if (!TargetActor)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		// Rotate towards target
		FVector DirectionToTarget = TargetActor->GetActorLocation() - Monster->GetActorLocation();
		DirectionToTarget.Z = 0.0f;
		DirectionToTarget.Normalize();

		FRotator TargetRotation = DirectionToTarget.Rotation();
		FRotator CurrentRotation = Monster->GetActorRotation();
		FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed);
		Monster->SetActorRotation(NewRotation);

		// Check if rotation is complete
		FVector CurrentForward = Monster->GetActorForwardVector();
		float DotProduct = FVector::DotProduct(CurrentForward, DirectionToTarget);
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		if (AngleDegrees <= RotationTolerance)
		{
			bIsRotating = false;

			// Now execute the attack
			bool bAttackStarted = Monster->ActivateAttackAbility(AttackID);
			if (!bAttackStarted)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}

			AttackStartTime = OwnerComp.GetWorld()->GetTimeSeconds();

			if (!bWaitForAnimationComplete)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}
		}

		return;
	}

	// Wait for attack to complete
	if (bWaitForAnimationComplete)
	{
		float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime - AttackStartTime;

		// Check if attack animation is still playing
		UAnimInstance* AnimInstance = Monster->GetMesh()->GetAnimInstance();
		if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
		{
			// Still playing, but check timeout
			if (ElapsedTime >= MaxAttackDuration)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
		else
		{
			// Animation finished
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

FString UBTTask_MonsterAttack::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();

	if (AttackID != NAME_None)
	{
		Description += FString::Printf(TEXT("\nAttack: %s"), *AttackID.ToString());
	}
	else
	{
		Description += TEXT("\nAttack: Random");
	}

	if (bRotateToTarget)
	{
		Description += TEXT("\nRotate to target");
	}

	return Description;
}
