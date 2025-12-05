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
	
	// Default direction
	SwitchDirection = ESwitchTargetDirection::Right;
	
	// ActivationRequiredTags will be set in Blueprint (e.g., State.LockOn.Active)
	// GAS will automatically block activation if tag is not present
}

void UHarmoniaGameplayAbility_SwitchTarget::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// GAS already checked ActivationRequiredTags, so we know we're locked on
	if (UHarmoniaLockOnComponent* LockOnComp = GetLockOnComponent())
	{
		// Switch based on configured direction
		switch (SwitchDirection)
		{
		case ESwitchTargetDirection::Left:
			LockOnComp->SwitchTargetLeft();
			break;
		case ESwitchTargetDirection::Right:
			LockOnComp->SwitchTargetRight();
			break;
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
