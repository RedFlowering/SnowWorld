// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_FastTravel.generated.h"

class AHarmoniaWaypointActor;

/**
 * Fast Travel Gameplay Ability
 * Allows instant travel to discovered waypoints
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_FastTravel : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_FastTravel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** Travel animation/effect montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FastTravel")
	TObjectPtr<UAnimMontage> TravelMontage;

	/** Travel effect duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FastTravel")
	float TravelDuration = 2.0f;

	/** Mana/Resource cost for travel */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FastTravel")
	float TravelCost = 50.0f;

private:
	UFUNCTION()
	void CompleteTeleport();

	FTimerHandle TeleportTimerHandle;
	FVector TargetLocation;
	FRotator TargetRotation;
};
