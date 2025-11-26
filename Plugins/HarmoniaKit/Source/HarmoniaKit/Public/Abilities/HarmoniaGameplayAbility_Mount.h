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
 * ============================================================================
 * Required Tag Configuration (set in Blueprint or derived class):
 * ============================================================================
 *
 * AbilityTags:
 *   - Ability.Movement.Mount (identifies this ability)
 *
 * ActivationOwnedTags (tags applied while mounting):
 *   - State.Mounting
 *
 * ActivationBlockedTags (tags that prevent mounting):
 *   - State.Combat.Attacking
 *   - State.Dodging
 *   - State.Mounted (already mounted)
 *   - State.Swimming
 *   - State.Climbing
 *
 * BlockAbilitiesWithTag (abilities to block while mounting):
 *   - State.Combat.Attacking
 *   - State.Dodging
 *   - State.Mounted
 *
 * CancelAbilitiesWithTag (abilities to cancel when mounting starts):
 *   - (none by default)
 *
 * Related Gameplay Events:
 *   - GameplayEvent.Mount.Mounted (sent on successful mount)
 *   - GameplayEvent.Mount.Dismounted (sent on dismount)
 *
 * Movement Restriction Check:
 *   - Checks for Movement.Restricted.NoMount tag
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
