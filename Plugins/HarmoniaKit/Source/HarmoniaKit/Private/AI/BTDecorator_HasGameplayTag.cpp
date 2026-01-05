// Copyright 2025 Snow Game Studio.

#include "AI/BTDecorator_HasGameplayTag.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UBTDecorator_HasGameplayTag::UBTDecorator_HasGameplayTag()
{
	NodeName = "Target Has Gameplay Tag";
	
	TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_HasGameplayTag, TargetKey), AActor::StaticClass());
}

bool UBTDecorator_HasGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return bInverseResult;
	}

	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!Target)
	{
		return bInverseResult;
	}

	// Get AbilitySystemComponent from target
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);
	if (!ASCInterface)
	{
		return bInverseResult;
	}

	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return bInverseResult;
	}

	bool bHasTag = ASC->HasMatchingGameplayTag(TagToCheck);

	return bInverseResult ? !bHasTag : bHasTag;
}

FString UBTDecorator_HasGameplayTag::GetStaticDescription() const
{
	FString Prefix = bInverseResult ? TEXT("NOT ") : TEXT("");
	return FString::Printf(TEXT("%sHas Tag: %s"), *Prefix, *TagToCheck.ToString());
}
