// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Roll.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AlsCharacter.h"
#include "Utility/AlsGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Roll)

UHarmoniaGameplayAbility_Roll::UHarmoniaGameplayAbility_Roll(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHarmoniaGameplayAbility_Roll::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AAlsCharacter* AlsCharacter = GetAlsCharacter();
	if (!AlsCharacter)
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

	// Apply stamina cost if configured
	if (CostGameplayEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(AlsCharacter);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), EffectContext);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Apply I-frame effect if configured
	if (IFrameEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(AlsCharacter);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(IFrameEffectClass, GetAbilityLevel(), EffectContext);
		if (SpecHandle.IsValid())
		{
			IFrameEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// ALS has already started rolling before this event was sent
	// GA just needs to monitor for completion and clean up effects
	
	// Start polling for roll completion
	StartRollCheckTask();
}

void UHarmoniaGameplayAbility_Roll::StartRollCheckTask()
{
	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, RollCheckInterval);
	if (WaitTask)
	{
		WaitTask->OnFinish.AddDynamic(this, &UHarmoniaGameplayAbility_Roll::OnRollCheckTimer);
		WaitTask->ReadyForActivation();
	}
}

void UHarmoniaGameplayAbility_Roll::OnRollCheckTimer()
{
	AAlsCharacter* AlsCharacter = GetAlsCharacter();
	if (!AlsCharacter)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Check if ALS is still in Rolling LocomotionAction
	if (AlsCharacter->GetLocomotionAction() != AlsLocomotionActionTags::Rolling)
	{
		// Rolling completed, end ability
		K2_EndAbility();
		return;
	}

	// Still rolling, continue polling
	StartRollCheckTask();
}

void UHarmoniaGameplayAbility_Roll::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove I-frame effect
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (IFrameEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(IFrameEffectHandle);
			IFrameEffectHandle.Invalidate();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AAlsCharacter* UHarmoniaGameplayAbility_Roll::GetAlsCharacter() const
{
	return Cast<AAlsCharacter>(GetAvatarActorFromActorInfo());
}
