// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_MeleeAttack.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayCueManager.h"
#include "Camera/CameraShakeBase.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"

UHarmoniaGameplayAbility_MeleeAttack::UHarmoniaGameplayAbility_MeleeAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Setup default tags
	AttackingTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Blocking")));
	BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Character.State.Dodging")));
}

bool UHarmoniaGameplayAbility_MeleeAttack::CanActivateAbility(
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

	// Check if melee combat component allows attack
	if (UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent())
	{
		if (!MeleeComp->CanAttack())
		{
			return false;
		}

		// Check stamina
		const float StaminaCost = bIsHeavyAttack ? MeleeComp->GetHeavyAttackStaminaCost() : MeleeComp->GetLightAttackStaminaCost();
		if (!MeleeComp->HasEnoughStamina(StaminaCost))
		{
			return false;
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_MeleeAttack::ActivateAbility(
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

	// Cache components
	MeleeCombatComponent = GetMeleeCombatComponent();
	AttackComponent = GetAttackComponent();

	if (!MeleeCombatComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply attacking tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTags(AttackingTags);
	}

	// Get combo sequence
	if (!MeleeCombatComponent->GetComboSequence(bIsHeavyAttack, CurrentComboSequence))
	{
		// No combo sequence found, end ability
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Start attack in melee combat component
	MeleeCombatComponent->StartAttack(bIsHeavyAttack);

	// Perform the attack
	PerformMeleeAttack();
}

void UHarmoniaGameplayAbility_MeleeAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Remove attacking tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTags(AttackingTags);
	}

	// End attack in melee combat component
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->EndAttack();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ============================================================================
// Attack Execution
// ============================================================================

void UHarmoniaGameplayAbility_MeleeAttack::PerformMeleeAttack()
{
	UAnimMontage* Montage = GetCurrentAttackMontage();
	if (!Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Play the attack montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.0f,
		NAME_None,
		false,
		1.0f
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageInterrupted);
		MontageTask->OnBlendOut.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageBlendOut);

		MontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

UAnimMontage* UHarmoniaGameplayAbility_MeleeAttack::GetCurrentAttackMontage() const
{
	FHarmoniaComboAttackStep CurrentStep;
	if (GetCurrentAttackStep(CurrentStep))
	{
		return CurrentStep.AttackMontage;
	}
	return nullptr;
}

bool UHarmoniaGameplayAbility_MeleeAttack::GetCurrentAttackStep(FHarmoniaComboAttackStep& OutStep) const
{
	if (!MeleeCombatComponent)
	{
		return false;
	}

	const int32 ComboIndex = MeleeCombatComponent->GetCurrentComboIndex();
	if (CurrentComboSequence.ComboSteps.IsValidIndex(ComboIndex))
	{
		OutStep = CurrentComboSequence.ComboSteps[ComboIndex];
		return true;
	}

	return false;
}

// ============================================================================
// Combo Management
// ============================================================================

void UHarmoniaGameplayAbility_MeleeAttack::QueueNextCombo()
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->QueueNextCombo();
	}
}

bool UHarmoniaGameplayAbility_MeleeAttack::IsInComboWindow() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent->IsInComboWindow();
	}
	return false;
}

// ============================================================================
// Animation Callbacks
// ============================================================================

void UHarmoniaGameplayAbility_MeleeAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_MeleeAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_MeleeAttack::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_MeleeAttack::OnMontageBlendOut()
{
	// Optional: Handle blend out
}

// ============================================================================
// Hit Detection
// ============================================================================

void UHarmoniaGameplayAbility_MeleeAttack::OnAttackHit(const FHarmoniaAttackHitResult& HitResult)
{
	if (!HitResult.IsValid())
	{
		return;
	}

	// Apply damage to target
	ApplyDamageToTarget(HitResult.HitActor, HitResult);

	// Trigger visual effects
	TriggerHitEffects(HitResult);
}

// ============================================================================
// Private Methods
// ============================================================================

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_MeleeAttack::GetMeleeCombatComponent() const
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

UHarmoniaSenseAttackComponent* UHarmoniaGameplayAbility_MeleeAttack::GetAttackComponent() const
{
	if (AttackComponent)
	{
		return AttackComponent;
	}

	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Owner = ActorInfo->OwnerActor.Get())
		{
			return Owner->FindComponentByClass<UHarmoniaSenseAttackComponent>();
		}
	}

	return nullptr;
}

void UHarmoniaGameplayAbility_MeleeAttack::ApplyDamageToTarget(AActor* TargetActor, const FHarmoniaAttackHitResult& HitResult)
{
	if (!TargetActor || !DamageEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	// Check for backstab
	bool bIsBackstab = false;
	float DamageMultiplier = 1.0f;
	EHarmoniaCriticalAttackType CriticalType = EHarmoniaCriticalAttackType::Normal;

	if (MeleeCombatComponent)
	{
		const FVector AttackOrigin = GetAvatarActorFromActorInfo()->GetActorLocation();
		bIsBackstab = MeleeCombatComponent->IsBackstabAttack(TargetActor, AttackOrigin);

		if (bIsBackstab)
		{
			DamageMultiplier = MeleeCombatComponent->GetBackstabDamageMultiplier();
			CriticalType = EHarmoniaCriticalAttackType::Backstab;
		}
	}

	// Create effect context
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);
	EffectContext.AddHitResult(FHitResult()); // Could pass actual hit info here

	// Apply damage effect
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		// Apply damage multiplier for backstab
		if (bIsBackstab)
		{
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.DamageMultiplier")),
				DamageMultiplier
			);

			// Set as critical hit
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.Critical")),
				1.0f
			);

			// Set critical type
			SpecHandle.Data->SetSetByCallerMagnitude(
				FGameplayTag::RequestGameplayTag(FName("Data.CriticalType")),
				static_cast<float>(CriticalType)
			);
		}

		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, MakeTargetData(TargetActor));
	}

	// Apply additional effects
	for (TSubclassOf<UGameplayEffect> AdditionalEffect : AdditionalHitEffects)
	{
		if (AdditionalEffect)
		{
			FGameplayEffectSpecHandle AdditionalSpecHandle = MakeOutgoingGameplayEffectSpec(AdditionalEffect, GetAbilityLevel());
			if (AdditionalSpecHandle.IsValid())
			{
				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, AdditionalSpecHandle, MakeTargetData(TargetActor));
			}
		}
	}
}

void UHarmoniaGameplayAbility_MeleeAttack::TriggerHitEffects(const FHarmoniaAttackHitResult& HitResult)
{
	// Trigger gameplay cue
	if (HitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.HitLocation;
		CueParams.Normal = HitResult.HitNormal;
		CueParams.Instigator = GetAvatarActorFromActorInfo();
		CueParams.EffectCauser = GetAvatarActorFromActorInfo();

		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
		}
	}

	// Trigger camera shake
	if (HitCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetOwningActorFromActorInfo()->GetInstigatorController()))
		{
			PC->ClientStartCameraShake(HitCameraShakeClass);
		}
	}
}

FGameplayAbilityTargetDataHandle UHarmoniaGameplayAbility_MeleeAttack::MakeTargetData(AActor* TargetActor) const
{
	FGameplayAbilityTargetDataHandle TargetData;

	if (TargetActor)
	{
		FLyraGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();
		NewTargetData->HitResult.Actor = TargetActor;
		NewTargetData->HitResult.Location = TargetActor->GetActorLocation();

		TargetData.Add(NewTargetData);
	}

	return TargetData;
}
