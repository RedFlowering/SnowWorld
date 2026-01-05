// Copyright 2025 Snow Game Studio.

#include "AI/BTDecorator_HeightCheck.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"

UBTDecorator_HeightCheck::UBTDecorator_HeightCheck()
{
	NodeName = "Height Difference Check";
}

bool UBTDecorator_HeightCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

	// Get target from AI Controller
	AActor* Target = nullptr;
	AHarmoniaMonsterAIController* MonsterAI = Cast<AHarmoniaMonsterAIController>(AIController);
	if (MonsterAI)
	{
		Target = MonsterAI->GetCurrentTarget();
	}

	if (!Target)
	{
		return false;
	}

	float HeightDifference = Target->GetActorLocation().Z - Pawn->GetActorLocation().Z;

	return HeightDifference >= MinHeightDifference && HeightDifference <= MaxHeightDifference;
}

FString UBTDecorator_HeightCheck::GetStaticDescription() const
{
	return FString::Printf(TEXT("Height: %.0f to %.0f"), MinHeightDifference, MaxHeightDifference);
}
