// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTService_PatternAvailability.generated.h"

/**
 * UBTService_PatternAvailability
 *
 * Service that checks if any attack patterns are available
 * and updates a blackboard bool accordingly.
 * 
 * Use this to:
 * - Block attack branches when all patterns are on cooldown
 * - Trigger retreat/dodge behavior when no attacks are available
 */
UCLASS(meta = (DisplayName = "Pattern Availability Check"))
class HARMONIAKIT_API UBTService_PatternAvailability : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_PatternAvailability();

	//~UBTService interface
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTService interface

protected:
	/**
	 * Blackboard key to store whether any pattern is available
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsPatternAvailableKey;

	/**
	 * Blackboard key to store the shortest remaining cooldown time
	 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ShortestCooldownKey;

	/**
	 * Category to check for available patterns (use -1 for any category)
	 */
	UPROPERTY(EditAnywhere, Category = "Pattern")
	int32 CategoryToCheck = -1;
};
