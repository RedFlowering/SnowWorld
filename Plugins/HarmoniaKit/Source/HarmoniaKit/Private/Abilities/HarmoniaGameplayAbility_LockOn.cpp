// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_LockOn.h"
#include "Components/HarmoniaLockOnComponent.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_LockOn)

UHarmoniaGameplayAbility_LockOn::UHarmoniaGameplayAbility_LockOn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Duration-based ability - stays active while locked on
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// ActivationOwnedTags will be set in Blueprint (e.g., State.LockOn.Active)
	// This replaces hardcoded tag management in component
}

void UHarmoniaGameplayAbility_LockOn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UHarmoniaLockOnComponent* LockOnComp = GetLockOnComponent();
	if (!LockOnComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Try to enable lock-on
	LockOnComp->EnableLockOn();
	
	// If lock-on failed (no valid target), cancel ability
	if (!LockOnComp->IsLockedOn())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Listen for target lost (target died, out of range, etc.)
	LockOnComp->OnLockOnStateChanged.AddDynamic(this, &UHarmoniaGameplayAbility_LockOn::OnTargetLost);
}

void UHarmoniaGameplayAbility_LockOn::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// GAS calls this when input is pressed while ability is already active
	// Second press = toggle off
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_LockOn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Disable lock-on when ability ends
	if (UHarmoniaLockOnComponent* LockOnComp = GetLockOnComponent())
	{
		LockOnComp->OnLockOnStateChanged.RemoveDynamic(this, &UHarmoniaGameplayAbility_LockOn::OnTargetLost);
		LockOnComp->DisableLockOn();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_LockOn::OnTargetLost(bool bIsLockedOn)
{
	// If target was lost, end the ability
	if (!bIsLockedOn)
	{
		K2_EndAbility();
	}
}

UHarmoniaLockOnComponent* UHarmoniaGameplayAbility_LockOn::GetLockOnComponent() const
{
	if (const ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		return Character->FindComponentByClass<UHarmoniaLockOnComponent>();
	}
	return nullptr;
}
