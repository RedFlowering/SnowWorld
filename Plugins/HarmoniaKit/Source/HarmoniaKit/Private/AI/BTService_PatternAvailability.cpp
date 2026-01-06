// Copyright 2025 Snow Game Studio.

#include "AI/BTService_PatternAvailability.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Components/HarmoniaMonsterPatternComponent.h"

UBTService_PatternAvailability::UBTService_PatternAvailability()
{
	NodeName = "Pattern Availability Check";
	Interval = 0.25f;
	RandomDeviation = 0.05f;
	bCallTickOnSearchStart = true;

	// Setup blackboard key filters
	IsPatternAvailableKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PatternAvailability, IsPatternAvailableKey));
	ShortestCooldownKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_PatternAvailability, ShortestCooldownKey));
}

void UBTService_PatternAvailability::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		IsPatternAvailableKey.ResolveSelectedKey(*BBAsset);
		ShortestCooldownKey.ResolveSelectedKey(*BBAsset);
	}
}

void UBTService_PatternAvailability::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
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

	UHarmoniaMonsterPatternComponent* PatternComp = Monster->FindComponentByClass<UHarmoniaMonsterPatternComponent>();
	if (!PatternComp)
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	// Check if any pattern is available
	bool bHasAvailable = PatternComp->HasAnyAvailablePattern(CategoryToCheck);
	float ShortestCooldown = PatternComp->GetShortestCooldownRemaining(CategoryToCheck);

	// Update blackboard values
	if (IsPatternAvailableKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(IsPatternAvailableKey.SelectedKeyName, bHasAvailable);
	}

	if (ShortestCooldownKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsFloat(ShortestCooldownKey.SelectedKeyName, ShortestCooldown);
	}
}

FString UBTService_PatternAvailability::GetStaticDescription() const
{
	FString Description = FString::Printf(TEXT("Pattern Availability Check\n"));

	if (CategoryToCheck >= 0)
	{
		Description += FString::Printf(TEXT("Category: %d\n"), CategoryToCheck);
	}
	else
	{
		Description += TEXT("Category: Any\n");
	}

	if (IsPatternAvailableKey.SelectedKeyName != NAME_None)
	{
		Description += FString::Printf(TEXT("Available Key: %s\n"), *IsPatternAvailableKey.SelectedKeyName.ToString());
	}

	if (ShortestCooldownKey.SelectedKeyName != NAME_None)
	{
		Description += FString::Printf(TEXT("Cooldown Key: %s"), *ShortestCooldownKey.SelectedKeyName.ToString());
	}

	return Description;
}
