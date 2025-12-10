// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Mantle.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Core/HarmoniaCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Mantle)

UHarmoniaGameplayAbility_Mantle::UHarmoniaGameplayAbility_Mantle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHarmoniaGameplayAbility_Mantle::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!GetHarmoniaCharacter())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	StartMantleCheckTask();
}

void UHarmoniaGameplayAbility_Mantle::StartMantleCheckTask()
{
	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, MantleCheckInterval);
	if (WaitTask)
	{
		WaitTask->OnFinish.AddDynamic(this, &UHarmoniaGameplayAbility_Mantle::OnMantleCheckTimer);
		WaitTask->ReadyForActivation();
	}
}

void UHarmoniaGameplayAbility_Mantle::OnMantleCheckTimer()
{
	AHarmoniaCharacter* HarmoniaCharacter = GetHarmoniaCharacter();
	if (!HarmoniaCharacter)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UCharacterMovementComponent* CMC = HarmoniaCharacter->GetCharacterMovement();
	if (!CMC)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (bIsMantling)
	{
		if (CMC->MovementMode != MOVE_Custom)
		{
			CancelMantleExecuteAbility();
			bIsMantling = false;
		}
		StartMantleCheckTask();
		return;
	}

	if (!CMC->IsFalling())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (HarmoniaCharacter->TryMantleFromGA())
	{
		bIsMantling = true;
		
		if (MantleExecuteEventTag.IsValid())
		{
			if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
			{
				FGameplayEventData EventData;
				EventData.Instigator = HarmoniaCharacter;
				EventData.Target = HarmoniaCharacter;
				ASC->HandleGameplayEvent(MantleExecuteEventTag, &EventData);
			}
		}
	}

	StartMantleCheckTask();
}

bool UHarmoniaGameplayAbility_Mantle::CanBeCanceled() const
{
	if (bIsMantling)
	{
		return false;
	}
	return Super::CanBeCanceled();
}

void UHarmoniaGameplayAbility_Mantle::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsMantling)
	{
		CancelMantleExecuteAbility();
		bIsMantling = false;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Mantle::CancelMantleExecuteAbility()
{
	if (!MantleExecuteAbilityTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTagContainer TagsToCancel;
		TagsToCancel.AddTag(MantleExecuteAbilityTag);
		ASC->CancelAbilities(&TagsToCancel, nullptr, nullptr);
	}
}

AHarmoniaCharacter* UHarmoniaGameplayAbility_Mantle::GetHarmoniaCharacter() const
{
	return Cast<AHarmoniaCharacter>(GetAvatarActorFromActorInfo());
}
