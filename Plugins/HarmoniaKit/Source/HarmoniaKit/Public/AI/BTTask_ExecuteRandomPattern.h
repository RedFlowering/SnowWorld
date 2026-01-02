// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_ExecuteRandomPattern.generated.h"

class UHarmoniaMonsterPatternComponent;

/**
 * UBTTask_ExecuteRandomPattern
 *
 * Behavior Tree task that executes a random available pattern
 * 
 * Usage:
 * - Add to behavior tree
 * - Task will select a random pattern from available patterns (not on cooldown)
 * - Pattern selection uses weight system defined in pattern data
 */
UCLASS(Blueprintable, meta = (DisplayName = "Execute Random Pattern"))
class HARMONIAKIT_API UBTTask_ExecuteRandomPattern : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_ExecuteRandomPattern();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	/**
	 * Enable/Disable this task
	 */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	/**
	 * Wait for pattern completion before succeeding
	 */
	UPROPERTY(EditAnywhere, Category = "Pattern")
	bool bWaitForCompletion = true;

	/**
	 * Maximum time to wait for pattern (prevents infinite wait)
	 */
	UPROPERTY(EditAnywhere, Category = "Pattern", meta = (EditCondition = "bWaitForCompletion", ClampMin = "1.0", ClampMax = "60.0"))
	float MaxWaitTime = 10.0f;

private:
	/** Cached pattern component */
	UPROPERTY()
	TWeakObjectPtr<UHarmoniaMonsterPatternComponent> CachedPatternComponent;

	/** Currently executing pattern name */
	FName CurrentPatternName;

	/** Time when pattern started */
	float PatternStartTime = 0.0f;

	/** Get or cache the pattern component */
	UHarmoniaMonsterPatternComponent* GetPatternComponent(AActor* Owner);
};


