// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaMovementSystemDefinitions.h"
#include "HarmoniaGameplayAbility_Swim.generated.h"

/**
 * Swimming and Diving Gameplay Ability
 * Handles underwater movement and oxygen management
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
