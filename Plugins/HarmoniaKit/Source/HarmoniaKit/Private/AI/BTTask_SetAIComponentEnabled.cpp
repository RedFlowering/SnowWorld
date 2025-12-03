// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/BTTask_SetAIComponentEnabled.h"
#include "AIController.h"
#include "Components/HarmoniaBaseAIComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetAIComponentEnabled::UBTTask_SetAIComponentEnabled()
{
	NodeName = "Set AI Component Enabled";
	bNotifyTick = false;
	bNotifyTaskFinished = false;
}

EBTNodeResult::Type UBTTask_SetAIComponentEnabled::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	// Find the component of the specified class
	UHarmoniaBaseAIComponent* AIComponent = ControlledPawn->FindComponentByClass<UHarmoniaBaseAIComponent>();
	if (!AIComponent || (ComponentClass && !AIComponent->IsA(ComponentClass)))
	{
		// If ComponentClass is specified, find the exact class
		if (ComponentClass)
		{
			TArray<UActorComponent*> Components;
			ControlledPawn->GetComponents(ComponentClass, Components);
			if (Components.Num() > 0)
			{
				AIComponent = Cast<UHarmoniaBaseAIComponent>(Components[0]);
			}
		}
	}

	if (!AIComponent)
	{
		return EBTNodeResult::Failed;
	}

	// Set the component enabled state
	AIComponent->SetAIComponentEnabled(bEnable);

	return EBTNodeResult::Succeeded;
}

FString UBTTask_SetAIComponentEnabled::GetStaticDescription() const
{
	FString ComponentName = ComponentClass ? ComponentClass->GetName() : TEXT("Any AI Component");
	FString Action = bEnable ? TEXT("Enable") : TEXT("Disable");
	return FString::Printf(TEXT("%s %s"), *Action, *ComponentName);
}
