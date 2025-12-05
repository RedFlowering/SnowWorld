// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_SwitchTarget.h"
#include "Components/HarmoniaLockOnComponent.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_SwitchTarget)

UHarmoniaGameplayAbility_SwitchTarget::UHarmoniaGameplayAbility_SwitchTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Instant ability
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// ActivationRequiredTags will be set in Blueprint (e.g., State.LockOn.Active)
	// GAS will automatically block activation if tag is not present
}

void UHarmoniaGameplayAbility_SwitchTarget::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// GAS already checked ActivationRequiredTags, so we know we're locked on
	if (UHarmoniaLockOnComponent* LockOnComp = GetLockOnComponent())
	{
		// Get input axis value from the trigger event
		// Positive value = right, Negative value = left
		float AxisValue = 0.0f;
		
		if (TriggerEventData)
		{
			// EventMagnitude can carry the axis value
			AxisValue = TriggerEventData->EventMagnitude;
		}
		
		if (AxisValue > 0.0f)
		{
			LockOnComp->SwitchTargetRight();
		}
		else if (AxisValue < 0.0f)
		{
			LockOnComp->SwitchTargetLeft();
		}
	}

	// Instant ability - end immediately
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UHarmoniaLockOnComponent* UHarmoniaGameplayAbility_SwitchTarget::GetLockOnComponent() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		return Character->FindComponentByClass<UHarmoniaLockOnComponent>();
	}
	return nullptr;
}
