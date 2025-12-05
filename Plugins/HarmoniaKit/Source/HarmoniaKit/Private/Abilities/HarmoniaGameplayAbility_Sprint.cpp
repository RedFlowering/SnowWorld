// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Sprint.h"
#include "AlsCharacter.h"
#include "Utility/AlsGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Sprint)

UHarmoniaGameplayAbility_Sprint::UHarmoniaGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Duration-based ability - stays active while sprinting
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Default restore gait is Running
	RestoreGaitTag = AlsGaitTags::Running;
	
	// ActivationOwnedTags can be set in Blueprint (e.g., State.Movement.Sprinting)
	// ActivationBlockedTags can block sprint during certain states (e.g., State.Combat.Blocking)
}

void UHarmoniaGameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AAlsCharacter* AlsCharacter = GetAlsCharacter();
	if (!AlsCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Set gait to Sprinting
	AlsCharacter->SetDesiredGait(AlsGaitTags::Sprinting);
}

void UHarmoniaGameplayAbility_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Input released = stop sprinting
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Restore previous gait when sprint ends
	if (AAlsCharacter* AlsCharacter = GetAlsCharacter())
	{
		AlsCharacter->SetDesiredGait(RestoreGaitTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AAlsCharacter* UHarmoniaGameplayAbility_Sprint::GetAlsCharacter() const
{
	return Cast<AAlsCharacter>(GetAvatarActorFromActorInfo());
}
