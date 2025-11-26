// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaMovementSystemDefinitions.h"
#include "HarmoniaGameplayAbility_Mount.generated.h"

class UHarmoniaMountComponent;

/**
 * Mount Gameplay Ability
 * Allows character to mount vehicles/creatures
 *
 * Features:
 * - Mount different types of creatures
 * - Mounting animation
 * - Stamina-based riding
 * - Sprint capability
 * - Flying mount support
 *
 * Tag Configuration (use inherited containers):
 * - ActivationOwnedTags: Tags applied while mounting (e.g., State.Mounting)
 * - ActivationBlockedTags: Tags that prevent mounting
 * - BlockAbilitiesWithTag: Abilities blocked while mounting (e.g., State.Combat.Attacking, State.Dodging, State.Mounted)
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Mount : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Mount(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	/** Animation callbacks */
	UFUNCTION()
	void OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageCancelled(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageInterrupted(UAnimMontage* Montage, bool bInterrupted);

protected:
	/** Mount type to use (can be overridden by event data) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mount")
	EHarmoniaMountType DefaultMountType = EHarmoniaMountType::Horse;

	/** Mounting animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mount|Animation")
	TObjectPtr<UAnimMontage> MountMontage = nullptr;

	/** Cached mount component */
	UPROPERTY()
	mutable TObjectPtr<UHarmoniaMountComponent> MountComponent = nullptr;

private:
	UHarmoniaMountComponent* GetMountComponent() const;
};
