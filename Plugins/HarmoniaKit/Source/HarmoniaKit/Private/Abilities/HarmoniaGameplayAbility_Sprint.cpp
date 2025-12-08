// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Sprint.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AlsCharacter.h"
#include "Utility/AlsGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Sprint)

UHarmoniaGameplayAbility_Sprint::UHarmoniaGameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	RestoreGaitTag = AlsGaitTags::Running;
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

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Check minimum stamina requirement
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet && AttributeSet->GetStamina() < MinStaminaToActivate)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AlsCharacter->SetDesiredGait(AlsGaitTags::Sprinting);

	// Apply stamina cost effect
	if (CostGameplayEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), EffectContext);
		if (SpecHandle.IsValid())
		{
			SprintCostEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Bind to stamina depletion event
	if (AttributeSet)
	{
		UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
		MutableAttributeSet->OnOutOfStamina.AddUObject(this, &UHarmoniaGameplayAbility_Sprint::OnOutOfStamina);
	}
}

void UHarmoniaGameplayAbility_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	// Remove stamina cost effect
	if (ASC && SprintCostEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(SprintCostEffectHandle);
		SprintCostEffectHandle.Invalidate();
	}

	// Unbind stamina delegate
	if (ASC)
	{
		if (const UHarmoniaAttributeSet* HarmoniaSet = ASC->GetSet<UHarmoniaAttributeSet>())
		{
			UHarmoniaAttributeSet* MutableSet = const_cast<UHarmoniaAttributeSet*>(HarmoniaSet);
			MutableSet->OnOutOfStamina.RemoveAll(this);
		}
	}

	// Restore gait
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

void UHarmoniaGameplayAbility_Sprint::OnOutOfStamina(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* EffectSpec, float Magnitude, float OldValue, float NewValue)
{
	K2_EndAbility();
}
