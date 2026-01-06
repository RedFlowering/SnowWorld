// Copyright 2025 Snow Game Studio.

#include "AI/BTDecorator_IsFacingTarget.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"

UBTDecorator_IsFacingTarget::UBTDecorator_IsFacingTarget()
{
	NodeName = "Is Facing Target";
}

bool UBTDecorator_IsFacingTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

	FVector ToTarget = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal2D();
	FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal2D();
	
	float DotProduct = FVector::DotProduct(Forward, ToTarget);
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
	float HalfTolerance = AngleTolerance / 2.0f;
	
	return AngleDegrees <= HalfTolerance;
}

FString UBTDecorator_IsFacingTarget::GetStaticDescription() const
{
	return FString::Printf(TEXT("Facing Target (±%.0f°)"), AngleTolerance / 2.0f);
}
