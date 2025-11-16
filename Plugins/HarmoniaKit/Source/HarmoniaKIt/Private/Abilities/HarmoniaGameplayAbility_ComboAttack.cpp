// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_ComboAttack.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_ComboAttack::UHarmoniaGameplayAbility_ComboAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Set default activation policy
	ActivationPolicy = ELyraAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ELyraAbilityActivationGroup::Exclusive_Replaceable;

	// Setup default tags
	AttackingTags.AddTag(HarmoniaGameplayTags::State_Combat_Attacking);

	BlockedTags.AddTag(HarmoniaGameplayTags::State_Combat_Attacking);
}

bool UHarmoniaGameplayAbility_ComboAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if we're already in a combo window - if so, allow activation for combo continuation
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC && ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_Combat_ComboWindow))
	{
		return true;
	}

	// If not in combo window, check if we're not already attacking
	if (ASC && ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_Combat_Attacking))
	{
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_ComboAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply attacking tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTags(AttackingTags);
	}

	// If we're in a combo window, advance to next combo
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_Combat_ComboWindow))
		{
			AdvanceCombo();
		}
	}

	// Perform the attack
	PerformComboAttack();
}

void UHarmoniaGameplayAbility_ComboAttack::PerformComboAttack()
{
	if (!HasCurrentComboData())
	{
		UE_LOG(LogTemp, Warning, TEXT("ComboAttack: No valid combo data for index %d"), CurrentComboIndex);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FComboAttackData ComboData = GetCurrentComboData();
	if (!ComboData.AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("ComboAttack: No montage for index %d"), CurrentComboIndex);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Play the attack montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ComboData.AttackMontage,
		1.0f,
		NAME_None,
		false,
		1.0f
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_ComboAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_ComboAttack::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_ComboAttack::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}

	// Start combo window timer
	if (UWorld* World = GetWorld())
	{
		// Add combo window tag
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Combat_ComboWindow);
		}

		// Set timer to close combo window
		World->GetTimerManager().SetTimer(
			ComboWindowTimerHandle,
			[this]()
			{
				// Remove combo window tag
				if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
				{
					ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Combat_ComboWindow);
				}

				// If no next combo was queued, start reset timer
				if (!bNextComboQueued)
				{
					if (UWorld* World = GetWorld())
					{
						World->GetTimerManager().SetTimer(
							ComboResetTimerHandle,
							this,
							&UHarmoniaGameplayAbility_ComboAttack::ResetCombo,
							1.0f,
							false
						);
					}
				}
			},
			ComboData.ComboWindowDuration,
			false
		);
	}

	// TODO: Apply damage, effects, etc. based on ComboData
	UE_LOG(LogTemp, Log, TEXT("ComboAttack: Performing combo %d - %s (Damage: %.2fx)"),
		CurrentComboIndex,
		*ComboData.DisplayName.ToString(),
		ComboData.DamageMultiplier);
}

void UHarmoniaGameplayAbility_ComboAttack::AdvanceCombo()
{
	if (HasNextComboData())
	{
		CurrentComboIndex++;
		bNextComboQueued = true;
		UE_LOG(LogTemp, Log, TEXT("ComboAttack: Advanced to combo index %d"), CurrentComboIndex);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("ComboAttack: No more combos in sequence, staying at index %d"), CurrentComboIndex);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::ResetCombo()
{
	CurrentComboIndex = 0;
	bNextComboQueued = false;

	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
		World->GetTimerManager().ClearTimer(ComboResetTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("ComboAttack: Combo sequence reset"));
}

void UHarmoniaGameplayAbility_ComboAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Remove attacking tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTags(AttackingTags);
	}

	// Clear combo window timer if still active
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
	}

	// If cancelled, reset combo
	if (bWasCancelled)
	{
		ResetCombo();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_ComboAttack::OnMontageCompleted()
{
	// If next combo was queued during this attack, don't end the ability
	// The new activation will handle it
	if (!bNextComboQueued)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		// Reset the queued flag for next iteration
		bNextComboQueued = false;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_ComboAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_ComboAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

FComboAttackData UHarmoniaGameplayAbility_ComboAttack::GetCurrentComboData() const
{
	if (!ComboDataTable || !ComboSequence.IsValidIndex(CurrentComboIndex))
	{
		return FComboAttackData();
	}

	const FName RowName = ComboSequence[CurrentComboIndex];
	const FComboAttackData* Data = ComboDataTable->FindRow<FComboAttackData>(RowName, TEXT("GetCurrentComboData"));
	return Data ? *Data : FComboAttackData();
}

FComboAttackData UHarmoniaGameplayAbility_ComboAttack::GetNextComboData() const
{
	const int32 NextIndex = CurrentComboIndex + 1;
	if (!ComboDataTable || !ComboSequence.IsValidIndex(NextIndex))
	{
		return FComboAttackData();
	}

	const FName RowName = ComboSequence[NextIndex];
	const FComboAttackData* Data = ComboDataTable->FindRow<FComboAttackData>(RowName, TEXT("GetNextComboData"));
	return Data ? *Data : FComboAttackData();
}

bool UHarmoniaGameplayAbility_ComboAttack::HasCurrentComboData() const
{
	return ComboDataTable && ComboSequence.IsValidIndex(CurrentComboIndex);
}

bool UHarmoniaGameplayAbility_ComboAttack::HasNextComboData() const
{
	const int32 NextIndex = CurrentComboIndex + 1;
	return ComboDataTable && ComboSequence.IsValidIndex(NextIndex);
}
