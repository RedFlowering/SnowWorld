// Copyright RedFlowering. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_Mount.h"
#include "Components/HarmoniaMountComponent.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"

UHarmoniaGameplayAbility_Mount::UHarmoniaGameplayAbility_Mount(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	DefaultMountType = EHarmoniaMountType::Horse;

	// Setup tags
	ActivationOwnedTags.AddTag(HarmoniaGameplayTags::State_Mounting);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Combat_Attacking);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Dodging);
	BlockAbilitiesWithTag.AddTag(HarmoniaGameplayTags::State_Mounted);
}

void UHarmoniaGameplayAbility_Mount::ActivateAbility(
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

	UHarmoniaMountComponent* MountComp = GetMountComponent();
	if (!MountComp)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Determine mount type
	EHarmoniaMountType MountType = DefaultMountType;

	// Start mounting
	if (!MountComp->StartMount(MountType))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Play mounting animation if available
	if (MountMontage)
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character && Character->GetMesh())
		{
			UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(MountMontage);
				AnimInstance->Montage_SetEndDelegate(
					FOnMontageEnded::CreateUObject(this, &UHarmoniaGameplayAbility_Mount::OnMontageCompleted),
					MountMontage
				);
			}
		}
	}
	else
	{
		// No animation, complete mounting immediately
		MountComp->CompleteMounting();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_Mount::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_Mount::CanActivateAbility(
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

	UHarmoniaMountComponent* MountComp = GetMountComponent();
	if (!MountComp)
	{
		return false;
	}

	return MountComp->CanMount();
}

void UHarmoniaGameplayAbility_Mount::OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	UHarmoniaMountComponent* MountComp = GetMountComponent();
	if (MountComp)
	{
		MountComp->CompleteMounting();
	}

	bool bReplicateEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Mount::OnMontageCancelled(UAnimMontage* Montage, bool bInterrupted)
{
	bool bReplicateEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Mount::OnMontageInterrupted(UAnimMontage* Montage, bool bInterrupted)
{
	bool bReplicateEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UHarmoniaMountComponent* UHarmoniaGameplayAbility_Mount::GetMountComponent() const
{
	if (MountComponent)
	{
		return MountComponent;
	}

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		MountComponent = ActorInfo->AvatarActor->FindComponentByClass<UHarmoniaMountComponent>();
	}

	return MountComponent;
}
