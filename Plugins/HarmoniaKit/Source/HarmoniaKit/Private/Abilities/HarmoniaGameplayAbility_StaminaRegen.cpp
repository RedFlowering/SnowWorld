// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_StaminaRegen.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_StaminaRegen)

UHarmoniaGameplayAbility_StaminaRegen::UHarmoniaGameplayAbility_StaminaRegen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Passive ability - activates automatically when granted
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	// This ability should activate immediately when granted
	// Configure via AbilitySet with ActivationPolicy = OnSpawn
}

void UHarmoniaGameplayAbility_StaminaRegen::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!StaminaRegenEffectClass)
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

	// Apply the stamina regen effect
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaRegenEffectClass, GetAbilityLevel(), EffectContext);

    if (SpecHandle.IsValid())
	{
		ActiveRegenEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GA_StaminaRegen] Failed to create SpecHandle!"));
	}

	// This ability stays active indefinitely (passive)
	// It will be ended when the character dies or the ability is removed
}

void UHarmoniaGameplayAbility_StaminaRegen::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Remove the stamina regen effect when ability ends
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
