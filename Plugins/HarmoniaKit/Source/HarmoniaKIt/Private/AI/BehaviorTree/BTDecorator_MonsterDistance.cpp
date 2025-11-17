// Copyright 2025 Snow Game Studio.

#include "AI/BehaviorTree/BTDecorator_MonsterDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UBTDecorator_MonsterDistance::UBTDecorator_MonsterDistance()
{
	NodeName = "Monster Distance Check";

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_MonsterDistance, TargetKey), AActor::StaticClass());
	TargetKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_MonsterDistance, TargetKey));
}

bool UBTDecorator_MonsterDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// If decorator is disabled, always return true (allow execution)
	if (!bDecoratorEnabled)
	{
		return true;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* Monster = AIController->GetPawn();
	if (!Monster)
	{
		return false;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	// Get target position
	FVector TargetLocation = FVector::ZeroVector;
	bool bHasTarget = false;

	// Try to get as object (actor) first
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (TargetActor)
	{
		TargetLocation = TargetActor->GetActorLocation();
		bHasTarget = true;
	}
	else
	{
		// Try to get as vector
		TargetLocation = BlackboardComp->GetValueAsVector(TargetKey.SelectedKeyName);
		if (!TargetLocation.IsZero())
		{
			bHasTarget = true;
		}
	}

	if (!bHasTarget)
	{
		return false;
	}

	// Calculate distance
	FVector MonsterLocation = Monster->GetActorLocation();
	float ActualDistance;

	if (b2DDistance)
	{
		FVector MonsterLoc2D = FVector(MonsterLocation.X, MonsterLocation.Y, 0.0f);
		FVector TargetLoc2D = FVector(TargetLocation.X, TargetLocation.Y, 0.0f);
		ActualDistance = FVector::Dist(MonsterLoc2D, TargetLoc2D);
	}
	else
	{
		ActualDistance = FVector::Dist(MonsterLocation, TargetLocation);
	}

	// Compare distance based on comparison type
	switch (Comparison)
	{
	case EMonsterDistanceComparison::LessThan:
		return ActualDistance < Distance;

	case EMonsterDistanceComparison::LessOrEqual:
		return ActualDistance <= Distance;

	case EMonsterDistanceComparison::Equal:
		return FMath::IsNearlyEqual(ActualDistance, Distance, 10.0f);

	case EMonsterDistanceComparison::GreaterOrEqual:
		return ActualDistance >= Distance;

	case EMonsterDistanceComparison::GreaterThan:
		return ActualDistance > Distance;

	default:
		return false;
	}
}

FString UBTDecorator_MonsterDistance::GetStaticDescription() const
{
	if (!bDecoratorEnabled)
	{
		return TEXT("[DISABLED] Always allows execution");
	}

	FString ComparisonStr;
	switch (Comparison)
	{
	case EMonsterDistanceComparison::LessThan:
		ComparisonStr = TEXT("<");
		break;
	case EMonsterDistanceComparison::LessOrEqual:
		ComparisonStr = TEXT("<=");
		break;
	case EMonsterDistanceComparison::Equal:
		ComparisonStr = TEXT("=");
		break;
	case EMonsterDistanceComparison::GreaterOrEqual:
		ComparisonStr = TEXT(">=");
		break;
	case EMonsterDistanceComparison::GreaterThan:
		ComparisonStr = TEXT(">");
		break;
	}

	return FString::Printf(TEXT("Distance %s %.0f%s"),
		*ComparisonStr,
		Distance,
		b2DDistance ? TEXT(" (2D)") : TEXT(""));
}
