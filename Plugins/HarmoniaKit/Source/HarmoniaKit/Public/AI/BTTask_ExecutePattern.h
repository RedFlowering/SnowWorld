// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_ExecutePattern.generated.h"

class UHarmoniaMonsterPatternComponent;

/**
 * UBTTask_ExecutePattern
 *
 * Behavior Tree task that executes a named pattern from MonsterPatternComponent
 * 
 * Usage:
 * - Add to behavior tree and specify PatternName
 * - Pattern abilities are defined in MonsterPatternComponent
 * - Task will execute the pattern and optionally wait for completion
 */
UCLASS(Blueprintable, meta = (DisplayName = "Execute Pattern"))
class HARMONIAKIT_API UBTTask_ExecutePattern : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

public:
	UBTTask_ExecutePattern();

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
	 * Name of the pattern to execute (must match a pattern in MonsterPatternComponent)
	 */
	UPROPERTY(EditAnywhere, Category = "Pattern")
	FName PatternName;

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

	/** Time when pattern started */
	float PatternStartTime = 0.0f;

	/** Get or cache the pattern component */
	UHarmoniaMonsterPatternComponent* GetPatternComponent(AActor* Owner);
};


