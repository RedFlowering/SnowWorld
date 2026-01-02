// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_SyncAIComponentToBlackboard.generated.h"

class UHarmoniaBaseAIComponent;

/**
 * Behavior Tree Service to sync AI Component state to Blackboard
 * This allows BT to react to AI component state changes
 */
UCLASS(Blueprintable, meta = (DisplayName = "Sync AI Component To Blackboard"))
class HARMONIAKIT_API UBTService_SyncAIComponentToBlackboard : public UBTService_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTService_SyncAIComponentToBlackboard();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** The class of AI component to sync */
	UPROPERTY(EditAnywhere, Category = "AI Component")
	TSubclassOf<UHarmoniaBaseAIComponent> ComponentClass;

	/** Blackboard key to store "Is Component Enabled" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsEnabledKey;

	/** Blackboard key to store "Is In Combat" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsInCombatKey;

	/** Blackboard key to store "Has Valid Target" */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasValidTargetKey;
};


