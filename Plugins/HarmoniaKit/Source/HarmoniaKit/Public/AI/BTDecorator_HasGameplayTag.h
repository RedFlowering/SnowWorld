// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "BTDecorator_HasGameplayTag.generated.h"

class UAbilitySystemComponent;

/**
 * UBTDecorator_HasGameplayTag
 *
 * Decorator that checks if target has a specific gameplay tag.
 */
UCLASS(meta = (DisplayName = "Target Has Gameplay Tag"))
class HARMONIAKIT_API UBTDecorator_HasGameplayTag : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HasGameplayTag();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Tag to check */
	UPROPERTY(EditAnywhere, Category = "Condition")
	FGameplayTag TagToCheck;

	/** Inverse result (true if tag is NOT present) */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInverseResult = false;

	/** Blackboard key for target actor */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;
};
