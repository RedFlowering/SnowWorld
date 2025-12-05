// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_SwitchTarget.generated.h"

class UHarmoniaLockOnComponent;

/** Direction to switch lock-on target */
UENUM(BlueprintType)
enum class ESwitchTargetDirection : uint8
{
	Left,
	Right
};

/**
 * UHarmoniaGameplayAbility_SwitchTarget
 *
 * Gameplay Ability for switching lock-on targets.
 * Wraps UHarmoniaLockOnComponent::SwitchTargetLeft/Right() in a GA.
 *
 * Configuration:
 * - Set SwitchDirection in Blueprint to Left or Right
 * - Create two GA Blueprints from this class:
 *   - GA_SwitchTargetLeft (Direction = Left)
 *   - GA_SwitchTargetRight (Direction = Right)
 *
 * Tag Checking via GAS (NO hardcoding):
 * - Set ActivationRequiredTags in Blueprint (e.g., State.LockOn.Active)
 * - GAS automatically blocks activation if required tags are missing
 *
 * Usage:
 * - Add both GAs to AbilitySet with InputTag.SwitchTargetLeft/Right
 * - Ensure character has UHarmoniaLockOnComponent
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_SwitchTarget : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_SwitchTarget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** Direction to switch target - set in Blueprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SwitchTarget")
	ESwitchTargetDirection SwitchDirection = ESwitchTargetDirection::Right;

	/** Get the LockOn component from the owning actor */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	UHarmoniaLockOnComponent* GetLockOnComponent() const;
};
