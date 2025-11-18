// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaWeaponUpgradeComponent.h"
#include "Net/UnrealNetwork.h"
#include "System/HarmoniaCombatPowerCalculator.h"

UHarmoniaWeaponUpgradeComponent::UHarmoniaWeaponUpgradeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaWeaponUpgradeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaWeaponUpgradeComponent, UpgradeLevel);
	DOREPLIFETIME(UHarmoniaWeaponUpgradeComponent, InfusionType);
}

// ============================================================================
// Upgrade Management
// ============================================================================

bool UHarmoniaWeaponUpgradeComponent::CanUpgrade() const
{
	return UpgradeLevel < MaxUpgradeLevel;
}

bool UHarmoniaWeaponUpgradeComponent::UpgradeWeapon()
{
	if (!CanUpgrade())
	{
		return false;
	}

	// TODO: Check for materials and currency

	UpgradeLevel++;
	RecalculateStatModifiers();

	OnRep_UpgradeLevel();

	return true;
}

float UHarmoniaWeaponUpgradeComponent::GetUpgradeDamageBonus() const
{
	return UpgradeLevel * DamageBonusPerLevel;
}

// ============================================================================
// Infusion System
// ============================================================================

bool UHarmoniaWeaponUpgradeComponent::CanInfuse(EHarmoniaElementType ElementType) const
{
	if (ElementType == EHarmoniaElementType::None)
	{
		return false;
	}

	// Can always change infusion
	return true;
}

bool UHarmoniaWeaponUpgradeComponent::InfuseWeapon(EHarmoniaElementType ElementType)
{
	if (!CanInfuse(ElementType))
	{
		return false;
	}

	// TODO: Check for materials and currency

	InfusionType = ElementType;
	RecalculateStatModifiers();

	OnRep_InfusionType();

	return true;
}

void UHarmoniaWeaponUpgradeComponent::RemoveInfusion()
{
	InfusionType = EHarmoniaElementType::None;
	RecalculateStatModifiers();

	OnRep_InfusionType();
}

float UHarmoniaWeaponUpgradeComponent::GetInfusionDamageBonus() const
{
	if (InfusionType == EHarmoniaElementType::None)
	{
		return 0.0f;
	}

	return InfusionDamageBonus;
}

// ============================================================================
// Requirements
// ============================================================================

TArray<FName> UHarmoniaWeaponUpgradeComponent::GetRequiredMaterials(int32 TargetLevel) const
{
	TArray<FName> Materials;

	// Example materials based on level
	if (TargetLevel <= 3)
	{
		Materials.Add(FName("IronOre"));
	}
	else if (TargetLevel <= 6)
	{
		Materials.Add(FName("SteelIngot"));
	}
	else if (TargetLevel <= 9)
	{
		Materials.Add(FName("MithrilBar"));
	}
	else
	{
		Materials.Add(FName("AdamantiteBar"));
	}

	return Materials;
}

int32 UHarmoniaWeaponUpgradeComponent::GetRequiredCurrency(int32 TargetLevel) const
{
	return BaseUpgradeCost * FMath::Pow(CostMultiplierPerLevel, TargetLevel - 1);
}

// ============================================================================
// Stats
// ============================================================================

float UHarmoniaWeaponUpgradeComponent::GetTotalDamageMultiplier() const
{
	float Total = 1.0f;

	// Add upgrade bonus
	Total += GetUpgradeDamageBonus();

	// Add infusion bonus
	Total += GetInfusionDamageBonus();

	return Total;
}

void UHarmoniaWeaponUpgradeComponent::GetTotalStatModifiers(TArray<FEquipmentStatModifier>& OutModifiers) const
{
	OutModifiers = UpgradeStatModifiers;
}

// ============================================================================
// Private Methods
// ============================================================================

void UHarmoniaWeaponUpgradeComponent::RecalculateStatModifiers()
{
	UpgradeStatModifiers.Empty();

	// Add damage modifier
	FEquipmentStatModifier DamageMod;
	DamageMod.AttributeName = "AttackPower";
	DamageMod.ModifierType = EStatModifierType::Percentage;
	DamageMod.Value = GetUpgradeDamageBonus() + GetInfusionDamageBonus();
	UpgradeStatModifiers.Add(DamageMod);

	// Add element-specific modifiers
	if (InfusionType != EHarmoniaElementType::None)
	{
		FEquipmentStatModifier ElementMod;
		ElementMod.AttributeName = FString::Printf(TEXT("%sDamage"), *UEnum::GetValueAsString(InfusionType));
		ElementMod.ModifierType = EStatModifierType::Flat;
		ElementMod.Value = 10.0f * (UpgradeLevel + 1);
		UpgradeStatModifiers.Add(ElementMod);
	}
}

void UHarmoniaWeaponUpgradeComponent::OnRep_UpgradeLevel()
{
	// Notify UI or other systems
	RecalculateStatModifiers();
}

void UHarmoniaWeaponUpgradeComponent::OnRep_InfusionType()
{
	// Notify UI or other systems
	RecalculateStatModifiers();
}
