// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaBaseCombatComponent.h"
#include "HarmoniaLogCategories.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystem/HarmoniaAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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

UHarmoniaSenseAttackComponent* UHarmoniaBaseCombatComponent::GetAttackComponent() const
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
			
			if (!CachedAttributeSet)
			{
				// Debug: List all AttributeSets on ASC
				UE_LOG(LogTemp, Warning, TEXT("[BaseCombatComponent] HarmoniaAttributeSet not found! Listing all AttributeSets on ASC:"));
				const TArray<UAttributeSet*>& AttributeSets = ASC->GetSpawnedAttributes();
				
				for (int32 i = 0; i < AttributeSets.Num(); ++i)
				{
					if (AttributeSets[i])
					{
						UE_LOG(LogTemp, Warning, TEXT("  [%d] %s"), i, *AttributeSets[i]->GetClass()->GetName());
					}
				}
				
				if (AttributeSets.Num() == 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("  No AttributeSets found on ASC!"));
				}
			}
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
	return UHarmoniaAbilitySystemLibrary::ConsumeStamina(ASC, StaminaCost);
}

float UHarmoniaBaseCombatComponent::GetCurrentStamina() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseCombatComponent] GetCurrentStamina: ASC is NULL! Owner=%s"), 
			GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
		return 0.0f;
	}

	UHarmoniaAttributeSet* Attributes = GetAttributeSet();
	if (!Attributes)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseCombatComponent] GetCurrentStamina: AttributeSet is NULL! ASC=%s"), 
			*ASC->GetName());
		return 0.0f;
	}

	const float Stamina = Attributes->GetStamina();
	UE_LOG(LogTemp, Log, TEXT("[BaseCombatComponent] GetCurrentStamina: %.1f (Max: %.1f)"), 
		Stamina, Attributes->GetMaxStamina());
	return Stamina;
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
