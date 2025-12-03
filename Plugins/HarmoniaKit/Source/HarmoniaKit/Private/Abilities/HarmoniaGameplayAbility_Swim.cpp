// Copyright RedFlowering. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_Swim.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UHarmoniaGameplayAbility_Swim::UHarmoniaGameplayAbility_Swim(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// ============================================================================
	// Tag Configuration: Do NOT hardcode tags in constructor!
	// Configure these in Blueprint or derived class CDO:
	// ============================================================================
	// ActivationOwnedTags:
	//   - State.Swimming
	// BlockAbilitiesWithTag:
	//   - State.Mounted
	//   - State.Climbing
	// ============================================================================

	bIsDiving = false;
	CurrentOxygen = OxygenCapacity;
}

void UHarmoniaGameplayAbility_Swim::ActivateAbility(
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

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Set movement mode to swimming
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->SetMovementMode(MOVE_Swimming);
		MovementComp->MaxSwimSpeed = SwimmingSpeed;
	}

	// Initialize oxygen
	CurrentOxygen = OxygenCapacity;

	// Send swimming started event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Swimming_Started, &EventData);
	}
}

void UHarmoniaGameplayAbility_Swim::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		// Restore normal movement
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp)
		{
			MovementComp->SetMovementMode(MOVE_Walking);
		}
	}

	// Stop diving if diving
	if (bIsDiving)
	{
		StopDiving();
	}

	// Send swimming stopped event
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Swimming_Stopped, &EventData);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_Swim::CanActivateAbility(
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

	// Check for swimming restrictions
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::Movement_Restricted_NoSwim))
		{
			return false;
		}
	}

	// Check if in water (character movement component should handle this)
	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (Character)
	{
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp)
		{
			return MovementComp->IsSwimming();
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_Swim::TickOxygen(float DeltaTime)
{
	if (!bIsDiving)
	{
		// Regenerate oxygen when not diving
		CurrentOxygen = FMath::Min(CurrentOxygen + (OxygenConsumptionRate * DeltaTime * 2.0f), OxygenCapacity);
		return;
	}

	// Consume oxygen while diving
	CurrentOxygen -= OxygenConsumptionRate * DeltaTime;

	if (CurrentOxygen <= 0.0f)
	{
		CurrentOxygen = 0.0f;

		// Send oxygen depleted event
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Drowning);

			FGameplayEventData EventData;
			ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Oxygen_Depleted, &EventData);
		}
	}
	else if (CurrentOxygen <= OxygenCapacity * 0.25f)
	{
		// Low oxygen warning
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_LowOxygen);
		}
	}
}

void UHarmoniaGameplayAbility_Swim::StartDiving()
{
	if (bIsDiving)
	{
		return;
	}

	bIsDiving = true;

	// Add diving tags
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Diving);
		ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Underwater);

		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_Diving_Started, &EventData);
	}

	// Adjust swimming speed for diving
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo)
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
			if (MovementComp)
			{
				MovementComp->MaxSwimSpeed = DivingSpeed;
			}
		}
	}
}

void UHarmoniaGameplayAbility_Swim::StopDiving()
{
	if (!bIsDiving)
	{
		return;
	}

	bIsDiving = false;

	// Remove diving tags
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Diving);
		ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Underwater);
		ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_LowOxygen);
		ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Drowning);
	}

	// Restore swimming speed
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo)
	{
		ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
		if (Character)
		{
			UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
			if (MovementComp)
			{
				MovementComp->MaxSwimSpeed = SwimmingSpeed;
			}
		}
	}
}
