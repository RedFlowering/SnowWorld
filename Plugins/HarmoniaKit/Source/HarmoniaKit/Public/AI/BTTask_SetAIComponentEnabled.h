// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_SetAIComponentEnabled.generated.h"

class UHarmoniaBaseAIComponent;

/**
 * Behavior Tree Task to enable/disable a Harmonia AI Component
 * This allows BT to control when specific AI components are active
 */
UCLASS(Blueprintable, meta = (DisplayName = "Set AI Component Enabled"))
class HARMONIAKIT_API UBTTask_SetAIComponentEnabled : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_SetAIComponentEnabled();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** The class of AI component to enable/disable */
	UPROPERTY(EditAnywhere, Category = "AI Component")
	TSubclassOf<UHarmoniaBaseAIComponent> ComponentClass;

	/** Whether to enable or disable the component */
	UPROPERTY(EditAnywhere, Category = "AI Component")
	bool bEnable = true;
};


