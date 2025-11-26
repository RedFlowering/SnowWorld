// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaMovementSystemDefinitions.h"
#include "HarmoniaGameplayAbility_Swim.generated.h"

/**
 * Swimming and Diving Gameplay Ability
 * Handles underwater movement and oxygen management
 *
 * ============================================================================
 * Required Tag Configuration (set in Blueprint or derived class):
 * ============================================================================
 *
 * AbilityTags:
 *   - Ability.Movement.Swim (identifies this ability)
 *
 * ActivationOwnedTags (tags applied while swimming):
 *   - State.Swimming
 *
 * ActivationBlockedTags (tags that prevent swimming):
 *   - State.Mounted
 *   - State.Climbing
 *   - State.HitReaction
 *
 * BlockAbilitiesWithTag (abilities to block while swimming):
 *   - State.Mounted
 *   - State.Climbing
 *
 * CancelAbilitiesWithTag (abilities to cancel when swimming starts):
 *   - (none by default)
 *
 * Related Gameplay Events:
 *   - GameplayEvent.Swimming.Started (sent on activation)
 *   - GameplayEvent.Swimming.Stopped (sent on end)
 *   - GameplayEvent.Diving.Started (sent when diving)
 *   - GameplayEvent.Oxygen.Depleted (sent when oxygen runs out)
 *
 * Movement Restriction Check:
 *   - Checks for Movement.Restricted.NoSwim tag
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Swim : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Swim(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** Swimming speed multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swimming")
	float SwimmingSpeed = 400.0f;

	/** Diving speed multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swimming")
	float DivingSpeed = 300.0f;

	/** Oxygen capacity (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swimming|Oxygen")
	float OxygenCapacity = 60.0f;

	/** Oxygen consumption rate per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swimming|Oxygen")
	float OxygenConsumptionRate = 1.0f;

	/** Stamina cost per second while swimming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swimming")
	float StaminaCostPerSecond = 5.0f;

	/** Is currently diving */
	UPROPERTY(BlueprintReadOnly, Category = "Swimming")
	bool bIsDiving;

private:
	/** Current oxygen level */
	float CurrentOxygen;

	/** Tick oxygen consumption */
	void TickOxygen(float DeltaTime);

	/** Start diving */
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void StartDiving();

	/** Stop diving */
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void StopDiving();
};
