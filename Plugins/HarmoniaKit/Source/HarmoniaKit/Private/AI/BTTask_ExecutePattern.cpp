// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_ExecutePattern.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/HarmoniaMonsterPatternComponent.h"

UBTTask_ExecutePattern::UBTTask_ExecutePattern()
{
	NodeName = "Execute Pattern";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ExecutePattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Check if task is enabled
	if (!bTaskEnabled)
	{
		return EBTNodeResult::Succeeded;
	}

	if (PatternName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecutePattern: PatternName is not set!"));
		return EBTNodeResult::Failed;
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
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecutePattern: No MonsterPatternComponent found on %s"), *Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	// Check if pattern is available
	if (!PatternComp->IsPatternAvailable(PatternName))
	{
		// Pattern on cooldown or blocked
		return EBTNodeResult::Failed;
	}

	// Execute pattern
	bool bPatternStarted = PatternComp->ExecutePattern(PatternName);
	if (!bPatternStarted)
	{
		return EBTNodeResult::Failed;
	}

	PatternStartTime = OwnerComp.GetWorld()->GetTimeSeconds();

	// If we should wait for completion, subscribe to completion event
	if (bWaitForCompletion)
	{
		// Note: Dynamic delegates use AddDynamic, handled via polling in TickTask
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UBTTask_ExecutePattern::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecutePattern: Pattern '%s' timed out after %.1f seconds"), 
			*PatternName.ToString(), MaxWaitTime);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Check if pattern completed
	if (CachedPatternComponent.IsValid() && !CachedPatternComponent->IsExecutingPattern())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_ExecutePattern::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Stop current pattern
	if (CachedPatternComponent.IsValid())
	{
		CachedPatternComponent->StopCurrentPattern();
	}

	return EBTNodeResult::Aborted;
}

FString UBTTask_ExecutePattern::GetStaticDescription() const
{
	FString Description = Super::GetStaticDescription();

	if (!PatternName.IsNone())
	{
		Description += FString::Printf(TEXT("\nPattern: %s"), *PatternName.ToString());
	}
	else
	{
		Description += TEXT("\nPattern: None");
	}

	if (bWaitForCompletion)
	{
		Description += TEXT("\nWait for completion");
	}

	return Description;
}

UHarmoniaMonsterPatternComponent* UBTTask_ExecutePattern::GetPatternComponent(AActor* Owner)
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
