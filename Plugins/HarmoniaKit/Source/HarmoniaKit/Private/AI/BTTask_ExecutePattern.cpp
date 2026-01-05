// Copyright 2025 Snow Game Studio.

#include "AI/BTTask_ExecutePattern.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/HarmoniaMonsterPatternComponent.h"
#include "Monsters/HarmoniaMonsterInterface.h"

UBTTask_ExecutePattern::UBTTask_ExecutePattern()
{
	NodeName = "Execute Pattern";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true;
}

void UBTTask_ExecutePattern::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
}

EBTNodeResult::Type UBTTask_ExecutePattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] ExecutePattern: No MonsterPatternComponent found on %s"), *Pawn->GetName());
		return EBTNodeResult::Failed;
	}

	// If already executing a pattern, wait for it to complete
	if (PatternComp->IsExecutingPattern())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] ExecutePattern: Already executing pattern, waiting..."));
		return EBTNodeResult::InProgress;
	}

	// Get target from monster directly using interface
	AActor* Target = IHarmoniaMonsterInterface::Execute_GetCurrentTarget(Pawn);
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] ExecutePattern: Monster has no current target"));
		return EBTNodeResult::Failed;
	}

	// Execute contextual pattern based on category
	UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] ExecutePattern: Starting pattern, Category=%d, Target=%s"), 
		static_cast<int32>(TargetCategory), *Target->GetName());
	
	bool bPatternStarted = PatternComp->ExecuteContextualPattern(TargetCategory, Target);

	if (!bPatternStarted)
	{
		// No matching pattern found for category and conditions
		UE_LOG(LogTemp, Warning, TEXT("[HARMONIA_AI] ExecutePattern: No pattern found for category %d"), 
			static_cast<int32>(TargetCategory));
		return EBTNodeResult::Failed;
	}

	PatternStartTime = OwnerComp.GetWorld()->GetTimeSeconds();

	// If we should wait for completion, subscribe to completion event
	if (bWaitForCompletion)
	{
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
		UE_LOG(LogTemp, Warning, TEXT("BTTask_ExecutePattern: Category %d timed out after %.1f seconds"), 
			static_cast<int32>(TargetCategory), MaxWaitTime);
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

	FString CategoryName;
	switch (TargetCategory)
	{
		case EPatternCategory::Attack: CategoryName = TEXT("Attack"); break;
		case EPatternCategory::Defense: CategoryName = TEXT("Defense"); break;
		case EPatternCategory::Evasion: CategoryName = TEXT("Evasion"); break;
		case EPatternCategory::Movement: CategoryName = TEXT("Movement"); break;
		default: CategoryName = TEXT("Unknown"); break;
	}

	Description += FString::Printf(TEXT("\nCategory: %s"), *CategoryName);

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
