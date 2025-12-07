// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaGameplayTags.h"
#include "HarmoniaLogCategories.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystem/HarmoniaAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UHarmoniaMeleeCombatComponent::UHarmoniaMeleeCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Enable only when needed

	// Initialize default gameplay tags
	AttackingTag = HarmoniaGameplayTags::Character_State_Attacking;
	BlockingTag = HarmoniaGameplayTags::Character_State_Blocking;
	ParryingTag = HarmoniaGameplayTags::Character_State_Parrying;
	DodgingTag = HarmoniaGameplayTags::Character_State_Dodging;
	InvulnerableTag = HarmoniaGameplayTags::Character_State_Invulnerable;
}

void UHarmoniaMeleeCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaMeleeCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ============================================================================
// Weapon Management
// ============================================================================

void UHarmoniaMeleeCombatComponent::SetCurrentWeaponType(EHarmoniaMeleeWeaponType NewWeaponType)
{
	if (CurrentWeaponType != NewWeaponType)
	{
		CurrentWeaponType = NewWeaponType;

		// Reset combo when weapon changes
		ResetCombo();

		// Refresh cached combos for new weapon
		RefreshCachedCombos();
	}
}

bool UHarmoniaMeleeCombatComponent::GetCurrentWeaponData(FHarmoniaMeleeWeaponData& OutWeaponData) const
{
	return GetWeaponDataForType(CurrentWeaponType, OutWeaponData);
}

bool UHarmoniaMeleeCombatComponent::GetWeaponDataForType(EHarmoniaMeleeWeaponType WeaponType, FHarmoniaMeleeWeaponData& OutWeaponData) const
{
	if (!WeaponDataTable)
	{
		return false;
	}

	FName RowName = EnumToRowName(WeaponType);
	FHarmoniaMeleeWeaponData* WeaponData = WeaponDataTable->FindRow<FHarmoniaMeleeWeaponData>(RowName, TEXT("GetWeaponDataForType"));
	if (WeaponData)
	{
		OutWeaponData = *WeaponData;
		return true;
	}

	return false;
}

// ============================================================================
// Combat State
// ============================================================================

void UHarmoniaMeleeCombatComponent::SetDefenseState(EHarmoniaDefenseState NewState)
{
	if (DefenseState == NewState)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Remove old state tag
	switch (DefenseState)
	{
	case EHarmoniaDefenseState::Blocking:
		ASC->RemoveLooseGameplayTag(BlockingTag);
		break;
	case EHarmoniaDefenseState::Parrying:
		ASC->RemoveLooseGameplayTag(ParryingTag);
		break;
	case EHarmoniaDefenseState::Dodging:
		ASC->RemoveLooseGameplayTag(DodgingTag);
		break;
	default:
		break;
	}

	DefenseState = NewState;

	// Apply new state tag
	switch (DefenseState)
	{
	case EHarmoniaDefenseState::Blocking:
		ASC->AddLooseGameplayTag(BlockingTag);
		break;
	case EHarmoniaDefenseState::Parrying:
		ASC->AddLooseGameplayTag(ParryingTag);
		break;
	case EHarmoniaDefenseState::Dodging:
		ASC->AddLooseGameplayTag(DodgingTag);
		break;
	default:
		break;
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
	if (ASC)
	{
		if (bInIFrames)
		{
			ASC->AddLooseGameplayTag(InvulnerableTag);

			if (Duration > 0.0f)
			{
				GetWorld()->GetTimerManager().SetTimer(IFrameTimerHandle, this, &UHarmoniaMeleeCombatComponent::ClearInvulnerability, Duration, false);
			}
		}
		else
		{
			ASC->RemoveLooseGameplayTag(InvulnerableTag);
			GetWorld()->GetTimerManager().ClearTimer(IFrameTimerHandle);
		}
	}
}

void UHarmoniaMeleeCombatComponent::ClearInvulnerability()
{
	SetInvulnerable(false);
}

// ============================================================================
// Combo System
// ============================================================================

int32 UHarmoniaMeleeCombatComponent::GetMaxComboCount() const
{
	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.MaxComboChain;
	}
	return 3; // Default
}

void UHarmoniaMeleeCombatComponent::AdvanceCombo()
{
	const int32 MaxCombo = GetMaxComboCount();
	CurrentComboIndex = (CurrentComboIndex + 1) % MaxCombo;

	// Start combo window timer
	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		GetWorld()->GetTimerManager().SetTimer(
			ComboWindowTimerHandle,
			this,
			&UHarmoniaMeleeCombatComponent::OnComboWindowExpired,
			WeaponData.ComboWindowDuration,
			false
		);
	}
}

void UHarmoniaMeleeCombatComponent::ResetCombo()
{
	CurrentComboIndex = 0;
	bNextComboQueued = false;
	GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
}

bool UHarmoniaMeleeCombatComponent::IsInComboWindow() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(ComboWindowTimerHandle);
}

void UHarmoniaMeleeCombatComponent::QueueNextCombo()
{
	if (IsInComboWindow())
	{
		bNextComboQueued = true;
	}
}

void UHarmoniaMeleeCombatComponent::OnComboWindowExpired()
{
	bNextComboQueued = false;
}

bool UHarmoniaMeleeCombatComponent::GetComboSequence(EHarmoniaAttackType AttackType, FHarmoniaComboAttackSequence& OutSequence) const
{
	// Return cached combo data
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

	// Fallback: direct lookup if cache not valid
	if (!ComboSequencesDataTable)
	{
		return false;
	}

	const FString ContextString = TEXT("GetComboSequence");
	TArray<FHarmoniaComboAttackSequence*> AllRows;
	ComboSequencesDataTable->GetAllRows<FHarmoniaComboAttackSequence>(ContextString, AllRows);

	for (const FHarmoniaComboAttackSequence* Row : AllRows)
	{
		if (Row && Row->WeaponType == CurrentWeaponType && Row->AttackType == AttackType)
		{
			// Check owner type tag if specified
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
				// Both don't have owner tag - match
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

	for (const FHarmoniaComboAttackSequence* Row : AllRows)
	{
		if (!Row || Row->WeaponType != CurrentWeaponType)
		{
			continue;
		}

		// Check owner type tag match
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
			// Only cache if not already present (first match wins)
			if (!CachedCombos.Contains(Row->AttackType))
			{
				CachedCombos.Add(Row->AttackType, *Row);
			}
		}
	}

	bComboCacheValid = true;

	UE_LOG(LogHarmoniaCombat, Log, TEXT("RefreshCachedCombos: Weapon=%d, OwnerTag=%s, CachedTypes=%d"),
		(int32)CurrentWeaponType,
		*OwnerTypeTag.ToString(),
		CachedCombos.Num());
}

bool UHarmoniaMeleeCombatComponent::GetCurrentComboAttackData(FHarmoniaAttackData& OutAttackData) const
{
	// Get current combo sequence based on attack type
	const EHarmoniaAttackType CurrentAttackType = bIsHeavyAttack ? EHarmoniaAttackType::Heavy : EHarmoniaAttackType::Light;
	FHarmoniaComboAttackSequence ComboSequence;
	if (!GetComboSequence(CurrentAttackType, ComboSequence))
	{
		return false;
	}

	// Check if current combo index is valid
	if (!ComboSequence.ComboSteps.IsValidIndex(CurrentComboIndex))
	{
		return false;
	}

	const FHarmoniaComboAttackStep& ComboStep = ComboSequence.ComboSteps[CurrentComboIndex];

	// Use AttackDataOverride if enabled, otherwise use weapon default
	if (ComboStep.bUseAttackDataOverride)
	{
		OutAttackData = ComboStep.AttackDataOverride;

		// Apply combo step's damage multiplier on top
		OutAttackData.DamageConfig.DamageMultiplier *= ComboStep.DamageMultiplier;
	}
	else
	{
		// Get weapon default attack data
		FHarmoniaMeleeWeaponData WeaponData;
		if (GetCurrentWeaponData(WeaponData))
		{
			// Use weapon's default trace config
			OutAttackData.TraceConfig = WeaponData.DefaultTraceConfig;

			// Apply combo step's damage multiplier
			OutAttackData.DamageConfig.DamageMultiplier = ComboStep.DamageMultiplier;
		}
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

	const float StaminaCost = GetLightAttackStaminaCost();
	if (!HasEnoughStamina(StaminaCost))
	{
		return false;
	}

	// If in combo window, queue next attack
	if (IsInComboWindow())
	{
		QueueNextCombo();
		return true;
	}

	// Start new attack
	StartAttack(false);
	return true;
}

bool UHarmoniaMeleeCombatComponent::RequestHeavyAttack()
{
	if (!CanAttack())
	{
		return false;
	}

	const float StaminaCost = GetHeavyAttackStaminaCost();
	if (!HasEnoughStamina(StaminaCost))
	{
		return false;
	}

	// Heavy attacks don't combo in most soul-likes
	StartAttack(true);
	return true;
}

void UHarmoniaMeleeCombatComponent::StartAttack(bool bHeavyAttack)
{
	bIsAttacking = true;
	bIsHeavyAttack = bHeavyAttack;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
		ASC->AddLooseGameplayTag(AttackingTag);
	}

	// Consume stamina
	const float StaminaCost = bHeavyAttack ? GetHeavyAttackStaminaCost() : GetLightAttackStaminaCost();
	ConsumeStamina(StaminaCost);
}

void UHarmoniaMeleeCombatComponent::EndAttack()
{
	bIsAttacking = false;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(AttackingTag);
	}

	// Check if should advance combo
	if (bNextComboQueued && !bIsHeavyAttack)
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
	// Can't attack while blocking, parrying, or stunned
	if (DefenseState == EHarmoniaDefenseState::Blocking ||
		DefenseState == EHarmoniaDefenseState::Stunned)
	{
		return false;
	}

	// Check ability system for blocking tags
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Blocked.Attack")));

		if (ASC->HasAnyMatchingGameplayTags(BlockedTags))
		{
			return false;
		}
	}

	return true;
}

float UHarmoniaMeleeCombatComponent::GetLightAttackStaminaCost() const
{
	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.LightAttackStaminaCost;
	}
	return 10.0f; // Default
}

float UHarmoniaMeleeCombatComponent::GetHeavyAttackStaminaCost() const
{
	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.HeavyAttackStaminaCost;
	}
	return 25.0f; // Default
}

// ============================================================================
// Defense
// ============================================================================

bool UHarmoniaMeleeCombatComponent::CanBlock() const
{
	if (DefenseState != EHarmoniaDefenseState::None)
	{
		return false;
	}

	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.bCanBlock;
	}

	return false;
}

bool UHarmoniaMeleeCombatComponent::CanParry() const
{
	if (DefenseState != EHarmoniaDefenseState::None)
	{
		return false;
	}

	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.bCanParry;
	}

	return false;
}

bool UHarmoniaMeleeCombatComponent::CanDodge() const
{
	// Can dodge from most states except stunned
	if (DefenseState == EHarmoniaDefenseState::Stunned)
	{
		return false;
	}

	return true;
}

float UHarmoniaMeleeCombatComponent::GetBlockDamageReduction() const
{
	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.BlockDamageReduction;
	}

	return DefaultDefenseConfig.BlockDamageReduction;
}

float UHarmoniaMeleeCombatComponent::GetBlockStaminaCost() const
{
	FHarmoniaMeleeWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.BlockStaminaCost;
	}

	return DefaultDefenseConfig.BlockStaminaCost;
}

void UHarmoniaMeleeCombatComponent::OnAttackBlocked(AActor* Attacker, float Damage)
{
	// Consume stamina
	const float StaminaCost = GetBlockStaminaCost();
	if (!ConsumeStamina(StaminaCost))
	{
		// Not enough stamina, guard broken
		SetDefenseState(EHarmoniaDefenseState::Stunned);

		// Apply guard break effects
		// Note: In full implementation, apply a stun/guard break gameplay effect to the owner
		// This effect should:
		// - Prevent actions for a duration (via gameplay tags)
		// - Play guard break animation
		// - Possibly apply a defense debuff
		// Example: ASC->ApplyGameplayEffectToSelf(GuardBreakEffectClass, 1.0f, ASC->MakeEffectContext());
		UE_LOG(LogHarmoniaCombat, Log, TEXT("Guard broken - stunned state applied"));
	}
}

void UHarmoniaMeleeCombatComponent::OnParrySuccess(AActor* Attacker)
{
	// Successful parry opens up attacker for riposte
	StartRiposteWindow(Attacker, DefaultRiposteConfig.RiposteWindowDuration);

	// Apply stun/parried effect to attacker
	// Note: In full implementation, get the attacker's ASC and apply a parry stun effect
	// This effect should:
	// - Stun the attacker for a short duration
	// - Make them vulnerable to riposte attacks
	// - Play appropriate animations/VFX
	//
	// Example implementation:
	// if (UAbilitySystemComponent* AttackerASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Attacker))
	// {
	//     FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	//     Context.AddInstigator(GetOwner(), GetOwner());
	//     AttackerASC->ApplyGameplayEffectToSelf(ParryStunEffectClass, 1.0f, Context);
	// }
	UE_LOG(LogHarmoniaCombat, Log, TEXT("Parry successful - attacker %s is vulnerable to riposte"), *Attacker->GetName());
}

// ============================================================================
// Stamina Management
// ============================================================================



// ============================================================================
// Riposte System
// ============================================================================

bool UHarmoniaMeleeCombatComponent::CanRiposte() const
{
	return DefenseState == EHarmoniaDefenseState::RiposteWindow;
}

float UHarmoniaMeleeCombatComponent::GetRiposteWindowDuration() const
{
	return DefaultRiposteConfig.RiposteWindowDuration;
}

float UHarmoniaMeleeCombatComponent::GetRiposteDamageMultiplier() const
{
	return DefaultRiposteConfig.RiposteDamageMultiplier;
}

void UHarmoniaMeleeCombatComponent::StartRiposteWindow(AActor* ParriedTargetActor, float Duration)
{
	if (!ParriedTargetActor)
	{
		return;
	}

	// Store parried target
	ParriedTarget = ParriedTargetActor;

	// Set defense state to riposte window
	SetDefenseState(EHarmoniaDefenseState::RiposteWindow);

	// Start riposte window timer
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
	GetWorld()->GetTimerManager().ClearTimer(RiposteWindowTimerHandle);
}

void UHarmoniaMeleeCombatComponent::OnRiposteWindowExpired()
{
	EndRiposteWindow();
}

AActor* UHarmoniaMeleeCombatComponent::GetParriedTarget() const
{
	return ParriedTarget.Get();
}

void UHarmoniaMeleeCombatComponent::ClearParriedTarget()
{
	ParriedTarget.Reset();
}

// ============================================================================
// Backstab System
// ============================================================================

bool UHarmoniaMeleeCombatComponent::IsBackstabAttack(AActor* Target, FVector AttackOrigin) const
{
	if (!Target || !DefaultBackstabConfig.bEnableBackstab)
	{
		return false;
	}

	// Get target's forward vector
	FVector TargetForward = Target->GetActorForwardVector();
	TargetForward.Z = 0.0f; // Ignore vertical component
	TargetForward.Normalize();

	// Get direction from target to attacker
	FVector ToAttacker = AttackOrigin - Target->GetActorLocation();
	ToAttacker.Z = 0.0f; // Ignore vertical component

	const float Distance = ToAttacker.Size();
	if (Distance > DefaultBackstabConfig.BackstabMaxDistance)
	{
		return false; // Too far away
	}

	ToAttacker.Normalize();

	// Calculate angle between target's forward and direction to attacker
	// If attacker is behind target, the angle should be close to 180 degrees
	const float DotProduct = FVector::DotProduct(TargetForward, ToAttacker);
	const float AngleRadians = FMath::Acos(DotProduct);
	const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// Check if within backstab angle tolerance
	// 180 degrees means directly behind, lower tolerance = stricter backstab
	const float RequiredAngle = 180.0f - DefaultBackstabConfig.BackstabAngleTolerance;

	return AngleDegrees >= RequiredAngle;
}

float UHarmoniaMeleeCombatComponent::GetBackstabDamageMultiplier() const
{
	return DefaultBackstabConfig.BackstabDamageMultiplier;
}
