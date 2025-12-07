// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_MeleeAttack.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "GameplayCueManager.h"
#include "Camera/CameraShakeBase.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

UHarmoniaGameplayAbility_MeleeAttack::UHarmoniaGameplayAbility_MeleeAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Allow re-triggering during combo window for input queueing
	bRetriggerInstancedAbility = true;

	// Tag configuration: See Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.2
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
		UE_LOG(LogTemp, Warning, TEXT("[MeleeAttack] CanActivateAbility: Super::CanActivateAbility returned false"));
		return false;
	}

	// Check if melee combat component allows attack
	UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent();
	if (!MeleeComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MeleeAttack] CanActivateAbility: MeleeCombatComponent is NULL!"));
		return false;
	}

	if (!MeleeComp->CanAttack())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MeleeAttack] CanActivateAbility: MeleeComp->CanAttack() returned false"));
		return false;
	}

	// Check stamina
	const float StaminaCost = (AttackType == EHarmoniaAttackType::Heavy) ? MeleeComp->GetHeavyAttackStaminaCost() : MeleeComp->GetLightAttackStaminaCost();
	const float CurrentStamina = MeleeComp->GetCurrentStamina();
	
	if (!MeleeComp->HasEnoughStamina(StaminaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MeleeAttack] CanActivateAbility: Not enough stamina! (%.1f < %.1f)"), CurrentStamina, StaminaCost);
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_MeleeAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Cache components first
	MeleeCombatComponent = GetMeleeCombatComponent();
	AttackComponent = GetAttackComponent();

	if (!MeleeCombatComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[MeleeAttack] ActivateAbility: MeleeCombatComponent is NULL!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Error, TEXT("[MeleeAttack] ActivateAbility: CommitAbility FAILED!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get combo sequence based on attack type
	if (!MeleeCombatComponent->GetComboSequence(AttackType, CurrentComboSequence))
	{
		UE_LOG(LogTemp, Error, TEXT("[MeleeAttack] ActivateAbility: GetComboSequence FAILED! AttackType=%d"), (int32)AttackType);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Start attack in melee combat component
	MeleeCombatComponent->StartAttack(AttackType);

	// Perform the attack
	PerformMeleeAttack();
	
	// Start waiting for combo input during the attack
	StartWaitingForComboInput();
}

void UHarmoniaGameplayAbility_MeleeAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// End attack in melee combat component
	if (MeleeCombatComponent)
	{
		// If ability was cancelled (by dodge, block, etc.), reset combo
		if (bWasCancelled)
		{
			MeleeCombatComponent->ResetCombo();
		}
		
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
		UE_LOG(LogTemp, Error, TEXT("[MeleeAttack] PerformMeleeAttack: Montage is NULL! Check combo sequence data."));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Get the current combo step to get montage section name
	FHarmoniaComboAttackStep CurrentStep;
	FName SectionName = NAME_None;
	if (GetCurrentAttackStep(CurrentStep))
	{
		SectionName = CurrentStep.MontageSectionName;
	}

	// Play the attack montage with the correct section
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		Montage,
		1.0f,
		SectionName,  // Use the section name from combo step
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
		UE_LOG(LogTemp, Log, TEXT("[MeleeAttack] PerformMeleeAttack: MontageTask activated"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MeleeAttack] PerformMeleeAttack: MontageTask creation FAILED!"));
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

void UHarmoniaGameplayAbility_MeleeAttack::StartWaitingForComboInput()
{
	// Create task to wait for input press during the attack animation
	UAbilityTask_WaitInputPress* WaitInputTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	if (WaitInputTask)
	{
		WaitInputTask->OnPress.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnComboInputPressed);
		WaitInputTask->ReadyForActivation();
	}
}

void UHarmoniaGameplayAbility_MeleeAttack::OnComboInputPressed(float TimeWaited)
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->QueueNextCombo();
	}
	
	// Start waiting for another input (in case of 3+ combo)
	StartWaitingForComboInput();
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

UHarmoniaSenseAttackComponent* UHarmoniaGameplayAbility_MeleeAttack::GetAttackComponent() const
{
	if (AttackComponent)
	{
		return AttackComponent;
	}

	// In Lyra, OwnerActor is PlayerState but components are on the Avatar (Character/Pawn)
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Avatar = ActorInfo->AvatarActor.Get())
		{
			return Avatar->FindComponentByClass<UHarmoniaSenseAttackComponent>();
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
		FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();

		// Create a proper FHitResult
		FHitResult HitResult;
		HitResult.HitObjectHandle = FActorInstanceHandle(TargetActor);
		HitResult.Location = TargetActor->GetActorLocation();
		HitResult.ImpactPoint = TargetActor->GetActorLocation();

		NewTargetData->HitResult = HitResult;

		TargetData.Add(NewTargetData);
	}

	return TargetData;
}
