// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaBaseCombatComponent.h"
#include "HarmoniaLogCategories.h"
#include "Components/HarmoniaSenseComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystem/HarmoniaAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"

UHarmoniaBaseCombatComponent::UHarmoniaBaseCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHarmoniaBaseCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// Pre-cache component references
	GetAttackComponent();
	GetAbilitySystemComponent();
	GetAttributeSet();
	GetEquipmentComponent();
}

// ============================================================================
// Component References
// ============================================================================

UHarmoniaSenseComponent* UHarmoniaBaseCombatComponent::GetAttackComponent() const
{
	if (!CachedAttackComponent)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			CachedAttackComponent = Owner->FindComponentByClass<UHarmoniaSenseComponent>();
		}
	}
	return CachedAttackComponent;
}

UAbilitySystemComponent* UHarmoniaBaseCombatComponent::GetAbilitySystemComponent() const
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

UHarmoniaAttributeSet* UHarmoniaBaseCombatComponent::GetAttributeSet() const
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

UHarmoniaEquipmentComponent* UHarmoniaBaseCombatComponent::GetEquipmentComponent() const
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

// ============================================================================
// Stamina Management
// ============================================================================

bool UHarmoniaBaseCombatComponent::HasEnoughStamina(float StaminaCost) const
{
	return GetCurrentStamina() >= StaminaCost;
}

bool UHarmoniaBaseCombatComponent::ConsumeStamina(float StaminaCost)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	// Use the HarmoniaAbilitySystemLibrary to consume stamina
	// Note: HarmoniaAttributeSet::PostAttributeChange automatically applies
	// StaminaRecoveryBlockEffectClass when stamina decreases
	return UHarmoniaAbilitySystemLibrary::ConsumeStamina(ASC, StaminaCost);
}

float UHarmoniaBaseCombatComponent::GetCurrentStamina() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetStamina();
	}
	return 0.0f;
}

float UHarmoniaBaseCombatComponent::GetMaxStamina() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetMaxStamina();
	}
	return 100.0f;
}

float UHarmoniaBaseCombatComponent::GetStaminaPercentage() const
{
	float MaxStamina = GetMaxStamina();
	if (MaxStamina > 0.0f)
	{
		return GetCurrentStamina() / MaxStamina;
	}
	return 0.0f;
}

float UHarmoniaBaseCombatComponent::GetStaminaRecoveryDelay() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetStaminaRecoveryDelay();
	}
	return 1.5f; // Default delay
}

// ============================================================================
// Mana Management
// ============================================================================

bool UHarmoniaBaseCombatComponent::HasEnoughMana(float ManaCost) const
{
	return GetCurrentMana() >= ManaCost;
}

bool UHarmoniaBaseCombatComponent::ConsumeMana(float ManaCost)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	// Check if we have enough mana
	if (!HasEnoughMana(ManaCost))
	{
		return false;
	}

	// Apply mana cost by directly modifying the attribute
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		float CurrentMana = Attributes->GetMana();
		Attributes->SetMana(FMath::Max(0.0f, CurrentMana - ManaCost));
		return true;
	}
	
	return false;
}

float UHarmoniaBaseCombatComponent::GetCurrentMana() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetMana();
	}
	return 0.0f;
}

float UHarmoniaBaseCombatComponent::GetMaxMana() const
{
	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (Attributes)
	{
		return Attributes->GetMaxMana();
	}
	return 100.0f;
}
