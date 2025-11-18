// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/BTDecorator_EvaluateCombatAdvantage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "AIController.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Components/HarmoniaTerritoryDisputeComponent.h"

UBTDecorator_EvaluateCombatAdvantage::UBTDecorator_EvaluateCombatAdvantage()
{
	NodeName = "Evaluate Combat Advantage";

	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_EvaluateCombatAdvantage, TargetKey), AActor::StaticClass());
}

bool UBTDecorator_EvaluateCombatAdvantage::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	AHarmoniaMonsterBase* Monster = Cast<AHarmoniaMonsterBase>(AIController->GetPawn());
	if (!Monster)
	{
		return false;
	}

	UHarmoniaTerritoryDisputeComponent* DisputeComp = Monster->FindComponentByClass<UHarmoniaTerritoryDisputeComponent>();
	if (!DisputeComp)
	{
		return false;
	}

	// Get target
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!Target)
	{
		return false;
	}

	// Evaluate combat advantage
	EHarmoniaCombatAdvantage CurrentAdvantage = DisputeComp->EvaluateCombatAdvantage(Target);

	bool bResult = false;

	if (bCheckGreaterOrEqual)
	{
		// Pass if current advantage is >= required advantage
		bResult = static_cast<int32>(CurrentAdvantage) >= static_cast<int32>(RequiredAdvantage);
	}
	else
	{
		// Pass if exactly equal
		bResult = CurrentAdvantage == RequiredAdvantage;
	}

	// Invert if needed
	if (bInvertResult)
	{
		bResult = !bResult;
	}

	return bResult;
}

FString UBTDecorator_EvaluateCombatAdvantage::GetStaticDescription() const
{
	FString AdvantageStr;
	switch (RequiredAdvantage)
	{
	case EHarmoniaCombatAdvantage::SignificantDisadvantage:
		AdvantageStr = TEXT("Significant Disadvantage");
		break;
	case EHarmoniaCombatAdvantage::SlightDisadvantage:
		AdvantageStr = TEXT("Slight Disadvantage");
		break;
	case EHarmoniaCombatAdvantage::Even:
		AdvantageStr = TEXT("Even");
		break;
	case EHarmoniaCombatAdvantage::SlightAdvantage:
		AdvantageStr = TEXT("Slight Advantage");
		break;
	case EHarmoniaCombatAdvantage::SignificantAdvantage:
		AdvantageStr = TEXT("Significant Advantage");
		break;
	}

	FString ComparisonStr = bCheckGreaterOrEqual ? TEXT(">=") : TEXT("==");
	FString InvertStr = bInvertResult ? TEXT(" (Inverted)") : TEXT("");

	return FString::Printf(TEXT("%s: %s %s%s"), *TargetKey.SelectedKeyName.ToString(), *ComparisonStr, *AdvantageStr, *InvertStr);
}
