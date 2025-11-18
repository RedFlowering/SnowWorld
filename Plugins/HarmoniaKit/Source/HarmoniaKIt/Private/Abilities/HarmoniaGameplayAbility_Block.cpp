// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Block.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UHarmoniaGameplayAbility_Block::UHarmoniaGameplayAbility_Block(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Setup default tags
	BlockingTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Blocking")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking")));
}

bool UHarmoniaGameplayAbility_Block::CanActivateAbility(
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
		return MeleeComp->CanBlock();
	}

	return false;
}

void UHarmoniaGameplayAbility_Block::ActivateAbility(
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

	// Apply blocking tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTags(BlockingTags);
	}

	// Set defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::Blocking);
	}

	// Play block animation if available
	if (BlockStartMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			BlockStartMontage,
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

void UHarmoniaGameplayAbility_Block::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Remove blocking tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTags(BlockingTags);
	}

	// Reset defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Block::GetMeleeCombatComponent() const
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
