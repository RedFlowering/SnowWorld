// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_MeleeAttack.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "GameplayCueManager.h"
#include "Camera/CameraShakeBase.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"

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
		return false;
	}

	// Check if melee combat component allows attack
	UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent();
	if (!MeleeComp || !MeleeComp->CanAttack())
	{
		return false;
	}

	// For Ultimate attacks, check if we have enough gauge and not on cooldown
	if (AttackType == EHarmoniaAttackType::Ultimate)
	{
		// Get combo sequence to check required gauge and cooldown
		FHarmoniaComboAttackSequence UltimateSequence;
		if (MeleeComp->GetComboSequence(EHarmoniaAttackType::Ultimate, UltimateSequence))
		{
			if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
			{
				// Check gauge
				const float CurrentGauge = ASC->GetNumericAttribute(UHarmoniaAttributeSet::GetUltimateGaugeAttribute());
				if (CurrentGauge < UltimateSequence.UltimateGaugeRequired)
				{
					return false;
				}
			}

			// Check cooldown (timestamp-based)
			if (UltimateSequence.UltimateCooldown > 0.0f)
			{
				if (const UWorld* World = GEngine->GetWorldFromContextObject(ActorInfo->OwnerActor.Get(), EGetWorldErrorMode::LogAndReturnNull))
				{
					const float CurrentTime = World->GetTimeSeconds();
					if (CurrentTime < LastUltimateUseTime + UltimateSequence.UltimateCooldown)
					{
						return false; // Still on cooldown
					}
				}
			}
		}
	}

	// Note: Stamina cost is handled by CommitAbilityCost via CostGameplayEffectClass

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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get combo sequence based on attack type
	if (!MeleeCombatComponent->GetComboSequence(AttackType, CurrentComboSequence))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Start attack in melee combat component
	MeleeCombatComponent->StartAttack(AttackType);

	// Branch based on attack type
	if (AttackType == EHarmoniaAttackType::Charged)
	{
		// For charged attacks, start charging and wait for input release
		StartCharging();
	}
	else if (AttackType == EHarmoniaAttackType::Ultimate)
	{
		// Record use time for cooldown (gauge consumption handled by CostGameplayEffectClass)
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			LastUltimateUseTime = Avatar->GetWorld()->GetTimeSeconds();
		}

		// Perform the attack
		PerformMeleeAttack();
		StartWaitingForComboInput();
	}
	else
	{
		// For normal attacks, perform immediately
		PerformMeleeAttack();
		
		// Start waiting for combo input during the attack
		StartWaitingForComboInput();
	}
}

void UHarmoniaGameplayAbility_MeleeAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Clean up charge state
	if (bIsCharging)
	{
		bIsCharging = false;

		// Stop charge timer
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			Avatar->GetWorldTimerManager().ClearTimer(ChargeTickTimerHandle);
		}

		// Note: State.Charging tag is removed automatically by GA's Activation Owned Tags
	}

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

void UHarmoniaGameplayAbility_MeleeAttack::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// For charged attacks, release triggers the attack
	if (AttackType == EHarmoniaAttackType::Charged && bIsCharging)
	{
		ReleaseChargeAttack();
	}

	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
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
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Link section to end section if specified
	// IMPORTANT: Must be done AFTER ReadyForActivation so the montage is playing
	if (CurrentStep.MontageSectionName != NAME_None && CurrentStep.EndSectionName != NAME_None)
	{
		if (USkeletalMeshComponent* Mesh = GetActorInfo().SkeletalMeshComponent.Get())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				if (AnimInstance->Montage_IsPlaying(Montage))
				{
					AnimInstance->Montage_SetNextSection(CurrentStep.MontageSectionName, CurrentStep.EndSectionName, Montage);
				}
			}
		}
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

		// Clear the "End Section" linkage if it exists
		// This ensures we don't automatically jump to the recovery animation
		// allowing the next combo step to play naturally when this one finishes
		FHarmoniaComboAttackStep CurrentStep;
		if (GetCurrentAttackStep(CurrentStep) && CurrentStep.MontageSectionName != NAME_None)
		{
			if (USkeletalMeshComponent* Mesh = GetActorInfo().SkeletalMeshComponent.Get())
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					if (UAnimMontage* Montage = GetCurrentAttackMontage())
					{
						// Check if this is the last combo step
						// If it is, we WANT the End/Recovery section to play even if input is queued
						// This gives finishers a proper "weight" and recovery period
						const int32 CurrentIndex = MeleeCombatComponent->GetCurrentComboIndex();
						const bool bIsLastStep = (CurrentIndex >= CurrentComboSequence.ComboSteps.Num() - 1);

						if (!bIsLastStep)
						{
							// Reset next section to None (or loop) to prevent jumping to EndSection
							// Only do this for non-final steps to allow fluid chaining
							AnimInstance->Montage_SetNextSection(CurrentStep.MontageSectionName, NAME_None, Montage);
						}
					}
				}
			}
		}
	}
	
	// Start waiting for another input (in case of 3+ combo)
	StartWaitingForComboInput();
}

// ============================================================================
// Animation Callbacks
// ============================================================================

void UHarmoniaGameplayAbility_MeleeAttack::OnMontageCompleted()
{
	// Check if next combo is queued and can be executed
	if (MeleeCombatComponent && MeleeCombatComponent->IsNextComboQueued())
	{
		// Check if there's a next combo step available
		const int32 NextComboIndex = MeleeCombatComponent->GetCurrentComboIndex() + 1;
		if (CurrentComboSequence.ComboSteps.IsValidIndex(NextComboIndex))
		{
			// Advance combo and continue
			MeleeCombatComponent->EndAttack(); // This will call AdvanceCombo() since bNextComboQueued is true
			MeleeCombatComponent->StartAttack(MeleeCombatComponent->GetCurrentAttackType());
			
			// Perform next attack
			PerformMeleeAttack();
			
			// Continue listening for input
			StartWaitingForComboInput();
			return;
		}
	}
	
	// No more combo steps or not queued - end ability
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

UHarmoniaSenseComponent* UHarmoniaGameplayAbility_MeleeAttack::GetAttackComponent() const
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
			return Avatar->FindComponentByClass<UHarmoniaSenseComponent>();
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

// ============================================================================
// Charge Attack
// ============================================================================

void UHarmoniaGameplayAbility_MeleeAttack::StartCharging()
{
	bIsCharging = true;
	CurrentChargeTime = 0.0f;
	CachedChargeLevel = 0;

	const FHarmoniaChargeConfig& ChargeConfig = GetChargeConfig();

	// Play charge montage if specified
	if (ChargeConfig.ChargeMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ChargeConfig.ChargeMontage,
			1.0f,
			NAME_None,
			true,  // Loop
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageInterrupted);
			MontageTask->ReadyForActivation();
		}
	}

	// Start charge tick timer
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->GetWorldTimerManager().SetTimer(
			ChargeTickTimerHandle,
			this,
			&UHarmoniaGameplayAbility_MeleeAttack::OnChargeTick,
			0.05f, // 50ms tick
			true
		);
	}
}

void UHarmoniaGameplayAbility_MeleeAttack::OnChargeTick()
{
	if (!bIsCharging)
	{
		return;
	}

	const FHarmoniaChargeConfig& ChargeConfig = GetChargeConfig();

	// Increment charge time
	CurrentChargeTime += 0.05f;

	// Clamp to max charge time (calculated from last charge level)
	const float MaxChargeTime = ChargeConfig.GetMaxChargeTime();
	if (CurrentChargeTime > MaxChargeTime)
	{
		CurrentChargeTime = MaxChargeTime;
	}

	// Update charge level and trigger cue if level changed
	const int32 NewChargeLevel = GetCurrentChargeLevel();
	if (NewChargeLevel != CachedChargeLevel)
	{
		CachedChargeLevel = NewChargeLevel;

		// Trigger charge level cue if specified
		if (ChargeConfig.ChargeLevels.IsValidIndex(NewChargeLevel - 1))
		{
			const FHarmoniaChargeLevel& Level = ChargeConfig.ChargeLevels[NewChargeLevel - 1];
			if (Level.ChargeLevelCueTag.IsValid())
			{
				if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
				{
					ASC->ExecuteGameplayCue(Level.ChargeLevelCueTag);
				}
			}
		}
	}

	// Update charge state in combat component (for debug display)
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->UpdateChargeState(CurrentChargeTime, CachedChargeLevel, MaxChargeTime);
	}
}

void UHarmoniaGameplayAbility_MeleeAttack::ReleaseChargeAttack()
{
	// Stop charge tick timer
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->GetWorldTimerManager().ClearTimer(ChargeTickTimerHandle);
	}

	bIsCharging = false;

	// Clear charge state in combat component (for debug display)
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->ClearChargeState();
	}

	const int32 ChargeLevel = GetCurrentChargeLevel();
	const FHarmoniaChargeConfig& ChargeConfig = GetChargeConfig();

	// Determine which animation to play and damage multiplier
	FName SectionToPlay = NAME_None;
	float DamageMultiplier = 1.0f;

	if (ChargeLevel > 0 && ChargeConfig.ChargeLevels.IsValidIndex(ChargeLevel - 1))
	{
		const FHarmoniaChargeLevel& Level = ChargeConfig.ChargeLevels[ChargeLevel - 1];
		SectionToPlay = Level.ReleaseMontageSectionName;
		DamageMultiplier = Level.DamageMultiplier;
	}

	// Stop the charging montage first
	if (ChargeConfig.ChargeMontage)
	{
		if (USkeletalMeshComponent* Mesh = GetActorInfo().SkeletalMeshComponent.Get())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(0.2f, ChargeConfig.ChargeMontage);
			}
		}
	}

	// Get the release montage from combo steps (or use charge config)
	FHarmoniaComboAttackStep CurrentStep;
	if (GetCurrentAttackStep(CurrentStep) && CurrentStep.AttackMontage)
	{
		// If section name is specified in charge level, use it
		// Otherwise use the combo step's section
		FName FinalSection = (SectionToPlay != NAME_None) ? SectionToPlay : CurrentStep.MontageSectionName;

		// Play the release attack montage
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			CurrentStep.AttackMontage,
			1.0f,
			FinalSection,
			false,
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageCompleted);
			MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageCancelled);
			MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_MeleeAttack::OnMontageInterrupted);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		// No attack montage, end ability
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}

	// Reset charge state
	CurrentChargeTime = 0.0f;
	CachedChargeLevel = 0;
}

int32 UHarmoniaGameplayAbility_MeleeAttack::GetCurrentChargeLevel() const
{
	const FHarmoniaChargeConfig& ChargeConfig = GetChargeConfig();
	int32 Level = 0;

	for (int32 i = 0; i < ChargeConfig.ChargeLevels.Num(); ++i)
	{
		if (CurrentChargeTime >= ChargeConfig.ChargeLevels[i].RequiredTime)
		{
			Level = i + 1;
		}
		else
		{
			break;
		}
	}

	return Level;
}

const FHarmoniaChargeConfig& UHarmoniaGameplayAbility_MeleeAttack::GetChargeConfig() const
{
	return CurrentComboSequence.ChargeConfig;
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
