// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaGameplayTags.h"
#include "HarmoniaLogCategories.h"
#include "HarmoniaDataTableBFL.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"
#include "SensedStimulStruct.h"
#include "GameplayCueManager.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/IHarmoniaDodgeAnimInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "System/HarmoniaSoundCacheSubsystem.h"
#include "Core/HarmoniaCharacterMovementComponent.h"

namespace HarmoniaCombatASC
{
	static UAbilitySystemComponent* ResolveASCFromActor(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		// 1) Try Actor's IAbilitySystemInterface (standard approach)
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				return ASC;
			}
		}

		// 2) Try Actor's direct ASC component
		if (UAbilitySystemComponent* DirectASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			return DirectASC;
		}

		// 3) Lyra pattern: Pawn(Avatar) -> PlayerState(Owner) ASC
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (APlayerState* PS = Pawn->GetPlayerState())
			{
				if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(PS))
				{
					if (UAbilitySystemComponent* PS_ASC = PS_ASI->GetAbilitySystemComponent())
					{
						return PS_ASC;
					}
				}

				if (UAbilitySystemComponent* PS_DirectASC = PS->FindComponentByClass<UAbilitySystemComponent>())
				{
					return PS_DirectASC;
				}
			}
		}

		// 4) Additional fallback: check for any attached ASC
		// (ChildActor, attached components, etc.)
		AActor* ParentActor = Actor->GetAttachParentActor();
		while (ParentActor)
		{
			// Check IAbilitySystemInterface on parent

			if (IAbilitySystemInterface* ParentASI = Cast<IAbilitySystemInterface>(ParentActor))
			{
				if (UAbilitySystemComponent* ParentASC = ParentASI->GetAbilitySystemComponent())
				{
					return ParentASC;
				}
			}

			// Check ASC component directly on parent

			if (UAbilitySystemComponent* ParentDirectASC = ParentActor->FindComponentByClass<UAbilitySystemComponent>())
			{
				return ParentDirectASC;
			}

			// Move up to next parent
			ParentActor = ParentActor->GetAttachParentActor();
		}

		// 5) Try Owner's ASC (for ChildActorComponent cases)
		if (AActor* OwnerActor = Actor->GetOwner())
		{
			if (OwnerActor != Actor)
			{
				if (IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(OwnerActor))
				{
					if (UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent())
					{
						return OwnerASC;
					}
				}

				if (UAbilitySystemComponent* OwnerDirectASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>())
				{
					return OwnerDirectASC;
				}
			}
		}

		return nullptr;
	}
}

UHarmoniaMeleeCombatComponent::UHarmoniaMeleeCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick disabled (event-driven updates only)

	// Enable network replication for Client RPCs (e.g., ClientReceiveHitReaction)
	SetIsReplicatedByDefault(true);

	AttackingTag = HarmoniaGameplayTags::State_Combat_Attacking;
	BlockingTag = HarmoniaGameplayTags::State_Dodging;  // Use State_Dodging for now, Blocking tag not defined
	ParryingTag = HarmoniaGameplayTags::State_Combat_Parrying;
	DodgingTag = HarmoniaGameplayTags::State_Dodging;
	InvulnerableTag = HarmoniaGameplayTags::State_Invincible;
}

void UHarmoniaMeleeCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaMeleeCombatComponent, bIsAttacking);
	DOREPLIFETIME(UHarmoniaMeleeCombatComponent, CurrentAttackType);
	DOREPLIFETIME(UHarmoniaMeleeCombatComponent, CurrentComboIndex);
	DOREPLIFETIME(UHarmoniaMeleeCombatComponent, bNextComboQueued);
}

void UHarmoniaMeleeCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Ticking is disabled by default (bCanEverTick = false)
}

void UHarmoniaMeleeCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ComboSequencesDataTable)
	{
		ComboSequencesDataTable = UHarmoniaDataTableBFL::GetComboAttackDataTable();
	}

	RefreshCachedCombos();

	if (AActor* Owner = GetOwner())
	{
		if (UHarmoniaEquipmentComponent* EquipComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>())
		{
			EquipComp->OnEquipmentChanged.AddDynamic(this, &UHarmoniaMeleeCombatComponent::OnEquipmentChanged);
		}
	}
}

void UHarmoniaMeleeCombatComponent::OnEquipmentChanged(EEquipmentSlot Slot, const FHarmoniaID& OldId, const FHarmoniaID& NewId)
{
	if (Slot == EEquipmentSlot::MainHand)
	{
		RefreshCachedCombos();
	}
}

FGameplayTag UHarmoniaMeleeCombatComponent::GetCurrentWeaponTypeTag() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UHarmoniaEquipmentComponent* EquipComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>())
		{
			return EquipComp->GetMainHandWeaponTypeTag();
		}
	}

	return FGameplayTag::RequestGameplayTag(FName("Weapon.Type.Fist"), false);
}

UAbilitySystemComponent* UHarmoniaMeleeCombatComponent::GetAbilitySystemComponent() const
{
	// If Owner is Pawn, ASC might be on PlayerState
	return HarmoniaCombatASC::ResolveASCFromActor(GetOwner());
}

UHarmoniaAttributeSet* UHarmoniaMeleeCombatComponent::GetAttributeSet() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return const_cast<UHarmoniaAttributeSet*>(ASC->GetSet<UHarmoniaAttributeSet>());
	}
	return nullptr;
}

// ============================================================================
// Combat State
// ============================================================================

void UHarmoniaMeleeCombatComponent::SetDefenseState(EHarmoniaDefenseState NewState)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (DefenseState == NewState)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	switch (DefenseState)
	{
	case EHarmoniaDefenseState::Blocking: ASC->RemoveLooseGameplayTag(BlockingTag); break;
	case EHarmoniaDefenseState::Parrying: ASC->RemoveLooseGameplayTag(ParryingTag); break;
	case EHarmoniaDefenseState::Dodging:  ASC->RemoveLooseGameplayTag(DodgingTag);  break;
	default: break;
	}

	DefenseState = NewState;

	switch (DefenseState)
	{
	case EHarmoniaDefenseState::Blocking: ASC->AddLooseGameplayTag(BlockingTag); break;
	case EHarmoniaDefenseState::Parrying: ASC->AddLooseGameplayTag(ParryingTag); break;
	case EHarmoniaDefenseState::Dodging:  ASC->AddLooseGameplayTag(DodgingTag);  break;
	default: break;
	}
}

void UHarmoniaMeleeCombatComponent::SetInvulnerable(bool bInvulnerable, float Duration)
{
	if (bInIFrames == bInvulnerable)
	{
		return;
	}

	bInIFrames = bInvulnerable;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !GetWorld())
	{
		return;
	}

	if (bInIFrames)
	{
		ASC->AddLooseGameplayTag(InvulnerableTag);

		if (Duration > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(
				IFrameTimerHandle,
				this,
				&UHarmoniaMeleeCombatComponent::ClearInvulnerability,
				Duration,
				false
			);
		}
	}
	else
	{
		ASC->RemoveLooseGameplayTag(InvulnerableTag);
		GetWorld()->GetTimerManager().ClearTimer(IFrameTimerHandle);
	}
}

void UHarmoniaMeleeCombatComponent::ClearInvulnerability()
{
	SetInvulnerable(false);
}

void UHarmoniaMeleeCombatComponent::SetInAttackWindow(bool bInWindow)
{
	bInAttackWindow = bInWindow;

	// Always clear hit tracking to ensure fresh hit detection for each attack
	HitActorsThisAttack.Empty();
}

// ============================================================================
// Combo System (attack chain management)
// ============================================================================

int32 UHarmoniaMeleeCombatComponent::GetMaxComboCount() const
{
	FHarmoniaComboAttackSequence ComboSequence;
	if (GetComboSequence(CurrentAttackType, ComboSequence))
	{
		return ComboSequence.ComboSteps.Num();
	}
	return 3;
}

void UHarmoniaMeleeCombatComponent::AdvanceCombo()
{
	CurrentComboIndex++;
}

void UHarmoniaMeleeCombatComponent::ResetCombo()
{
	CurrentComboIndex = 0;
	bNextComboQueued = false;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
	}
}

bool UHarmoniaMeleeCombatComponent::IsInComboWindow() const
{
	return GetWorld() ? GetWorld()->GetTimerManager().IsTimerActive(ComboWindowTimerHandle) : false;
}

void UHarmoniaMeleeCombatComponent::OpenComboWindow(float Duration)
{
	if (Duration > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ComboWindowTimerHandle,
			this,
			&UHarmoniaMeleeCombatComponent::OnComboWindowExpired,
			Duration,
			false
		);
	}
}

void UHarmoniaMeleeCombatComponent::QueueNextCombo()
{
	if (IsInComboWindow())
	{
		bNextComboQueued = true;
		
		// If we're the client, notify the server
		AActor* Owner = GetOwner();
		if (Owner && !Owner->HasAuthority())
		{
			ServerQueueNextCombo();
		}
	}
}

void UHarmoniaMeleeCombatComponent::ServerQueueNextCombo_Implementation()
{
	// Server receives the combo queue request from client
	if (IsInComboWindow())
	{
		bNextComboQueued = true;
	}
}

void UHarmoniaMeleeCombatComponent::OnComboWindowExpired()
{
	// Preserve queued state
}

bool UHarmoniaMeleeCombatComponent::GetComboSequence(EHarmoniaAttackType AttackType, FHarmoniaComboAttackSequence& OutSequence) const
{
	if (bComboCacheValid)
	{
		const FHarmoniaComboAttackSequence* CachedSequence = CachedCombos.Find(AttackType);
		if (CachedSequence && CachedSequence->ComboSteps.Num() > 0)
		{
			OutSequence = *CachedSequence;
			return true;
		}
		return false;
	}

	if (!ComboSequencesDataTable)
	{
		return false;
	}

	const FString ContextString = TEXT("GetComboSequence");
	TArray<FHarmoniaComboAttackSequence*> AllRows;
	ComboSequencesDataTable->GetAllRows<FHarmoniaComboAttackSequence>(ContextString, AllRows);

	const FGameplayTag CurrentWeaponTag = GetCurrentWeaponTypeTag();

	for (const FHarmoniaComboAttackSequence* Row : AllRows)
	{
		if (Row && Row->WeaponTypeTag.MatchesTagExact(CurrentWeaponTag) && Row->AttackType == AttackType)
		{
			if (OwnerTypeTag.IsValid() && Row->OwnerTypeTag.IsValid())
			{
				if (Row->OwnerTypeTag.MatchesTag(OwnerTypeTag))
				{
					OutSequence = *Row;
					return true;
				}
			}
			else if (!OwnerTypeTag.IsValid() && !Row->OwnerTypeTag.IsValid())
			{
				OutSequence = *Row;
				return true;
			}
		}
	}

	return false;
}

void UHarmoniaMeleeCombatComponent::RefreshCachedCombos()
{
	bComboCacheValid = false;
	CachedCombos.Empty();

	if (!ComboSequencesDataTable)
	{
		return;
	}

	const FString ContextString = TEXT("RefreshCachedCombos");
	TArray<FHarmoniaComboAttackSequence*> AllRows;
	ComboSequencesDataTable->GetAllRows<FHarmoniaComboAttackSequence>(ContextString, AllRows);

	const FGameplayTag CurrentWeaponTag = GetCurrentWeaponTypeTag();

	for (const FHarmoniaComboAttackSequence* Row : AllRows)
	{
		if (!Row || !Row->WeaponTypeTag.MatchesTagExact(CurrentWeaponTag))
		{
			continue;
		}

		bool bOwnerMatch = false;
		if (OwnerTypeTag.IsValid() && Row->OwnerTypeTag.IsValid())
		{
			bOwnerMatch = Row->OwnerTypeTag.MatchesTag(OwnerTypeTag);
		}
		else if (!OwnerTypeTag.IsValid() && !Row->OwnerTypeTag.IsValid())
		{
			bOwnerMatch = true;
		}

		if (bOwnerMatch)
		{
			if (!CachedCombos.Contains(Row->AttackType))
			{
				CachedCombos.Add(Row->AttackType, *Row);
			}
		}
	}

	bComboCacheValid = true;
}

bool UHarmoniaMeleeCombatComponent::GetCurrentComboAttackData(FHarmoniaAttackData& OutAttackData) const
{
	FHarmoniaComboAttackSequence ComboSequence;
	if (!GetComboSequence(CurrentAttackType, ComboSequence))
	{
		return false;
	}

	if (!ComboSequence.ComboSteps.IsValidIndex(CurrentComboIndex))
	{
		return false;
	}

	const FHarmoniaComboAttackStep& ComboStep = ComboSequence.ComboSteps[CurrentComboIndex];

	if (ComboStep.bUseAttackDataOverride)
	{
		OutAttackData = ComboStep.AttackDataOverride;
		OutAttackData.DamageConfig.DamageMultiplier *= ComboStep.DamageMultiplier;
	}
	else
	{
		OutAttackData = FHarmoniaAttackData();
		OutAttackData.DamageConfig.DamageMultiplier = ComboStep.DamageMultiplier;
	}

	return true;
}

// ============================================================================
// Attack Execution
// ============================================================================

bool UHarmoniaMeleeCombatComponent::RequestLightAttack()
{
	if (!CanAttack())
	{
		return false;
	}

	if (IsInComboWindow())
	{
		QueueNextCombo();
		return true;
	}

	StartAttack(EHarmoniaAttackType::Light);
	return true;
}

bool UHarmoniaMeleeCombatComponent::RequestHeavyAttack()
{
	if (!CanAttack())
	{
		return false;
	}

	if (IsInComboWindow() && IsAttacking())
	{
		QueueNextCombo();
		return true;
	}

	StartAttack(EHarmoniaAttackType::Heavy);
	return true;
}

void UHarmoniaMeleeCombatComponent::StartAttack(EHarmoniaAttackType InAttackType)
{
	bIsAttacking = true;

	// Reset combo state on new attack
	HitActorsThisAttack.Empty();

	if (CurrentAttackType != InAttackType)
	{
		CurrentComboIndex = 0;
		bNextComboQueued = false;
	}

	CurrentAttackType = InAttackType;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(AttackingTag);
	}
}

void UHarmoniaMeleeCombatComponent::EndAttack()
{
	bIsAttacking = false;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(AttackingTag))
		{
			ASC->RemoveLooseGameplayTag(AttackingTag);
		}
	}

	if (bNextComboQueued)
	{
		AdvanceCombo();
		bNextComboQueued = false;
	}
	else if (!IsInComboWindow())
	{
		ResetCombo();
	}
}

bool UHarmoniaMeleeCombatComponent::CanAttack() const
{
	if (DefenseState == EHarmoniaDefenseState::Blocking || DefenseState == EHarmoniaDefenseState::Stunned)
	{
		return false;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (AttackBlockedTag.IsValid() && ASC->HasMatchingGameplayTag(AttackBlockedTag))
		{
			return false;
		}
	}

	return true;
}

// ============================================================================
// Defense helpers
// ============================================================================

bool UHarmoniaMeleeCombatComponent::IsDefenseAngleValid(const FVector& AttackerLocation) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	FVector DefenderForward = Owner->GetActorForwardVector();
	DefenderForward.Z = 0.0f;
	DefenderForward.Normalize();

	FVector ToAttacker = AttackerLocation - Owner->GetActorLocation();
	ToAttacker.Z = 0.0f;

	if (ToAttacker.IsNearlyZero())
	{
		return true;
	}

	ToAttacker.Normalize();

	const float DotProduct = FVector::DotProduct(DefenderForward, ToAttacker);
	const float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
	const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	const float HalfDefenseAngle = DefenseAngle / 2.0f;
	return AngleDegrees <= HalfDefenseAngle;
}

void UHarmoniaMeleeCombatComponent::OnAttackBlocked(AActor* Attacker, float Damage, const FVector& ImpactPoint)
{
	OnBlockedAttack.Broadcast(Attacker, Damage, ImpactPoint);

	if (UHarmoniaAttributeSet* AttributeSet = GetAttributeSet())
	{
		if (AttributeSet->GetStamina() <= 0.0f)
		{
			SetDefenseState(EHarmoniaDefenseState::Stunned);
			UE_LOG(LogHarmoniaCombat, Log, TEXT("Guard broken - stunned state applied"));
		}
	}
}

void UHarmoniaMeleeCombatComponent::OnParrySuccess(AActor* Attacker)
{
	StartRiposteWindow(Attacker, DefaultRiposteConfig.RiposteWindowDuration);
	UE_LOG(LogHarmoniaCombat, Log, TEXT("Parry successful - attacker %s is vulnerable to riposte"), *GetNameSafe(Attacker));
}

// ============================================================================
// Backstab / Riposte (positional attacks)
// ============================================================================

void UHarmoniaMeleeCombatComponent::StartRiposteWindow(AActor* ParriedTargetActor, float Duration)
{
	if (!ParriedTargetActor || !GetWorld())
	{
		return;
	}

	ParriedTarget = ParriedTargetActor;
	SetDefenseState(EHarmoniaDefenseState::RiposteWindow);

	const float WindowDuration = (Duration > 0.0f) ? Duration : DefaultRiposteConfig.RiposteWindowDuration;
	GetWorld()->GetTimerManager().SetTimer(
		RiposteWindowTimerHandle,
		this,
		&UHarmoniaMeleeCombatComponent::OnRiposteWindowExpired,
		WindowDuration,
		false
	);
}

void UHarmoniaMeleeCombatComponent::EndRiposteWindow()
{
	SetDefenseState(EHarmoniaDefenseState::None);
	ParriedTarget.Reset();
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RiposteWindowTimerHandle);
	}
}

void UHarmoniaMeleeCombatComponent::OnRiposteWindowExpired()
{
	EndRiposteWindow();
}

// Damage Processing
// ============================================================================


void UHarmoniaMeleeCombatComponent::ResetHitTracking()
{
	HitActorsThisAttack.Empty();
	UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] HitActorsThisAttack reset for new attack"));
}

void UHarmoniaMeleeCombatComponent::ApplyDamageToTarget(AActor* TargetActor, const FVector& HitLocation)
{
	AActor* Owner = GetOwner();
	if (!Owner || !TargetActor || TargetActor == Owner)
	{
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] Early return: Invalid Owner/Target"));
		return;
	}

	if (!Cast<APawn>(TargetActor))
	{
		return;
	}


	if (HitActorsThisAttack.Contains(TargetActor))
	{
		return; // Already hit this actor in current attack
	}
	HitActorsThisAttack.Add(TargetActor);

	if (!Owner->HasAuthority())
	{
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] Early return: No Authority"));
		return;
	}

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Combat] Early return: No OwnerASC"));
		return;
	}

	UAbilitySystemComponent* TargetASC = HarmoniaCombatASC::ResolveASCFromActor(TargetActor);
	if (!TargetASC)
	{
		// Pawn without ASC is unusual - log as warning
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Combat] Early return: No TargetASC for Pawn (%s)"), *GetNameSafe(TargetActor));
		return;
	}

	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] Target is invulnerable, skipping"));
		return;
	}

	// === Parry Check ===
	if (UHarmoniaMeleeCombatComponent* TargetCombat = TargetActor->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
	{
		if (TargetCombat->TryParry(Owner, HitLocation))
		{
			UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] PARRY SUCCESS! Damage blocked for %s"), *GetNameSafe(TargetActor));
			return; // Parry successful - no damage applied
		}
	}

	FHarmoniaAttackData AttackData;
	if (!GetCurrentComboAttackData(AttackData))
	{
		AttackData.DamageConfig.DamageMultiplier = 1.0f;
	}

	float DamageMultiplier = AttackData.DamageConfig.DamageMultiplier;

	if (IsBackstabAttack(TargetActor, Owner->GetActorLocation()))
	{
		DamageMultiplier *= GetBackstabDamageMultiplier();
	}

	if (AttackData.DamageConfig.bCanCritical)
	{
		float CritChance = AttackData.DamageConfig.CriticalChance;
		float CritMultiplier = AttackData.DamageConfig.CriticalMultiplier;

		if (const UHarmoniaAttributeSet* AttributeSet = OwnerASC->GetSet<UHarmoniaAttributeSet>())
		{
			CritChance = AttributeSet->GetCriticalChance();
			CritMultiplier = AttributeSet->GetCriticalDamage();
		}

		if (FMath::FRand() < CritChance)
		{
			DamageMultiplier *= CritMultiplier;
		}
	}

	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(Owner, Owner);

	if (!HitLocation.IsZero())
	{
		FHitResult HitResult;
		HitResult.ImpactPoint = HitLocation;
		HitResult.Location = HitLocation;
		EffectContext.AddHitResult(HitResult);
	}

	// Debug: Show which DamageEffectClass is being used
	TSubclassOf<UGameplayEffect> EffectToApply = AttackData.DamageConfig.DamageEffectClass ? AttackData.DamageConfig.DamageEffectClass : DamageEffectClass;

	if (!EffectToApply)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Combat] Early return: No DamageEffectClass!"));
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(EffectToApply, 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Combat] Early return: Invalid SpecHandle"));
		return;
	}

	const FGameplayTag DamageMultiplierTag = HarmoniaGameplayTags::SetByCaller_DamageMultiplier.GetTag();
	if (DamageMultiplierTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierTag, DamageMultiplier);
	}
	OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	for (const TSubclassOf<UGameplayEffect>& AdditionalEffect : AdditionalHitEffects)
	{
		if (AdditionalEffect)
		{
			FGameplayEffectSpecHandle ExtraSpec = OwnerASC->MakeOutgoingSpec(AdditionalEffect, 1.0f, EffectContext);
			if (ExtraSpec.IsValid())
			{
				OwnerASC->ApplyGameplayEffectSpecToTarget(*ExtraSpec.Data.Get(), TargetASC);
			}
		}
	}

	if (HitGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitLocation.IsZero() ? TargetActor->GetActorLocation() : HitLocation;
		CueParams.Normal = (Owner->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
		CueParams.Instigator = Owner;
		CueParams.EffectCauser = Owner;
		TargetASC->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
	}

	if (UHarmoniaMeleeCombatComponent* TargetCombat = TargetActor->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
	{
		FSoftClassPath CameraShakePath;
		float CameraShakeScale = 1.0f;
		
		if (AttackData.HitReactionConfig.CameraShakeClass)
		{
			CameraShakePath = FSoftClassPath(AttackData.HitReactionConfig.CameraShakeClass);
			CameraShakeScale = AttackData.HitReactionConfig.CameraShakeScale;
		}
		
		TargetCombat->ClientReceiveHitReaction(DamageMultiplier, HitLocation, CameraShakePath, CameraShakeScale);
	}

	if (AttackData.HitReactionConfig.AttackerCameraShakeClass)
	{
		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
			{
				PC->ClientStartCameraShake(
					AttackData.HitReactionConfig.AttackerCameraShakeClass,
					AttackData.HitReactionConfig.AttackerCameraShakeScale
				);
			}
		}
	}
}

// ============================================================================
// Defense (missing implementations)
// ============================================================================

bool UHarmoniaMeleeCombatComponent::CanBlock() const
{
	if (bIsAttacking || DefenseState != EHarmoniaDefenseState::None)
	{
		return false;
	}

	// Check for stamina if needed
	if (const UHarmoniaAttributeSet* AttributeSet = GetAttributeSet())
	{
		if (AttributeSet->GetStamina() <= 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaMeleeCombatComponent::CanParry() const
{
	// Only block parry while attacking - riposte window should NOT prevent new parry attempts
	if (bIsAttacking)
	{
		return false;
	}

	if (const UHarmoniaAttributeSet* AttributeSet = GetAttributeSet())
	{
		if (AttributeSet->GetStamina() <= 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaMeleeCombatComponent::CanDodge() const
{
	// Check if attacking
	if (bIsAttacking)
	{
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Dodge] Failed: Currently attacking"));
		return false;
	}

	// Check if stunned
	if (DefenseState == EHarmoniaDefenseState::Stunned)
	{
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Dodge] Failed: Currently stunned"));
		return false;
	}

	// Check if grounded
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (const UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
		{
			if (!MovementComp->IsMovingOnGround())
			{
				UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Dodge] Failed: Not on ground (IsFalling=%d, MovementMode=%d)"),
					MovementComp->IsFalling(), static_cast<int32>(MovementComp->MovementMode));
				return false;
			}
		}
	}

	// Note: Stamina check is handled by GA_Dodge Cost GE

	UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Dodge] Success: All conditions passed"));
	return true;
}

// ============================================================================
// Dodge Animation (BlendSpace support with interface)
// ============================================================================

void UHarmoniaMeleeCombatComponent::SetDodgeDirection(float DirectionX, float DirectionY)
{
	CurrentDodgeDirection = FVector2D(FMath::Clamp(DirectionX, -1.0f, 1.0f), FMath::Clamp(DirectionY, -1.0f, 1.0f));

	// Find AnimInstance and set dodge params via interface
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// Use IHarmoniaDodgeAnimInterface (works with HarmoniaAnimInstance, custom ALS implementations, etc.)
	if (AnimInstance->Implements<UHarmoniaDodgeAnimInterface>())
	{
		IHarmoniaDodgeAnimInterface::Execute_SetDodgeDirection(AnimInstance, CurrentDodgeDirection.X, CurrentDodgeDirection.Y);
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Dodge] Set direction via IHarmoniaDodgeAnimInterface: X=%.2f, Y=%.2f"),
			CurrentDodgeDirection.X, CurrentDodgeDirection.Y);
	}
	else
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Dodge] AnimInstance does not implement IHarmoniaDodgeAnimInterface. Use UHarmoniaAnimInstance or implement the interface."));
	}
}

void UHarmoniaMeleeCombatComponent::ClearDodgeDirection()
{
	CurrentDodgeDirection = FVector2D::ZeroVector;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (AnimInstance->Implements<UHarmoniaDodgeAnimInterface>())
	{
		IHarmoniaDodgeAnimInterface::Execute_ClearDodgeDirection(AnimInstance);
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Dodge] Cleared direction via IHarmoniaDodgeAnimInterface"));
	}
}

// ============================================================================
// Riposte (missing implementations)
// ============================================================================

bool UHarmoniaMeleeCombatComponent::CanRiposte() const
{
	return DefenseState == EHarmoniaDefenseState::RiposteWindow && ParriedTarget.IsValid();
}

float UHarmoniaMeleeCombatComponent::GetRiposteWindowDuration() const
{
	return DefaultRiposteConfig.RiposteWindowDuration;
}

AActor* UHarmoniaMeleeCombatComponent::GetParriedTarget() const
{
	return ParriedTarget.Get();
}

void UHarmoniaMeleeCombatComponent::ClearParriedTarget()
{
	ParriedTarget.Reset();
}

float UHarmoniaMeleeCombatComponent::GetRiposteDamageMultiplier() const
{
	return DefaultRiposteConfig.RiposteDamageMultiplier;
}

// ============================================================================
// Backstab (missing implementations)
// ============================================================================

bool UHarmoniaMeleeCombatComponent::IsBackstabAttack(AActor* Target, FVector AttackOrigin) const
{
	if (!Target || !DefaultBackstabConfig.bEnableBackstab)
	{
		return false;
	}

	FVector TargetForward = Target->GetActorForwardVector();
	TargetForward.Z = 0.0f;
	TargetForward.Normalize();

	FVector ToAttacker = AttackOrigin - Target->GetActorLocation();
	ToAttacker.Z = 0.0f;

	if (ToAttacker.IsNearlyZero())
	{
		return false;
	}

	ToAttacker.Normalize();

	// Attacker should be behind the target (dot product < 0 means behind)
	const float DotProduct = FVector::DotProduct(TargetForward, ToAttacker);

	// Convert to angle
	const float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
	const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// If angle from forward is greater than (180 - BackstabAngleTolerance/2), attacker is in backstab arc
	const float BackstabThreshold = 180.0f - (DefaultBackstabConfig.BackstabAngleTolerance / 2.0f);
	return AngleDegrees >= BackstabThreshold;
}

float UHarmoniaMeleeCombatComponent::GetBackstabDamageMultiplier() const
{
	return DefaultBackstabConfig.BackstabDamageMultiplier;
}

// ============================================================================
// Network Hit Reaction
// ============================================================================

void UHarmoniaMeleeCombatComponent::ClientReceiveHitReaction_Implementation(float Damage, const FVector& HitLocation, const FSoftClassPath& CameraShakePath, float CameraShakeScale)
{
	// Apply camera shake on owning client using the path from combo sequence table
	if (CameraShakePath.IsValid())
	{
		if (UClass* CameraShakeClass = CameraShakePath.TryLoadClass<UCameraShakeBase>())
		{
			if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
			{
				if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
				{
					PC->ClientStartCameraShake(TSubclassOf<UCameraShakeBase>(CameraShakeClass), CameraShakeScale);
				}
			}
		}
	}

	// Trigger local HitReaction GameplayEvent for any listening abilities
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayEventData HitEventData;
		HitEventData.EventMagnitude = Damage;

		ASC->HandleGameplayEvent(HarmoniaGameplayTags::GameplayEvent_HitReaction, &HitEventData);
	}
}

// ============================================================================
// Attack Animation Replication
// ============================================================================

void UHarmoniaMeleeCombatComponent::MulticastPlayAttackMontage_Implementation(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage)
	{
		return;
	}

	// Skip on autonomous proxy - they already play locally via ability task
	AActor* Owner = GetOwner();
	if (Owner && Owner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (!OwnerCharacter)
	{
		return;
	}

	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
		
		if (SectionName != NAME_None)
		{
			AnimInstance->Montage_JumpToSection(SectionName, Montage);
		}
	}
}

void UHarmoniaMeleeCombatComponent::ServerPlayAttackMontage_Implementation(UAnimMontage* Montage, FName SectionName)
{
	// Server receives request from client - broadcast to all clients
	MulticastPlayAttackMontage(Montage, SectionName);
}

// ============================================================================
// Parry System Implementation
// ============================================================================

void UHarmoniaMeleeCombatComponent::RegisterParryableAttacker(AActor* Attacker, float MontageStartTime, const FHarmoniaParryConfig& Config)
{
	if (!Attacker)
	{
		return;
	}

	FParryableAttackerInfo Info;
	Info.ParryConfig = Config;
	Info.ParryWindowStartTime = MontageStartTime;
	Info.ParryWindowEndTime = MontageStartTime + 10.0f; // Will be updated by NotifyEnd
	Info.RegistrationTime = GetWorld()->GetTimeSeconds();
	Info.bParrySucceeded = false;

	ParryableAttackers.Add(Attacker, Info);

	UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] Attacker registered: %s, StartTime: %.3f"), 
		*GetNameSafe(Attacker), MontageStartTime);
}

void UHarmoniaMeleeCombatComponent::UpdateParryableEndTime(AActor* Attacker, float MontageEndTime)
{
	if (FParryableAttackerInfo* Info = ParryableAttackers.Find(Attacker))
	{
		Info->ParryWindowEndTime = MontageEndTime;

		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] EndTime updated: %s, Range: %.3f - %.3f"), 
			*GetNameSafe(Attacker), Info->ParryWindowStartTime, MontageEndTime);
	}
}

void UHarmoniaMeleeCombatComponent::UnregisterParryableAttacker(AActor* Attacker)
{
	if (ParryableAttackers.Remove(Attacker) > 0)
	{
		UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] Attacker unregistered: %s"), *GetNameSafe(Attacker));
	}
}

bool UHarmoniaMeleeCombatComponent::TryParry(AActor* Attacker, const FVector& HitLocation)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Attacker)
	{
		return false;
	}

	// 1. Check if defender has Parrying tag (ASC check)
	UAbilitySystemComponent* DefenderASC = GetAbilitySystemComponent();
	if (!DefenderASC || !DefenderASC->HasMatchingGameplayTag(ParryingTag))
	{
		return false;
	}

	// 2. Check if attacker has Parryable tag (ASC check)
	UAbilitySystemComponent* AttackerASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Attacker);
	if (!AttackerASC || !AttackerASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_Combat_Parryable))
	{
		return false;
	}

	// 3. Defense angle valid? (attacker in front)
	if (!IsDefenseAngleValid(Attacker->GetActorLocation()))
	{
		return false;
	}

	// === PARRY SUCCESS! ===
	UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] SUCCESS! Defender: %s, Attacker: %s"), 
		*GetNameSafe(Owner), *GetNameSafe(Attacker));

	// Get ParryConfig from attacker's MeleeCombatComponent (set by AnimNotifyState)
	FHarmoniaParryConfig ConfigToUse = DefaultParryConfig;
	if (UHarmoniaMeleeCombatComponent* AttackerMeleeComp = Attacker->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
	{
		ConfigToUse = AttackerMeleeComp->GetCurrentParryConfig();
	}

	ExecuteParrySuccess(Attacker, HitLocation, ConfigToUse);
	return true;
}

void UHarmoniaMeleeCombatComponent::ExecuteParrySuccess(AActor* Attacker, const FVector& ImpactPoint, const FHarmoniaParryConfig& Config)
{
	AActor* Owner = GetOwner();
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponent();
	UAbilitySystemComponent* AttackerASC = HarmoniaCombatASC::ResolveASCFromActor(Attacker);

	// Debug: Check if Config has effects set
	UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] ExecuteParrySuccess - VFX: %s, CameraShake: %s, HitStop: %s"),
		Config.ParryVFX ? TEXT("SET") : TEXT("NULL"),
		Config.ParryCameraShakeClass ? TEXT("SET") : TEXT("NULL"),
		Config.bUseHitStop ? TEXT("ON") : TEXT("OFF"));


	// === 1. Knockback (using Custom Movement Mode) ===
	FVector KnockbackDir = Owner->GetActorLocation() - Attacker->GetActorLocation();
	KnockbackDir.Z = 0.0f;  // Keep knockback horizontal only
	KnockbackDir = KnockbackDir.GetSafeNormal();
	const float KnockbackDuration = 0.3f;  // Short knockback duration
	
	// Defender knockback (backward - away from attacker)
	if (ACharacter* OwnerChar = Cast<ACharacter>(Owner))
	{
		if (UHarmoniaCharacterMovementComponent* HarmoniaMovement = Cast<UHarmoniaCharacterMovementComponent>(OwnerChar->GetCharacterMovement()))
		{
			HarmoniaMovement->StartKnockback(KnockbackDir, Config.DefenderKnockbackSpeed, KnockbackDuration);
			UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] Defender knockback: Speed=%.1f, Duration=%.2f"), 
				Config.DefenderKnockbackSpeed, KnockbackDuration);
		}
	}
	
	// Attacker knockback (backward - away from defender)
	if (ACharacter* AttackerChar = Cast<ACharacter>(Attacker))
	{
		if (UHarmoniaCharacterMovementComponent* HarmoniaMovement = Cast<UHarmoniaCharacterMovementComponent>(AttackerChar->GetCharacterMovement()))
		{
			HarmoniaMovement->StartKnockback(-KnockbackDir, Config.AttackerKnockbackSpeed, KnockbackDuration);
			UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] Attacker knockback: Speed=%.1f, Duration=%.2f"), 
				Config.AttackerKnockbackSpeed, KnockbackDuration);
		}
	}

	// === 2. Calculate effect location (used for VFX and Sound) ===
	// First try to use ImpactPoint if valid
	FVector EffectLocation = ImpactPoint;
	FRotator EffectRotation = KnockbackDir.Rotation();
	
	// If ImpactPoint is invalid, try weapon socket
	if (EffectLocation.IsNearlyZero())
	{
		if (UHarmoniaEquipmentComponent* EquipComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>())
		{
			if (USkeletalMeshComponent* WeaponMesh = EquipComp->GetEquippedMeshComponent(EEquipmentSlot::MainHand))
			{
				static const TArray<FName> PreferredSockets = { FName("blade_tip"), FName("blade_mid"), FName("weapon_tip"), FName("fx") };
				for (const FName& SocketName : PreferredSockets)
				{
					if (WeaponMesh->DoesSocketExist(SocketName))
					{
						EffectLocation = WeaponMesh->GetSocketLocation(SocketName);
						EffectRotation = WeaponMesh->GetSocketRotation(SocketName);
						break;
					}
				}
			}
		}
	}
	
	// Final fallback: midpoint between attacker and defender
	if (EffectLocation.IsNearlyZero())
	{
		EffectLocation = (Owner->GetActorLocation() + Attacker->GetActorLocation()) * 0.5f;
		EffectLocation.Z = Owner->GetActorLocation().Z + 80.0f;
	}

	// === 3. VFX ===
	if (Config.ParryVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Config.ParryVFX,
			EffectLocation,
			EffectRotation,
			Config.ParryVFXScale,
			true,
			true,
			ENCPoolMethod::AutoRelease
		);

		UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] VFX spawned: %s at location %s"), 
			*GetNameSafe(Config.ParryVFX), *EffectLocation.ToString());
	}

	// === 4. Sound (data-driven from equipment) ===
	if (UHarmoniaEquipmentComponent* EquipComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>())
	{
		FGameplayTag ParrySoundTag = EquipComp->GetMainHandCombatSoundTag(ECombatSoundType::Parry);
		if (ParrySoundTag.IsValid())
		{
			if (UHarmoniaSoundCacheSubsystem* SoundSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UHarmoniaSoundCacheSubsystem>())
			{
				SoundSubsystem->PlaySoundAtLocationByTag(this, ParrySoundTag, EffectLocation);
				
				UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] Sound played: %s at %s"), 
					*ParrySoundTag.ToString(), *EffectLocation.ToString());
			}
		}
		else
		{
			UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Combat] No ParrySoundTag set in equipment data"));
		}
	}

	// === 3. Camera shake ===
	if (Config.bUseCameraShake && Config.ParryCameraShakeClass)
	{
		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
			{
				PC->ClientStartCameraShake(Config.ParryCameraShakeClass, Config.ParryCameraShakeScale);

				UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] CameraShake played: Scale=%.2f"), 
					Config.ParryCameraShakeScale);
			}
		}
	}

	// === 4. Hit Stop (Time Dilation) ===
	if (Config.bUseHitStop && Config.HitStopDuration > 0.0f)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// Apply time dilation to both characters
			if (ACharacter* OwnerChar = Cast<ACharacter>(Owner))
			{
				OwnerChar->CustomTimeDilation = Config.HitStopTimeDilation;
			}
			if (ACharacter* AttackerChar = Cast<ACharacter>(Attacker))
			{
				AttackerChar->CustomTimeDilation = Config.HitStopTimeDilation;
			}

			// Set timer to restore normal time
			FTimerHandle HitStopTimerHandle;
			TWeakObjectPtr<AActor> WeakOwner = Owner;
			TWeakObjectPtr<AActor> WeakAttacker = Attacker;
			
			World->GetTimerManager().SetTimer(
				HitStopTimerHandle,
				[WeakOwner, WeakAttacker]()
				{
					if (ACharacter* OwnerChar = Cast<ACharacter>(WeakOwner.Get()))
					{
						OwnerChar->CustomTimeDilation = 1.0f;
					}
					if (ACharacter* AttackerChar = Cast<ACharacter>(WeakAttacker.Get()))
					{
						AttackerChar->CustomTimeDilation = 1.0f;
					}
				},
				Config.HitStopDuration,
				false
			);

			UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] HitStop applied: Duration=%.3f, TimeDilation=%.2f"), 
				Config.HitStopDuration, Config.HitStopTimeDilation);
		}
	}

	// === 5. Defender reward GE (stamina recovery) ===
	if (Config.DefenderRewardEffect && OwnerASC)
	{
		FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(Config.DefenderRewardEffect, 1.0f, Context);
		if (Spec.IsValid())
		{
			OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

			UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Combat] Defender reward GE applied: %s"), 
				*GetNameSafe(Config.DefenderRewardEffect));
		}
	}

	// === 6. Attacker Poise damage GE (groggy buildup) ===
	if (Config.AttackerPoiseDamageEffect && AttackerASC && OwnerASC)
	{
		FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(Config.AttackerPoiseDamageEffect, 1.0f, Context);
		if (Spec.IsValid())
		{
			// SetByCaller for Poise damage amount
			Spec.Data->SetSetByCallerMagnitude(
				HarmoniaGameplayTags::SetByCaller_PoiseDamage, 
				Config.PoiseDamageAmount);
			AttackerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

			UE_LOG(LogHarmoniaCombat, Log, TEXT("[Combat] Attacker Poise damage applied: %.1f"), 
				Config.PoiseDamageAmount);
		}
	}

	// === 7. Start riposte window ===
	OnParrySuccess(Attacker);
}

