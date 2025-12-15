// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Riposte.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"

UHarmoniaGameplayAbility_Riposte::UHarmoniaGameplayAbility_Riposte(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Use inherited tag containers - configure these in Blueprint or derived classes:
	// - ActivationOwnedTags: Tags applied while riposting (e.g., Character.State.Riposting)
	// - ActivationBlockedTags: Tags that prevent riposte (e.g., Character.State.Attacking, Character.State.Blocking, Character.State.Dodging)
	// - BlockAbilitiesWithTag: Abilities to block while riposting

	// Default riposte config
	RiposteConfig.RiposteWindowDuration = 2.0f;
	RiposteConfig.RiposteDamageMultiplier = 3.0f;
	RiposteConfig.RiposteStaminaCost = 15.0f;
	RiposteConfig.bGuaranteedCritical = true;
}

bool UHarmoniaGameplayAbility_Riposte::CanActivateAbility(
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
		// Can only riposte during riposte window
		if (MeleeComp->GetDefenseState() != EHarmoniaDefenseState::RiposteWindow)
		{
			return false;
		}

		// Check stamina
		if (!MeleeComp->HasEnoughStamina(RiposteConfig.RiposteStaminaCost))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_Riposte::ActivateAbility(
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

	if (!MeleeCombatComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Consume stamina
	MeleeCombatComponent->ConsumeStamina(RiposteConfig.RiposteStaminaCost);

	// Get parried target from combat component
	ParriedTarget = MeleeCombatComponent->GetParriedTarget();

	// Play riposte animation
	if (RiposteConfig.RiposteMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			RiposteConfig.RiposteMontage,
			1.0f,
			NAME_None,
			false,
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_Riposte::OnMontageCompleted);
			MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_Riposte::OnMontageCompleted);
			MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_Riposte::OnMontageCompleted);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		// No animation, apply damage immediately and end
		if (ParriedTarget.IsValid())
		{
			ApplyRiposteDamage(ParriedTarget.Get());
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UHarmoniaGameplayAbility_Riposte::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Reset defense state
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
		MeleeCombatComponent->ClearParriedTarget();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Riposte::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Riposte::OnRiposteHit(const FHarmoniaAttackHitResult& HitResult)
{
	if (!HitResult.IsValid())
	{
		return;
	}

	ApplyRiposteDamage(HitResult.HitActor);
}

void UHarmoniaGameplayAbility_Riposte::ApplyRiposteDamage(AActor* Target)
{
	if (!Target || !IsValid(Target))
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);

	if (!SourceASC || !TargetASC)
	{
		return;
	}

	// Apply riposte damage effect
	if (RiposteDamageEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
			RiposteDamageEffectClass,
			GetAbilityLevel(),
			EffectContext
		);

		if (SpecHandle.IsValid())
		{
			// Apply damage multiplier
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Damage")),
				RiposteConfig.RiposteDamageMultiplier
			);

			// Set as critical hit
			if (RiposteConfig.bGuaranteedCritical)
			{
				SpecHandle.Data->SetSetByCallerMagnitude(
					FGameplayTag::RequestGameplayTag(FName("Data.Critical")),
					1.0f
				);
			}

			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
		}
	}

	// Apply target stun effect
	if (TargetStunEffectClass && RiposteConfig.bPlayTargetStunAnimation)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
			TargetStunEffectClass,
			GetAbilityLevel(),
			EffectContext
		);

		if (SpecHandle.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
		}
	}

	// Trigger gameplay cue
	if (RiposteHitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = Target->GetActorLocation();
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.EffectCauser = GetAvatarActorFromActorInfo();
		SourceASC->ExecuteGameplayCue(RiposteHitGameplayCueTag, CueParams);
	}

	// Apply camera shake
	if (RiposteHitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetOwningActorFromActorInfo()->GetInstigatorController()))
		{
			PC->ClientStartCameraShake(RiposteHitCameraShakeClass);
		}
	}
}

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Riposte::GetMeleeCombatComponent() const
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
