// Copyright RedFlowering. All Rights Reserved.

#include "AbilitySystem/Abilities/HarmoniaGameplayAbility_ComboAttack.h"
#include "AbilitySystem/HarmoniaGameplayTagsBFL.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_ComboAttack::UHarmoniaGameplayAbility_ComboAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentComboIndex = 0;
	MaxComboIndex = 0;
	bNextComboRequested = false;
	bInComboWindow = false;
	bIsAttacking = false;

	// Set default activation policy
	ActivationPolicy = ELyraAbilityActivationPolicy::OnInputTriggered;

	// Set default tags - these should be configured in Blueprint or data
	AttackingTag = FGameplayTag::RequestGameplayTag(FName("Harmonia.State.Attacking"));
	ComboWindowTag = FGameplayTag::RequestGameplayTag(FName("Harmonia.State.ComboWindow"));
}

void UHarmoniaGameplayAbility_ComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Load combo data if not already loaded
	if (AllComboData.Num() == 0)
	{
		LoadComboData();
	}

	// If we're not in a combo window, start from the beginning
	if (!bInComboWindow)
	{
		CurrentComboIndex = 0;
	}

	// Perform the attack
	PerformComboAttack();
}

void UHarmoniaGameplayAbility_ComboAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear any active timers
	ClearComboWindowTimer();

	// Reset combo state if cancelled or no combo window active
	if (bWasCancelled || !bInComboWindow)
	{
		ResetCombo();
	}

	// Remove any gameplay tags we added
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(AttackingTag);
		ASC->RemoveLooseGameplayTag(ComboWindowTag);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_ComboAttack::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// If we're in a combo window and attacking, request the next combo
	if (bInComboWindow && bIsAttacking)
	{
		bNextComboRequested = true;
	}
}

void UHarmoniaGameplayAbility_ComboAttack::PerformComboAttack()
{
	// Get combo data for current index
	if (!GetComboDataForIndex(CurrentComboIndex, CurrentComboData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaGameplayAbility_ComboAttack: Failed to get combo data for index %d"), CurrentComboIndex);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Mark as attacking
	bIsAttacking = true;
	bNextComboRequested = false;

	// Add attacking tag
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(AttackingTag);
		ASC->AddLooseGameplayTag(CurrentComboData.ComboTag);
	}

	// Play attack animation
	PlayAttackMontage();

	// Apply damage effect
	ApplyAttackDamage();

	// Start combo window timer
	StartComboWindowTimer(CurrentComboData.ComboWindowDuration);
}

void UHarmoniaGameplayAbility_ComboAttack::AdvanceCombo()
{
	// Check if we can advance to the next combo
	if (CurrentComboIndex < MaxComboIndex)
	{
		CurrentComboIndex++;

		// Remove previous combo tag
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveLooseGameplayTag(CurrentComboData.ComboTag);
		}

		// Perform next combo attack
		PerformComboAttack();
	}
	else
	{
		// We've reached the end of the combo chain
		ResetCombo();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::ResetCombo()
{
	CurrentComboIndex = 0;
	bNextComboRequested = false;
	bInComboWindow = false;
	bIsAttacking = false;

	ClearComboWindowTimer();

	// Remove tags
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(AttackingTag);
		ASC->RemoveLooseGameplayTag(ComboWindowTag);
		ASC->RemoveLooseGameplayTag(CurrentComboData.ComboTag);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::OnComboWindowExpired()
{
	bInComboWindow = false;

	// Remove combo window tag
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(ComboWindowTag);
	}

	// If no next combo was requested, reset and end
	if (!bNextComboRequested)
	{
		ResetCombo();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;

	// If next combo was requested during the attack, advance
	if (bNextComboRequested && bInComboWindow)
	{
		AdvanceCombo();
	}
	else if (!bInComboWindow)
	{
		// Combo window expired, end ability
		ResetCombo();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::OnMontageCancelled(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;
	ResetCombo();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_ComboAttack::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	// Montage is blending out, but ability might continue for combo
	bIsAttacking = false;
}

void UHarmoniaGameplayAbility_ComboAttack::LoadComboData()
{
	UHarmoniaGameplayTagsBFL::GetAllComboAttackData(AllComboData);
	MaxComboIndex = UHarmoniaGameplayTagsBFL::GetMaxComboIndex();

	if (AllComboData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaGameplayAbility_ComboAttack: No combo attack data found in DataTable!"));
	}
}

bool UHarmoniaGameplayAbility_ComboAttack::GetComboDataForIndex(int32 ComboIndex, FHarmoniaComboAttackData& OutComboData) const
{
	for (const FHarmoniaComboAttackData& ComboData : AllComboData)
	{
		if (ComboData.ComboIndex == ComboIndex)
		{
			OutComboData = ComboData;
			return true;
		}
	}

	return false;
}

void UHarmoniaGameplayAbility_ComboAttack::StartComboWindowTimer(float Duration)
{
	ClearComboWindowTimer();

	bInComboWindow = true;

	// Add combo window tag
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(ComboWindowTag);
	}

	// Start timer
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		Character->GetWorldTimerManager().SetTimer(
			ComboWindowTimerHandle,
			this,
			&UHarmoniaGameplayAbility_ComboAttack::OnComboWindowExpired,
			Duration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::ClearComboWindowTimer()
{
	if (ComboWindowTimerHandle.IsValid())
	{
		if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		{
			Character->GetWorldTimerManager().ClearTimer(ComboWindowTimerHandle);
		}
		ComboWindowTimerHandle.Invalidate();
	}
}

void UHarmoniaGameplayAbility_ComboAttack::PlayAttackMontage()
{
	if (!CurrentComboData.AttackMontage.IsValid())
	{
		// Try to load the montage
		CurrentComboData.AttackMontage.LoadSynchronous();
	}

	UAnimMontage* MontageToPlay = CurrentComboData.AttackMontage.Get();
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHarmoniaGameplayAbility_ComboAttack: No attack montage set for combo index %d"), CurrentComboIndex);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// Set up montage delegates
	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UHarmoniaGameplayAbility_ComboAttack::OnMontageCompleted);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &UHarmoniaGameplayAbility_ComboAttack::OnMontageBlendOut);

	// Play montage
	float MontageDuration = AnimInstance->Montage_Play(MontageToPlay, 1.0f);

	if (MontageDuration > 0.0f)
	{
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, MontageToPlay);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::ApplyAttackDamage()
{
	if (!AttackDamageEffect)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Create effect context
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// Create effect spec
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AttackDamageEffect, GetAbilityLevel(), EffectContext);
	if (SpecHandle.IsValid())
	{
		// Set damage multiplier based on combo data
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), CurrentComboData.DamageMultiplier);

		// Apply the effect to self (this could be modified to apply to targets hit)
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
