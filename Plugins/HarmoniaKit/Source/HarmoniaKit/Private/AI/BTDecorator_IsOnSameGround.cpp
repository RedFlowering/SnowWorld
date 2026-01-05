// Copyright 2025 Snow Game Studio.

#include "AI/BTDecorator_IsOnSameGround.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_IsOnSameGround::UBTDecorator_IsOnSameGround()
{
	NodeName = "Is On Same Ground";
	
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_IsOnSameGround, TargetKey), AActor::StaticClass());
}

bool UBTDecorator_IsOnSameGround::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target)
	{
		return false;
	}

	float HeightDifference = FMath::Abs(Target->GetActorLocation().Z - Pawn->GetActorLocation().Z);

	return HeightDifference <= HeightTolerance;
}

FString UBTDecorator_IsOnSameGround::GetStaticDescription() const
{
	return FString::Printf(TEXT("On Same Ground (±%.0f)"), HeightTolerance);
}
