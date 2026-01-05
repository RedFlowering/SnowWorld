// Copyright 2025 Snow Game Studio.

#include "AI/BTDecorator_DistanceCheck.h"
#include "AIController.h"
#include "AI/HarmoniaMonsterAIController.h"

UBTDecorator_DistanceCheck::UBTDecorator_DistanceCheck()
{
	NodeName = "Distance Check";
}

bool UBTDecorator_DistanceCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] DistanceCheck: No AIController"));
		return false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] DistanceCheck: No Pawn"));
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
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] DistanceCheck: No Target"));
		return false;
	}

	float Distance = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());

	// Check minimum distance
	bool bPassedMin = (MinDistance <= 0.0f || Distance >= MinDistance);
	bool bPassedMax = (MaxDistance <= 0.0f || Distance <= MaxDistance);
	bool bResult = bPassedMin && bPassedMax;

	UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] DistanceCheck: Distance=%.1f, Range=[%.0f-%.0f], PassMin=%d, PassMax=%d, Result=%d"), 
		Distance, MinDistance, MaxDistance, bPassedMin, bPassedMax, bResult);

	return bResult;
}

FString UBTDecorator_DistanceCheck::GetStaticDescription() const
{
	FString Description;

	if (MinDistance > 0.0f && MaxDistance > 0.0f)
	{
		Description = FString::Printf(TEXT("Distance: %.0f - %.0f"), MinDistance, MaxDistance);
	}
	else if (MinDistance > 0.0f)
	{
		Description = FString::Printf(TEXT("Distance >= %.0f"), MinDistance);
	}
	else if (MaxDistance > 0.0f)
	{
		Description = FString::Printf(TEXT("Distance <= %.0f"), MaxDistance);
	}
	else
	{
		Description = TEXT("Distance: Any");
	}

	return Description;
}
