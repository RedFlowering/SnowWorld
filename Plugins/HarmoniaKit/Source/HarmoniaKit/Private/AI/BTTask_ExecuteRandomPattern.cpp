// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_ExecuteRandomPattern.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/HarmoniaMonsterPatternComponent.h"

UBTTask_ExecuteRandomPattern::UBTTask_ExecuteRandomPattern()
{
	NodeName = "Execute Random Pattern";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;

	// Initialize blackboard key selector
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_ExecuteRandomPattern, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_ExecuteRandomPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	// Get pattern component
	UHarmoniaMonsterPatternComponent* PatternComp = GetPatternComponent(Pawn);
	if (!PatternComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteRandomPattern: No MonsterPatternComponent found on %s"), *Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	// Check if any patterns are available
	TArray<FName> AvailablePatterns = PatternComp->GetAvailablePatterns();
	if (AvailablePatterns.Num() == 0)
	{
		// No patterns available (all on cooldown or blocked)
		return EBTNodeResult::Failed;
	}

	// Execute random pattern
	bool bPatternStarted = PatternComp->ExecuteRandomPattern();
	if (!bPatternStarted)
	{
		return EBTNodeResult::Failed;
	}

	// Store the pattern name for completion check
	CurrentPatternName = PatternComp->GetCurrentPatternName();
	PatternStartTime = OwnerComp.GetWorld()->GetTimeSeconds();

	// If we should wait for completion, use polling in TickTask
	if (bWaitForCompletion)
	{
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UBTTask_ExecuteRandomPattern::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	if (!bWaitForCompletion)
	{
		return;
	}

	// Check timeout
	float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - PatternStartTime;

	if (ElapsedTime >= MaxWaitTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecuteRandomPattern: Pattern timed out after %.1f seconds"), MaxWaitTime);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if pattern completed
	if (CachedPatternComponent.IsValid() && !CachedPatternComponent->IsExecutingPattern())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_ExecuteRandomPattern::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Stop current pattern
	if (CachedPatternComponent.IsValid())
	{
		CachedPatternComponent->StopCurrentPattern();
	}

	return EBTNodeResult::Aborted;
}

FString UBTTask_ExecuteRandomPattern::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();
	Description += TEXT("\nRandom Pattern Selection");

	if (bWaitForCompletion)
	{
		Description += TEXT("\nWait for completion");
	}

	return Description;
}

UHarmoniaMonsterPatternComponent* UBTTask_ExecuteRandomPattern::GetPatternComponent(AActor* Owner)
{
	if (CachedPatternComponent.IsValid())
	{
		return CachedPatternComponent.Get();
	}

	if (Owner)
	{
		CachedPatternComponent = Owner->FindComponentByClass<UHarmoniaMonsterPatternComponent>();
		return CachedPatternComponent.Get();
	}

	return nullptr;
}
