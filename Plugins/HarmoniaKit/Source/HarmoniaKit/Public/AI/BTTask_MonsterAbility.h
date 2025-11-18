// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_MonsterAbility.generated.h"

class UGameplayAbility;

/**
 * UBTTask_MonsterAbility
 *
 * Versatile Behavior Tree task for executing any Gameplay Ability
 * Can be used for attacks, buffs, healing, teleportation, etc.
 *
 * Usage:
 * - Specify Ability Class directly, OR
 * - Use Ability Tags to search for abilities, OR
 * - Leave both empty to execute based on conditions (health, distance, etc.)
 *
 * Features:
 * - Health-based conditions (e.g., use heal at 30% HP)
 * - Distance-based conditions (e.g., charge when far away)
 * - Cooldown checking
 * - Target validation
 */
UCLASS()
class HARMONIAKIT_API UBTTask_MonsterAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MonsterAbility();

	//~UBTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
	//~End of UBTTaskNode interface

protected:
	// ============================================================================
	// Task Control
	// ============================================================================

	/**
	 * Enable/Disable this task
	 */
	UPROPERTY(EditAnywhere, Category = "Task Control")
	bool bTaskEnabled = true;

	// ============================================================================
	// Ability Selection
	// ============================================================================

	/**
	 * Specific ability class to activate
	 * If set, this takes priority over AbilityTags
	 */
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/**
	 * Gameplay tags to search for abilities
	 * Used if AbilityClass is not set
	 * Example: "Ability.Monster.Attack.Charge", "Ability.Monster.Buff.Rage"
	 */
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTagContainer AbilityTags;

	/**
	 * If multiple abilities match the tags, select randomly
	 * Otherwise, select the first match
	 */
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bRandomSelection = false;

	// ============================================================================
	// Conditions
	// ============================================================================

	/**
	 * Only execute if health is within this range (0-1)
	 * Useful for: "Use heal when HP < 30%", "Rage at HP < 50%"
	 */
	UPROPERTY(EditAnywhere, Category = "Conditions|Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinHealthPercent = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Conditions|Health", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxHealthPercent = 1.0f;

	/**
	 * Require a target for this ability
	 */
	UPROPERTY(EditAnywhere, Category = "Conditions|Target")
	bool bRequiresTarget = false;

	/**
	 * Blackboard key for target actor (if bRequiresTarget is true)
	 */
	UPROPERTY(EditAnywhere, Category = "Conditions|Target", meta = (EditCondition = "bRequiresTarget"))
	FBlackboardKeySelector TargetKey;

	/**
	 * Only execute if target is within this distance range
	 * Useful for: "Charge when target > 500 units away"
	 */
	UPROPERTY(EditAnywhere, Category = "Conditions|Distance")
	float MinDistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Conditions|Distance")
	float MaxDistanceToTarget = 99999.0f;

	/**
	 * Check ability cooldown before executing
	 */
	UPROPERTY(EditAnywhere, Category = "Conditions|Cooldown")
	bool bCheckCooldown = true;

	// ============================================================================
	// Execution Options
	// ============================================================================

	/**
	 * Wait for ability to complete before finishing task
	 */
	UPROPERTY(EditAnywhere, Category = "Execution")
	bool bWaitForCompletion = false;

	/**
	 * Maximum time to wait for ability (prevents infinite wait)
	 */
	UPROPERTY(EditAnywhere, Category = "Execution", meta = (EditCondition = "bWaitForCompletion"))
	float MaxWaitTime = 5.0f;

	/**
	 * End ability if task is aborted
	 */
	UPROPERTY(EditAnywhere, Category = "Execution")
	bool bEndAbilityOnAbort = true;

private:
	/**
	 * Find the best ability to activate based on settings
	 */
	TSubclassOf<UGameplayAbility> FindAbilityToActivate(class AHarmoniaMonsterBase* Monster) const;

	/**
	 * Check if conditions are met for ability activation
	 */
	bool CheckConditions(class AHarmoniaMonsterBase* Monster, UBehaviorTreeComponent& OwnerComp) const;

	/**
	 * Validate target distance requirements
	 */
	bool CheckTargetDistance(class AHarmoniaMonsterBase* Monster, AActor* Target) const;
};
