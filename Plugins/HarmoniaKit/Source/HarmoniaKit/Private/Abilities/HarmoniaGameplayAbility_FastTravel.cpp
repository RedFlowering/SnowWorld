// Copyright RedFlowering. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_FastTravel.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_FastTravel::UHarmoniaGameplayAbility_FastTravel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationOwnedTags.AddTag(HarmoniaGameplayTags::State_FastTraveling);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Combat_Attacking);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Mounted);
}

void UHarmoniaGameplayAbility_FastTravel::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get target location from event data
	if (TriggerEventData && TriggerEventData->TargetData.IsValid(0))
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = TriggerEventData->TargetData;
		if (TargetDataHandle.IsValid(0))
		{
			const FGameplayAbilityTargetData* Data = TargetDataHandle.Get(0);
			if (Data)
			{
				TArray<TWeakObjectPtr<AActor>> TargetActors = Data->GetActors();
				if (TargetActors.Num() > 0 && TargetActors[0].IsValid())
				{
					AActor* TargetActor = TargetActors[0].Get();
					TargetLocation = TargetActor->GetActorLocation();
					TargetRotation = TargetActor->GetActorRotation();
				}
			}
		}
	}

	// Send start event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_FastTravel_Started, &EventData);
	}

	// Set timer to complete teleport
	if (ActorInfo->AvatarActor.IsValid())
	{
		ActorInfo->AvatarActor->GetWorldTimerManager().SetTimer(
			TeleportTimerHandle,
			this,
			&UHarmoniaGameplayAbility_FastTravel::CompleteTeleport,
			TravelDuration,
			false
		);
	}

	// Play travel animation/effect if available
	if (TravelMontage)
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character && Character->GetMesh())
		{
			UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(TravelMontage);
			}
		}
	}
}

void UHarmoniaGameplayAbility_FastTravel::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (TeleportTimerHandle.IsValid() && ActorInfo->AvatarActor.IsValid())
	{
		ActorInfo->AvatarActor->GetWorldTimerManager().ClearTimer(TeleportTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_FastTravel::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if fast travel is allowed
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::Movement_Restricted_NoFastTravel))
		{
			return false;
		}

		if (!ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_CanFastTravel))
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_FastTravel::CompleteTeleport()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Teleport character
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	Avatar->SetActorLocation(TargetLocation);
	Avatar->SetActorRotation(TargetRotation);

	// Send complete event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		EventData.EventMagnitude = TargetLocation.Size();
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_FastTravel_Completed, &EventData);
	}

	EndAbility(CurrentSpecHandle, ActorInfo, CurrentActivationInfo, true, false);
}
