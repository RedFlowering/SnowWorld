// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/BTService_SyncAIComponentToBlackboard.h"
#include "AIController.h"
#include "Components/HarmoniaBaseAIComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

UBTService_SyncAIComponentToBlackboard::UBTService_SyncAIComponentToBlackboard()
{
	NodeName = "Sync AI Component To Blackboard";
	Interval = 0.5f; // Update twice per second
	RandomDeviation = 0.1f;

	// Set up default blackboard keys
	IsEnabledKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SyncAIComponentToBlackboard, IsEnabledKey));
	IsInCombatKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SyncAIComponentToBlackboard, IsInCombatKey));
	HasValidTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTService_SyncAIComponentToBlackboard, HasValidTargetKey));
}

void UBTService_SyncAIComponentToBlackboard::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// Find the component of the specified class
	UHarmoniaBaseAIComponent* AIComponent = nullptr;

	if (ComponentClass)
	{
		TArray<UActorComponent*> Components;
		ControlledPawn->GetComponents(ComponentClass, Components);
		if (Components.Num() > 0)
		{
			AIComponent = Cast<UHarmoniaBaseAIComponent>(Components[0]);
		}
	}
	else
	{
		// If no class specified, use the first AI component found
		AIComponent = ControlledPawn->FindComponentByClass<UHarmoniaBaseAIComponent>();
	}

	if (!AIComponent)
	{
		return;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return;
	}

	// Sync component state to blackboard
	if (IsEnabledKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(IsEnabledKey.SelectedKeyName, AIComponent->IsAIComponentEnabled());
	}

	if (IsInCombatKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(IsInCombatKey.SelectedKeyName, AIComponent->IsInCombat());
	}

	if (HasValidTargetKey.SelectedKeyName != NAME_None)
	{
		BlackboardComp->SetValueAsBool(HasValidTargetKey.SelectedKeyName, AIComponent->HasValidTarget());
	}
}

FString UBTService_SyncAIComponentToBlackboard::GetStaticDescription() const
{
	FString ComponentName = ComponentClass ? ComponentClass->GetName() : TEXT("Any AI Component");
	return FString::Printf(TEXT("Sync %s state to blackboard"), *ComponentName);
}
