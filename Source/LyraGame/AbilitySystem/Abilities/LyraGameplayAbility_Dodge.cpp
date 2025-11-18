// Copyright 2024 Snow Game Studio.

#include "LyraGameplayAbility_Dodge.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"

ULyraGameplayAbility_Dodge::ULyraGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	ActivationPolicy = ELyraAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ELyraAbilityActivationGroup::Independent;
}

void ULyraGameplayAbility_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PerformDodge();
}

void ULyraGameplayAbility_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	RemoveInvincibilityFrames();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool ULyraGameplayAbility_Dodge::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ALyraCharacter* Character = Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		return false;
	}

	// Check if character is grounded
	const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp || !MovementComp->IsMovingOnGround())
	{
		return false;
	}

	return true;
}

void ULyraGameplayAbility_Dodge::PerformDodge()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Get dodge direction
	FVector DodgeDir = GetDodgeDirection();

	// Apply invincibility frames
	ApplyInvincibilityFrames();

	// Apply dodge movement
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		// Launch character in dodge direction
		FVector LaunchVelocity = DodgeDir * (DodgeDistance / DodgeDuration);
		LaunchVelocity.Z = 0.0f; // Keep it horizontal

		Character->LaunchCharacter(LaunchVelocity, true, true);
	}

	// Play dodge montage if available
	if (DodgeMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			DodgeMontage,
			1.0f,
			NAME_None,
			false
		);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &ULyraGameplayAbility_Dodge::K2_EndAbility);
			MontageTask->OnInterrupted.AddDynamic(this, &ULyraGameplayAbility_Dodge::K2_EndAbility);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		// If no montage, end ability after dodge duration
		UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, DodgeDuration);
		if (DelayTask)
		{
			DelayTask->OnFinish.AddDynamic(this, &ULyraGameplayAbility_Dodge::K2_EndAbility);
			DelayTask->ReadyForActivation();
		}
	}
}

FVector ULyraGameplayAbility_Dodge::GetDodgeDirection() const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return FVector::ForwardVector;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return Character->GetActorForwardVector();
	}

	// Get current movement input
	FVector InputVector = MovementComp->GetLastInputVector();

	// If no input, dodge forward
	if (InputVector.IsNearlyZero())
	{
		return Character->GetActorForwardVector();
	}

	// Normalize and return
	return InputVector.GetSafeNormal();
}

void ULyraGameplayAbility_Dodge::ApplyInvincibilityFrames()
{
	if (!InvincibilityEffect)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Apply invincibility effect
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(InvincibilityEffect, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		InvincibilityEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void ULyraGameplayAbility_Dodge::RemoveInvincibilityFrames()
{
	if (InvincibilityEffectHandle.IsValid())
	{
		ULyraAbilitySystemComponent* ASC = GetLyraAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveActiveGameplayEffect(InvincibilityEffectHandle);
			InvincibilityEffectHandle.Invalidate();
		}
	}
}
