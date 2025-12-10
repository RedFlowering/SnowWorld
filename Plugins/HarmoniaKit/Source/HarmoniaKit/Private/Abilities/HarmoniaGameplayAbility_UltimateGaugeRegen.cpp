// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_UltimateGaugeRegen.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_UltimateGaugeRegen)

UHarmoniaGameplayAbility_UltimateGaugeRegen::UHarmoniaGameplayAbility_UltimateGaugeRegen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UHarmoniaGameplayAbility_UltimateGaugeRegen::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!UltimateGaugeRegenEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(UltimateGaugeRegenEffectClass, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		ActiveRegenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void UHarmoniaGameplayAbility_UltimateGaugeRegen::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (ActiveRegenEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(ActiveRegenEffectHandle);
		}
		ActiveRegenEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
