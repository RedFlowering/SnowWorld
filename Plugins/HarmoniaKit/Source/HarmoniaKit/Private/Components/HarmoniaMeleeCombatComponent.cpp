// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaGameplayTags.h"
#include "HarmoniaLogCategories.h"
#include "HarmoniaDataTableBFL.h"

#include "Components/HarmoniaMnhTracerManagerComponent.h"
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
#include "Net/UnrealNetwork.h"

namespace HarmoniaCombatASC
{
	static UAbilitySystemComponent* ResolveASCFromActor(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		// 1) Actor가 IAbilitySystemInterface를 구현한 경우(가장 정석)
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Actor))
		{
			if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
			{
				return ASC;
			}
		}

		// 2) Actor에 ASC 컴포넌트가 직접 붙어있는 경우
		if (UAbilitySystemComponent* DirectASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			return DirectASC;
		}

		// 3) Lyra 스타일: Pawn(Avatar) -> PlayerState(Owner) 에 ASC가 붙어있는 경우
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

		return nullptr;
	}
}

UHarmoniaMeleeCombatComponent::UHarmoniaMeleeCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Tick 없음(필요 시 다시 켜기)

	// Enable network replication for Client RPCs (e.g., ClientReceiveHitReaction)
	SetIsReplicatedByDefault(true);

	AttackingTag = HarmoniaGameplayTags::Character_State_Attacking;
	BlockingTag = HarmoniaGameplayTags::Character_State_Blocking;
	ParryingTag = HarmoniaGameplayTags::Character_State_Parrying;
	DodgingTag = HarmoniaGameplayTags::Character_State_Dodging;
	InvulnerableTag = HarmoniaGameplayTags::Character_State_Invulnerable;
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

		// Cache MnhTracerManager for attack window integration
		CachedMnhTracerManager = Owner->FindComponentByClass<UHarmoniaMnhTracerManagerComponent>();
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
	// Owner가 Pawn이고 ASC가 PlayerState에 있는 구조까지 커버
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

	// Control MnhTracer with attack window
	if (CachedMnhTracerManager)
	{
		if (bInWindow)
		{
			CachedMnhTracerManager->StartWeaponTracers();
		}
		else
		{
			CachedMnhTracerManager->StopWeaponTracers();
		}
	}
}

// ============================================================================
// Combo System (기존 로직 유지)
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

void UHarmoniaMeleeCombatComponent::OnAttackBlocked(AActor* Attacker, float Damage)
{
	OnBlockedAttack.Broadcast(Attacker, Damage);

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
// Backstab / Riposte (기존 유지)
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

void UHarmoniaMeleeCombatComponent::ApplyDamageToTarget(AActor* TargetActor, const FVector& HitLocation)
{
	AActor* Owner = GetOwner();
	if (!Owner || !TargetActor || TargetActor == Owner)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[ApplyDamage] Early return: Invalid Owner or Target"));
		return;
	}

	if (!Owner->HasAuthority())
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[ApplyDamage] Early return: No authority"));
		return;
	}

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[ApplyDamage] Early return: No OwnerASC"));
		return;
	}

	UAbilitySystemComponent* TargetASC = HarmoniaCombatASC::ResolveASCFromActor(TargetActor);
	if (!TargetASC)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[ApplyDamage] Early return: No TargetASC"));
		return;
	}

	if (TargetASC->HasMatchingGameplayTag(InvulnerableTag))
	{
		UE_LOG(LogHarmoniaCombat, Log, TEXT("[ApplyDamage] Target is invulnerable, skipping"));
		return;
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

	TSubclassOf<UGameplayEffect> EffectToApply = AttackData.DamageConfig.DamageEffectClass ? AttackData.DamageConfig.DamageEffectClass : DamageEffectClass;
	if (!EffectToApply)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[ApplyDamage] Early return: No DamageEffectClass"));
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(EffectToApply, 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[ApplyDamage] Early return: Invalid SpecHandle"));
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
	if (bIsAttacking || DefenseState != EHarmoniaDefenseState::None)
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
	if (bIsAttacking || DefenseState == EHarmoniaDefenseState::Stunned)
	{
		return false;
	}

	if (const UHarmoniaAttributeSet* AttributeSet = GetAttributeSet())
	{
		if (AttributeSet->GetStamina() < DefaultDodgeConfig.StaminaCost)
		{
			return false;
		}
	}

	return true;
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
