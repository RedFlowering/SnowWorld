// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaMovementSystemDefinitions.h"
#include "HarmoniaGameplayAbility_Climb.generated.h"

/**
 * Climbing Gameplay Ability
 * Allows character to climb walls and surfaces
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.4.1 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Climb : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Climb(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	/** Climbing speed multiplier */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing")
	float ClimbingSpeed = 300.0f;

	/** Stamina cost per second while climbing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing")
	float StaminaCostPerSecond = 10.0f;

	/** Maximum climbing angle (degrees from vertical) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing")
	float MaxClimbingAngle = 15.0f;

	/** Detection range for climbable surfaces */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climbing")
	float DetectionRange = 100.0f;

private:
	/** Check if there's a climbable surface in front */
	bool FindClimbableSurface(const AActor* Avatar, FVector& OutSurfaceNormal) const;

	/** Apply climbing movement */
	void ApplyClimbingMovement(float DeltaTime);
};
