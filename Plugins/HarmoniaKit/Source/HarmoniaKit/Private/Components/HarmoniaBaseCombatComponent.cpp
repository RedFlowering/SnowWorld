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
#include "HarmoniaDataTableBFL.h"

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

// ============================================================================
// Buff Management
// ============================================================================

bool UHarmoniaBaseCombatComponent::ApplyBuffByTag(FGameplayTag EffectTag)
{
	FHarmoniaBuffData Data;
	if (!GetBuffData(EffectTag, Data))
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("ApplyBuffByTag: No data found for tag %s"), *EffectTag.ToString());
		return false;
	}

	if (!Data.ApplyEffectClass)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("ApplyBuffByTag: No ApplyEffectClass set for tag %s"), *EffectTag.ToString());
		return false;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(GetOwner());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Data.ApplyEffectClass, 1.0f, ContextHandle);
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		return true;
	}

	return false;
}

bool UHarmoniaBaseCombatComponent::RemoveBuffByTag(FGameplayTag EffectTag)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	// First, try to use RemoveEffectClass if specified in DataTable
	FHarmoniaBuffData Data;
	if (GetBuffData(EffectTag, Data) && Data.RemoveEffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(GetOwner());

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Data.RemoveEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			return true;
		}
	}

	// Fallback: Remove effects by matching granted tags
	FGameplayTagContainer TagsToRemove;
	TagsToRemove.AddTag(EffectTag);

	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagsToRemove);
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	if (ActiveEffects.Num() == 0)
	{
		return false;
	}

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}

	return true;
}

bool UHarmoniaBaseCombatComponent::GetBuffData(FGameplayTag EffectTag, FHarmoniaBuffData& OutData) const
{
	// Try to get from cache first
	if (!CachedBuffDataTable)
	{
		// Load from HarmoniaLoadManager via generated function
		CachedBuffDataTable = UHarmoniaDataTableBFL::GetBuffDataTable();

		if (!CachedBuffDataTable)
		{
			UE_LOG(LogHarmoniaKit, Warning, TEXT("GetBuffData: Failed to load BuffData table"));
			return false;
		}
	}

	// Iterate through all rows to find matching tag
	TArray<FHarmoniaBuffData*> AllRows;
	CachedBuffDataTable->GetAllRows<FHarmoniaBuffData>(TEXT("GetBuffData"), AllRows);

	for (const FHarmoniaBuffData* Row : AllRows)
	{
		if (Row && Row->EffectTag.MatchesTagExact(EffectTag))
		{
			OutData = *Row;
			return true;
		}
	}

	return false;
}
