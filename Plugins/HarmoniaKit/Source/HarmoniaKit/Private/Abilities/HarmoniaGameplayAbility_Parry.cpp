// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Parry.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaLogCategories.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"
#include "HarmoniaGameplayTags.h"

UHarmoniaGameplayAbility_Parry::UHarmoniaGameplayAbility_Parry(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Use inherited tag containers - configure these in Blueprint or derived classes:
	// - ActivationOwnedTags: Tags applied while parrying (e.g., Character.State.Parrying)
	// - ActivationBlockedTags: Tags that prevent parry (e.g., Character.State.Attacking)
	// - BlockAbilitiesWithTag: Abilities to block while parrying
}

bool UHarmoniaGameplayAbility_Parry::CanActivateAbility(
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
		if (!MeleeComp->CanParry())
		{
			UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Parry] CanActivate FAIL: CanParry() false"));
			return false;
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_Parry::ActivateAbility(
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

	UE_LOG(LogHarmoniaCombat, Log, TEXT("[Parry] Activated - Window: %.2fs"), ParryWindowDuration);

	// Note: Stamina cost is handled by Cost Gameplay Effect Class (set in BP)

	// Set defense state and add Parrying tag
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::Parrying);
	}
	// Note: State.Combat.Parrying tag is automatically added by ActivationOwnedTags (set in BP)

	// Start parry window timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ParryWindowTimerHandle,
			this,
			&UHarmoniaGameplayAbility_Parry::EndParryWindow,
			ParryWindowDuration,
			false
		);
	}

	// Play parry animation if available
	if (ParryMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ParryMontage,
			1.0f,
			NAME_None,
			false,
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->ReadyForActivation();
		}
	}
}

void UHarmoniaGameplayAbility_Parry::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ParryWindowTimerHandle);
	}

	// Reset defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
	}
	// Note: State.Combat.Parrying tag is automatically removed when ability ends (ActivationOwnedTags)

	UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Parry] EndAbility - Cancelled: %s"), bWasCancelled ? TEXT("Yes") : TEXT("No"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Parry::EndParryWindow()
{
	// Parry window expired, end ability
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Parry::GetMeleeCombatComponent() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent;
	}

	// In Lyra, OwnerActor is PlayerState but components are on the Avatar (Character/Pawn)
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Avatar = ActorInfo->AvatarActor.Get())
		{
			return Avatar->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
		}
	}

	return nullptr;
}
