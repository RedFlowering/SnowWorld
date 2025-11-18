// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
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
	AttackingTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking"));
	BlockingTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Blocking"));
	ParryingTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Parrying"));
	DodgingTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Dodging"));
	InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Invulnerable"));
}

void UHarmoniaMeleeCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache component references
	GetAttackComponent();
	GetAbilitySystemComponent();
	GetAttributeSet();
	GetEquipmentComponent();
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

	// Convert enum to string for row name
	FString WeaponTypeName = UEnum::GetValueAsString(WeaponType);
	WeaponTypeName = WeaponTypeName.Right(WeaponTypeName.Len() - WeaponTypeName.Find("::") - 2);

	FHarmoniaMeleeWeaponData* WeaponData = WeaponDataTable->FindRow<FHarmoniaMeleeWeaponData>(FName(*WeaponTypeName), TEXT("GetWeaponDataForType"));
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

bool UHarmoniaMeleeCombatComponent::GetComboSequence(bool bHeavyCombo, FHarmoniaComboAttackSequence& OutSequence) const
{
	if (!ComboSequencesDataTable)
	{
		return false;
	}

	// Try to find combo sequence for current weapon type
	FString WeaponTypeName = UEnum::GetValueAsString(CurrentWeaponType);
	WeaponTypeName = WeaponTypeName.Right(WeaponTypeName.Len() - WeaponTypeName.Find("::") - 2);

	FString ComboName = WeaponTypeName + (bHeavyCombo ? "_Heavy" : "_Light");

	FHarmoniaComboAttackSequence* ComboSequence = ComboSequencesDataTable->FindRow<FHarmoniaComboAttackSequence>(FName(*ComboName), TEXT("GetComboSequence"));
	if (ComboSequence)
	{
		OutSequence = *ComboSequence;
		return true;
	}

	return false;
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

		// TODO: Apply guard break effects
	}
}

void UHarmoniaMeleeCombatComponent::OnParrySuccess(AActor* Attacker)
{
	// Successful parry opens up attacker for riposte
	// TODO: Apply parry effects to attacker
}

// ============================================================================
// Stamina Management
// ============================================================================

bool UHarmoniaMeleeCombatComponent::HasEnoughStamina(float StaminaCost) const
{
	return GetCurrentStamina() >= StaminaCost;
}

bool UHarmoniaMeleeCombatComponent::ConsumeStamina(float StaminaCost)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();

	if (!ASC || !Attributes)
	{
		return false;
	}

	const float CurrentStamina = GetCurrentStamina();
	if (CurrentStamina < StaminaCost)
	{
		return false;
	}

	// Apply stamina cost via gameplay effect
	// For now, directly modify the attribute
	const float NewStamina = FMath::Max(0.0f, CurrentStamina - StaminaCost);

	// TODO: Use gameplay effect instead of direct modification
	// This is a temporary implementation

	return true;
}

float UHarmoniaMeleeCombatComponent::GetCurrentStamina() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetStamina();
	}
	return 0.0f;
}

float UHarmoniaMeleeCombatComponent::GetMaxStamina() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetMaxStamina();
	}
	return 100.0f;
}

// ============================================================================
// Component References
// ============================================================================

UHarmoniaSenseAttackComponent* UHarmoniaMeleeCombatComponent::GetAttackComponent() const
{
	if (!CachedAttackComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			CachedAttackComponent = Owner->FindComponentByClass<UHarmoniaSenseAttackComponent>();
		}
	}
	return CachedAttackComponent;
}

UAbilitySystemComponent* UHarmoniaMeleeCombatComponent::GetAbilitySystemComponent() const
{
	if (!CachedAbilitySystemComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			CachedAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
		}
	}
	return CachedAbilitySystemComponent;
}

UHarmoniaAttributeSet* UHarmoniaMeleeCombatComponent::GetAttributeSet() const
{
	if (!CachedAttributeSet)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		if (ASC)
		{
			CachedAttributeSet = const_cast<UHarmoniaAttributeSet*>(ASC->GetSet<UHarmoniaAttributeSet>());
		}
	}
	return CachedAttributeSet;
}

UHarmoniaEquipmentComponent* UHarmoniaMeleeCombatComponent::GetEquipmentComponent() const
{
	if (!CachedEquipmentComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			CachedEquipmentComponent = Owner->FindComponentByClass<UHarmoniaEquipmentComponent>();
		}
	}
	return CachedEquipmentComponent;
}
