// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaGameplayTags.h"
#include "HarmoniaLogCategories.h"
#include "HarmoniaDataTableBFL.h"
#include "Components/HarmoniaSenseComponent.h"
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

	// Auto-load DataTables from HarmoniaDataTableBFL if not already set
	if (!ComboSequencesDataTable)
	{
		ComboSequencesDataTable = UHarmoniaDataTableBFL::GetComboAttackDataTable();
	}

	// Always refresh cache on BeginPlay
	// Always refresh cache on BeginPlay
	RefreshCachedCombos();

	// Listen for equipment changes
	if (AActor* Owner = GetOwner())
	{
		if (UHarmoniaEquipmentComponent* EquipComp = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>())
		{
			EquipComp->OnEquipmentChanged.AddDynamic(this, &UHarmoniaMeleeCombatComponent::OnEquipmentChanged);
		}
	}
}


void UHarmoniaMeleeCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHarmoniaMeleeCombatComponent::OnEquipmentChanged(EEquipmentSlot Slot, const FHarmoniaID& OldId, const FHarmoniaID& NewId)
{
	// Update combos if main hand weapon changed
	if (Slot == EEquipmentSlot::MainHand)
	{
		RefreshCachedCombos();
	}
}


// ============================================================================
// Weapon Management
// ============================================================================

FGameplayTag UHarmoniaMeleeCombatComponent::GetCurrentWeaponTypeTag() const
{
	// Query EquipmentComponent as the single source of truth
	if (UHarmoniaEquipmentComponent* EquipComp = GetOwner()->FindComponentByClass<UHarmoniaEquipmentComponent>())
	{
		return EquipComp->GetMainHandWeaponTypeTag();
	}

	// Default to Fist if no equipment component
	return FGameplayTag::RequestGameplayTag(FName("Weapon.Type.Fist"), false);
}

// ============================================================================
// Combat State
// ============================================================================

void UHarmoniaMeleeCombatComponent::SetDefenseState(EHarmoniaDefenseState NewState)
{
	// Server authority check - prevent client manipulation
	AActor* Owner = GetOwner();
	if (Owner && !Owner->HasAuthority())
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
	// Get from cached combo sequence
	FHarmoniaComboAttackSequence ComboSequence;
	if (GetComboSequence(CurrentAttackType, ComboSequence))
	{
		return ComboSequence.ComboSteps.Num();
	}
	return 3; // Default
}

void UHarmoniaMeleeCombatComponent::AdvanceCombo()
{
	// Simply increment the index - validity is checked in the ability's OnMontageCompleted
	CurrentComboIndex++;

	// Combo window is managed by AnimNotifyState, not automatically started here
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

void UHarmoniaMeleeCombatComponent::OpenComboWindow(float Duration)
{
	// Start combo window timer - allows input queueing during this window
	if (Duration > 0.0f)
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
	// Strictly allow queueing only during combo window (controlled by AnimNotifyState)
	if (IsInComboWindow())
	{
		bNextComboQueued = true;
	}
}

void UHarmoniaMeleeCombatComponent::OnComboWindowExpired()
{
	// Don't clear bNextComboQueued here - let EndAttack handle it.
	// We want to preserve the queued state if the user clicked successfully while the window was open,
	// even if the timer expires before EndAttack is called.
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

	const FGameplayTag CurrentWeaponTag = GetCurrentWeaponTypeTag();

	for (const FHarmoniaComboAttackSequence* Row : AllRows)
	{
		if (Row && Row->WeaponTypeTag.MatchesTagExact(CurrentWeaponTag) && Row->AttackType == AttackType)
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

	const FGameplayTag CurrentWeaponTag = GetCurrentWeaponTypeTag();

	for (const FHarmoniaComboAttackSequence* Row : AllRows)
	{
		if (!Row || !Row->WeaponTypeTag.MatchesTagExact(CurrentWeaponTag))
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
}

bool UHarmoniaMeleeCombatComponent::GetCurrentComboAttackData(FHarmoniaAttackData& OutAttackData) const
{
	// Get current combo sequence based on attack type
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

	// Use AttackDataOverride if enabled
	if (ComboStep.bUseAttackDataOverride)
	{
		OutAttackData = ComboStep.AttackDataOverride;

		// Apply combo step's damage multiplier on top
		OutAttackData.DamageConfig.DamageMultiplier *= ComboStep.DamageMultiplier;
	}
	else
	{
		// Use default attack data with combo step's damage multiplier
		// Trace config and other settings should be configured in ComboStep's AttackDataOverride
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

	// If in combo window, queue next attack
	if (IsInComboWindow())
	{
		QueueNextCombo();
		return true;
	}

	// Start new attack (stamina cost handled by GA's CostGE)
	StartAttack(EHarmoniaAttackType::Light);
	return true;
}

bool UHarmoniaMeleeCombatComponent::RequestHeavyAttack()
{
	if (!CanAttack())
	{
		return false;
	}

	// If in combo window while attacking, queue the next attack
	if (IsInComboWindow() && IsAttacking())
	{
		QueueNextCombo();
		return true;
	}

	// Start new heavy attack combo (stamina cost handled by GA's CostGE)
	StartAttack(EHarmoniaAttackType::Heavy);
	return true;
}

void UHarmoniaMeleeCombatComponent::StartAttack(EHarmoniaAttackType InAttackType)
{
	bIsAttacking = true;

	// Reset combo if attack type changed (e.g., switching from Light to Heavy)
	if (CurrentAttackType != InAttackType)
	{
		CurrentComboIndex = 0;
		bNextComboQueued = false;
	}

	CurrentAttackType = InAttackType;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC)
	{
		ASC->AddLooseGameplayTag(AttackingTag);
	}

	// Note: Stamina consumption is handled by the GA's CommitAbilityCost/CostGE
}

void UHarmoniaMeleeCombatComponent::EndAttack()
{
	bIsAttacking = false;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && ASC->HasMatchingGameplayTag(AttackingTag))
	{
		ASC->RemoveLooseGameplayTag(AttackingTag);
	}

	// Check if should advance combo (for both Light and Heavy attacks)
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
	// Can't attack while blocking, parrying, or stunned
	if (DefenseState == EHarmoniaDefenseState::Blocking ||
		DefenseState == EHarmoniaDefenseState::Stunned)
	{
		return false;
	}

	// Check ability system for blocking tags
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && AttackBlockedTag.IsValid())
	{
		if (ASC->HasMatchingGameplayTag(AttackBlockedTag))
		{
			return false;
		}
	}

	return true;
}

// ============================================================================
// Defense
// ============================================================================

bool UHarmoniaMeleeCombatComponent::CanBlock() const
{
	// Can block if not already in a defense state
	return DefenseState == EHarmoniaDefenseState::None;
}

bool UHarmoniaMeleeCombatComponent::CanParry() const
{
	// Can parry if not already in a defense state
	return DefenseState == EHarmoniaDefenseState::None;
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

bool UHarmoniaMeleeCombatComponent::IsDefenseAngleValid(const FVector& AttackerLocation) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Get defender's forward direction (horizontal only)
	FVector DefenderForward = Owner->GetActorForwardVector();
	DefenderForward.Z = 0.0f;
	DefenderForward.Normalize();

	// Get direction from defender to attacker (horizontal only)
	FVector ToAttacker = AttackerLocation - Owner->GetActorLocation();
	ToAttacker.Z = 0.0f;
	
	if (ToAttacker.IsNearlyZero())
	{
		return true; // Attacker is on top of defender, consider it blockable
	}
	
	ToAttacker.Normalize();

	// Calculate angle between defender's forward and direction to attacker
	// DotProduct = 1 when attacker is directly in front, -1 when behind
	const float DotProduct = FVector::DotProduct(DefenderForward, ToAttacker);
	const float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
	const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// Check if within half the defense angle (since DefenseAngle is full arc)
	// e.g., DefenseAngle = 120 means 60 degrees each side of forward
	const float HalfDefenseAngle = DefenseAngle / 2.0f;
	
	return AngleDegrees <= HalfDefenseAngle;
}

void UHarmoniaMeleeCombatComponent::OnAttackBlocked(AActor* Attacker, float Damage)
{
	// Broadcast delegate for subscribed abilities (e.g., Block ability for stamina cost)
	OnBlockedAttack.Broadcast(Attacker, Damage);

	// Check if stamina is sufficient - if not, guard break
	if (UHarmoniaAttributeSet* AttributeSet = GetAttributeSet())
	{
		if (AttributeSet->GetStamina() <= 0.0f)
		{
			// Guard broken - stunned state
			SetDefenseState(EHarmoniaDefenseState::Stunned);
			UE_LOG(LogHarmoniaCombat, Log, TEXT("Guard broken - stunned state applied"));
		}
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
