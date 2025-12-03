// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/HarmoniaAbilitySystemLibrary.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"

// ============================================================================
// Stamina Functions
// ============================================================================

bool UHarmoniaAbilitySystemLibrary::HasEnoughStamina(UAbilitySystemComponent* AbilitySystemComponent, float StaminaCost)
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return false;
	}

	return AttributeSet->GetStamina() >= StaminaCost;
}

bool UHarmoniaAbilitySystemLibrary::ConsumeStamina(UAbilitySystemComponent* AbilitySystemComponent, float StaminaCost)
{
	if (!AbilitySystemComponent || StaminaCost <= 0.0f)
	{
		return false;
	}

	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return false;
	}

	if (AttributeSet->GetStamina() < StaminaCost)
	{
		return false;
	}

	// Apply stamina cost
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

	// Create a simple instant gameplay effect to reduce stamina
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UHarmoniaAttributeSet::GetStaminaAttribute();
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCallerMagnitude;
	SetByCallerMagnitude.DataTag = FGameplayTag::RequestGameplayTag(FName("Data.StaminaCost"));

	// Since we can't easily create a GameplayEffect at runtime in C++,
	// we'll directly modify the attribute (not recommended for production, but works for testing)
	// In production, you should use a pre-made GameplayEffect class

	// For now, directly modify (this bypasses gameplay effect system)
	UHarmoniaAttributeSet* MutableAttributeSet =
		const_cast<UHarmoniaAttributeSet*>(AttributeSet);
	if (MutableAttributeSet)
	{
		float NewStamina = FMath::Max(AttributeSet->GetStamina() - StaminaCost, 0.0f);
		MutableAttributeSet->SetStamina(NewStamina);
	}

	return true;
}

float UHarmoniaAbilitySystemLibrary::GetStamina(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? AttributeSet->GetStamina() : 0.0f;
}

float UHarmoniaAbilitySystemLibrary::GetMaxStamina(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? AttributeSet->GetMaxStamina() : 0.0f;
}

float UHarmoniaAbilitySystemLibrary::GetStaminaPercent(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet || AttributeSet->GetMaxStamina() <= 0.0f)
	{
		return 0.0f;
	}

	return AttributeSet->GetStamina() / AttributeSet->GetMaxStamina();
}

void UHarmoniaAbilitySystemLibrary::RestoreStamina(UAbilitySystemComponent* AbilitySystemComponent, float Amount)
{
	if (!AbilitySystemComponent || Amount <= 0.0f)
	{
		return;
	}

	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return;
	}

	UHarmoniaAttributeSet* MutableAttributeSet =
		const_cast<UHarmoniaAttributeSet*>(AttributeSet);
	if (MutableAttributeSet)
	{
		float NewStamina = FMath::Min(
			AttributeSet->GetStamina() + Amount,
			AttributeSet->GetMaxStamina()
		);
		MutableAttributeSet->SetStamina(NewStamina);
	}
}

// ============================================================================
// Health Functions
// ============================================================================

float UHarmoniaAbilitySystemLibrary::GetHealth(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

float UHarmoniaAbilitySystemLibrary::GetMaxHealth(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.0f;
}

float UHarmoniaAbilitySystemLibrary::GetHealthPercent(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet || AttributeSet->GetMaxHealth() <= 0.0f)
	{
		return 0.0f;
	}

	return AttributeSet->GetHealth() / AttributeSet->GetMaxHealth();
}

void UHarmoniaAbilitySystemLibrary::RestoreHealth(UAbilitySystemComponent* AbilitySystemComponent, float Amount)
{
	if (!AbilitySystemComponent || Amount <= 0.0f)
	{
		return;
	}

	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return;
	}

	UHarmoniaAttributeSet* MutableAttributeSet =
		const_cast<UHarmoniaAttributeSet*>(AttributeSet);
	if (MutableAttributeSet)
	{
		float NewHealth = FMath::Min(
			AttributeSet->GetHealth() + Amount,
			AttributeSet->GetMaxHealth()
		);
		MutableAttributeSet->SetHealth(NewHealth);
	}
}

// ============================================================================
// Poise Functions
// ============================================================================

float UHarmoniaAbilitySystemLibrary::GetPoise(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? AttributeSet->GetPoise() : 0.0f;
}

float UHarmoniaAbilitySystemLibrary::GetMaxPoise(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? AttributeSet->GetMaxPoise() : 0.0f;
}

bool UHarmoniaAbilitySystemLibrary::IsPoiseBroken(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	return AttributeSet ? (AttributeSet->GetPoise() <= 0.0f) : false;
}

void UHarmoniaAbilitySystemLibrary::ResetPoise(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return;
	}

	UHarmoniaAttributeSet* MutableAttributeSet =
		const_cast<UHarmoniaAttributeSet*>(AttributeSet);
	if (MutableAttributeSet)
	{
		MutableAttributeSet->SetPoise(AttributeSet->GetMaxPoise());
	}
}

// ============================================================================
// Primary Stats Functions
// ============================================================================

float UHarmoniaAbilitySystemLibrary::GetPrimaryStat(UAbilitySystemComponent* AbilitySystemComponent, FName StatName)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return 0.0f;
	}

	if (StatName == TEXT("Vitality"))
	{
		return AttributeSet->GetVitality();
	}
	else if (StatName == TEXT("Endurance"))
	{
		return AttributeSet->GetEndurance();
	}
	else if (StatName == TEXT("Strength"))
	{
		return AttributeSet->GetStrength();
	}
	else if (StatName == TEXT("Dexterity"))
	{
		return AttributeSet->GetDexterity();
	}
	else if (StatName == TEXT("Intelligence"))
	{
		return AttributeSet->GetIntelligence();
	}
	else if (StatName == TEXT("Faith"))
	{
		return AttributeSet->GetFaith();
	}
	else if (StatName == TEXT("Luck"))
	{
		return AttributeSet->GetLuck();
	}

	return 0.0f;
}

void UHarmoniaAbilitySystemLibrary::SetPrimaryStat(UAbilitySystemComponent* AbilitySystemComponent, FName StatName, float NewValue)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet)
	{
		return;
	}

	UHarmoniaAttributeSet* MutableAttributeSet =
		const_cast<UHarmoniaAttributeSet*>(AttributeSet);
	if (!MutableAttributeSet)
	{
		return;
	}

	// Clamp to 1-99 range (typical soul-like)
	NewValue = FMath::Clamp(NewValue, 1.0f, 99.0f);

	if (StatName == TEXT("Vitality"))
	{
		MutableAttributeSet->SetVitality(NewValue);
	}
	else if (StatName == TEXT("Endurance"))
	{
		MutableAttributeSet->SetEndurance(NewValue);
	}
	else if (StatName == TEXT("Strength"))
	{
		MutableAttributeSet->SetStrength(NewValue);
	}
	else if (StatName == TEXT("Dexterity"))
	{
		MutableAttributeSet->SetDexterity(NewValue);
	}
	else if (StatName == TEXT("Intelligence"))
	{
		MutableAttributeSet->SetIntelligence(NewValue);
	}
	else if (StatName == TEXT("Faith"))
	{
		MutableAttributeSet->SetFaith(NewValue);
	}
	else if (StatName == TEXT("Luck"))
	{
		MutableAttributeSet->SetLuck(NewValue);
	}
}

// ============================================================================
// Equipment Load Functions
// ============================================================================

float UHarmoniaAbilitySystemLibrary::GetEquipLoadPercent(UAbilitySystemComponent* AbilitySystemComponent)
{
	const UHarmoniaAttributeSet* AttributeSet = GetAttributeSetChecked(AbilitySystemComponent);
	if (!AttributeSet || AttributeSet->GetMaxEquipLoad() <= 0.0f)
	{
		return 0.0f;
	}

	return AttributeSet->GetEquipLoad() / AttributeSet->GetMaxEquipLoad();
}

FString UHarmoniaAbilitySystemLibrary::GetEquipLoadRollType(UAbilitySystemComponent* AbilitySystemComponent)
{
	float LoadPercent = GetEquipLoadPercent(AbilitySystemComponent);

	if (LoadPercent <= 0.3f)
	{
		return TEXT("Fast");
	}
	else if (LoadPercent <= 0.7f)
	{
		return TEXT("Medium");
	}
	else if (LoadPercent <= 1.0f)
	{
		return TEXT("Fat");
	}
	else
	{
		return TEXT("Overburdened");
	}
}

// ============================================================================
// Utility Functions
// ============================================================================

// Static attribute map - initialized once on first use
namespace HarmoniaAttributeMapping
{
	static TMap<FString, TFunction<FGameplayAttribute()>> AttributeNameMap;
	static bool bInitialized = false;

	void InitializeAttributeMap()
	{
		if (bInitialized)
		{
			return;
		}

		// Core Attributes
		AttributeNameMap.Add(TEXT("Health"), []() { return UHarmoniaAttributeSet::GetHealthAttribute(); });
		AttributeNameMap.Add(TEXT("MaxHealth"), []() { return UHarmoniaAttributeSet::GetMaxHealthAttribute(); });
		AttributeNameMap.Add(TEXT("Stamina"), []() { return UHarmoniaAttributeSet::GetStaminaAttribute(); });
		AttributeNameMap.Add(TEXT("MaxStamina"), []() { return UHarmoniaAttributeSet::GetMaxStaminaAttribute(); });
		AttributeNameMap.Add(TEXT("Mana"), []() { return UHarmoniaAttributeSet::GetManaAttribute(); });
		AttributeNameMap.Add(TEXT("MaxMana"), []() { return UHarmoniaAttributeSet::GetMaxManaAttribute(); });
		AttributeNameMap.Add(TEXT("StaminaRegenRate"), []() { return UHarmoniaAttributeSet::GetStaminaRegenRateAttribute(); });
		AttributeNameMap.Add(TEXT("ManaRegenRate"), []() { return UHarmoniaAttributeSet::GetManaRegenRateAttribute(); });

		// Combat Attributes
		AttributeNameMap.Add(TEXT("AttackPower"), []() { return UHarmoniaAttributeSet::GetAttackPowerAttribute(); });
		AttributeNameMap.Add(TEXT("Defense"), []() { return UHarmoniaAttributeSet::GetDefenseAttribute(); });
		AttributeNameMap.Add(TEXT("CriticalChance"), []() { return UHarmoniaAttributeSet::GetCriticalChanceAttribute(); });
		AttributeNameMap.Add(TEXT("CriticalDamage"), []() { return UHarmoniaAttributeSet::GetCriticalDamageAttribute(); });
		AttributeNameMap.Add(TEXT("AttackSpeed"), []() { return UHarmoniaAttributeSet::GetAttackSpeedAttribute(); });
		AttributeNameMap.Add(TEXT("MovementSpeed"), []() { return UHarmoniaAttributeSet::GetMovementSpeedAttribute(); });
		AttributeNameMap.Add(TEXT("Poise"), []() { return UHarmoniaAttributeSet::GetMaxPoiseAttribute(); });
		AttributeNameMap.Add(TEXT("MaxPoise"), []() { return UHarmoniaAttributeSet::GetMaxPoiseAttribute(); });
		AttributeNameMap.Add(TEXT("EquipLoad"), []() { return UHarmoniaAttributeSet::GetMaxEquipLoadAttribute(); });
		AttributeNameMap.Add(TEXT("MaxEquipLoad"), []() { return UHarmoniaAttributeSet::GetMaxEquipLoadAttribute(); });

		// Primary Stats
		AttributeNameMap.Add(TEXT("Vitality"), []() { return UHarmoniaAttributeSet::GetVitalityAttribute(); });
		AttributeNameMap.Add(TEXT("Endurance"), []() { return UHarmoniaAttributeSet::GetEnduranceAttribute(); });
		AttributeNameMap.Add(TEXT("Strength"), []() { return UHarmoniaAttributeSet::GetStrengthAttribute(); });
		AttributeNameMap.Add(TEXT("Dexterity"), []() { return UHarmoniaAttributeSet::GetDexterityAttribute(); });
		AttributeNameMap.Add(TEXT("Intelligence"), []() { return UHarmoniaAttributeSet::GetIntelligenceAttribute(); });
		AttributeNameMap.Add(TEXT("Faith"), []() { return UHarmoniaAttributeSet::GetFaithAttribute(); });
		AttributeNameMap.Add(TEXT("Luck"), []() { return UHarmoniaAttributeSet::GetLuckAttribute(); });

		bInitialized = true;
	}

	FGameplayAttribute FindAttribute(const FString& AttributeName)
	{
		InitializeAttributeMap();

		if (const TFunction<FGameplayAttribute()>* Getter = AttributeNameMap.Find(AttributeName))
		{
			return (*Getter)();
		}
		return FGameplayAttribute();
	}
}

UHarmoniaAttributeSet* UHarmoniaAbilitySystemLibrary::GetHarmoniaAttributeSet(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	return const_cast<UHarmoniaAttributeSet*>(
		AbilitySystemComponent->GetSet<UHarmoniaAttributeSet>()
	);
}

FGameplayAttribute UHarmoniaAbilitySystemLibrary::GetAttributeByName(const FString& AttributeName)
{
	return HarmoniaAttributeMapping::FindAttribute(AttributeName);
}

FGameplayAttribute UHarmoniaAbilitySystemLibrary::GetAttributeByTag(const FGameplayTag& StatTag)
{
	// Extract the last part of the tag (e.g., "Stat.Primary.Vitality" -> "Vitality")
	FString TagString = StatTag.ToString();
	FString AttributeName;
	TagString.Split(TEXT("."), nullptr, &AttributeName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	
	return HarmoniaAttributeMapping::FindAttribute(AttributeName);
}

const UHarmoniaAttributeSet* UHarmoniaAbilitySystemLibrary::GetAttributeSetChecked(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	return AbilitySystemComponent->GetSet<UHarmoniaAttributeSet>();
}
