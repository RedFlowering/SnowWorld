// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Dodge.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_Dodge::UHarmoniaGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Setup default tags
	DodgingTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dodging")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Blocking")));
}

bool UHarmoniaGameplayAbility_Dodge::CanActivateAbility(
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

	if (UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent())
	{
		if (!MeleeComp->CanDodge())
		{
			return false;
		}

		// Check stamina
		if (!MeleeComp->HasEnoughStamina(DodgeStaminaCost))
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_Dodge::ActivateAbility(
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

	MeleeCombatComponent = GetMeleeCombatComponent();

	// Consume stamina
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->ConsumeStamina(DodgeStaminaCost);
	}

	// Apply dodging tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTags(DodgingTags);
	}

	// Set defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::Dodging);
	}

	// Start i-frames timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			IFrameStartTimerHandle,
			this,
			&UHarmoniaGameplayAbility_Dodge::StartIFrames,
			IFrameStartTime,
			false
		);
	}

	// Play dodge animation
	if (DodgeMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			DodgeMontage,
			1.0f,
			NAME_None,
			false,
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_Dodge::OnMontageCompleted);
			MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_Dodge::OnMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_Dodge::OnMontageInterrupted);

			MontageTask->ReadyForActivation();
		}
	}

	// Apply dodge movement
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			// Get dodge direction (based on input or forward)
			FVector DodgeDirection = Character->GetActorForwardVector();

			// Apply launch velocity
			const FVector LaunchVelocity = DodgeDirection * (DodgeDistance / DodgeDuration);
			MovementComp->AddImpulse(LaunchVelocity, true);
		}
	}
}

void UHarmoniaGameplayAbility_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IFrameStartTimerHandle);
		World->GetTimerManager().ClearTimer(IFrameEndTimerHandle);
	}

	// End i-frames
	if (MeleeCombatComponent && MeleeCombatComponent->IsInvulnerable())
	{
		MeleeCombatComponent->SetInvulnerable(false);
	}

	// Remove dodging tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTags(DodgingTags);
	}

	// Reset defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Dodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Dodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_Dodge::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_Dodge::StartIFrames()
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetInvulnerable(true);

		// Set timer to end i-frames
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				IFrameEndTimerHandle,
				this,
				&UHarmoniaGameplayAbility_Dodge::EndIFrames,
				IFrameDuration,
				false
			);
		}
	}
}

void UHarmoniaGameplayAbility_Dodge::EndIFrames()
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetInvulnerable(false);
	}
}

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Dodge::GetMeleeCombatComponent() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent;
	}

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Owner = ActorInfo->OwnerActor.Get())
		{
			return Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
		}
	}

	return nullptr;
}
