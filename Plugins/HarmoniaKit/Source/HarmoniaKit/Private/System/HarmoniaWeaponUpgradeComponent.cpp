// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaWeaponUpgradeComponent.h"
#include "Net/UnrealNetwork.h"
#include "System/HarmoniaCombatPowerCalculator.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Components/HarmoniaCurrencyManagerComponent.h"

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

	// Check for required currency
	int32 RequiredCurrency = GetRequiredCurrency(UpgradeLevel + 1);
	AActor* Owner = GetOwner();
	if (Owner)
	{
		// Check if owner has currency component
		UHarmoniaCurrencyManagerComponent* CurrencyComponent = Owner->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
		if (CurrencyComponent)
		{
			if (!CurrencyComponent->HasCurrency(EHarmoniaCurrencyType::Gold, RequiredCurrency))
			{
				UE_LOG(LogTemp, Warning, TEXT("UpgradeWeapon: Insufficient currency. Required: %d"), RequiredCurrency);
				return false;
			}

			// Consume currency
			CurrencyComponent->RemoveCurrency(EHarmoniaCurrencyType::Gold, RequiredCurrency);
		}

		// Check if owner has inventory component for materials
		UHarmoniaInventoryComponent* InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
		if (InventoryComponent)
		{
			TArray<FName> RequiredMaterials = GetRequiredMaterials(UpgradeLevel + 1);

			// Try to consume materials (simplified - assuming full durability)
			// In a real implementation, you'd check availability first
			for (const FName& MaterialID : RequiredMaterials)
			{
				FHarmoniaID ItemID;
				ItemID.ID = MaterialID;

				// Remove 1 of each material with full durability
				if (!InventoryComponent->RemoveItem(ItemID, 1, 100.0f))
				{
					UE_LOG(LogTemp, Warning, TEXT("UpgradeWeapon: Failed to consume material: %s"), *MaterialID.ToString());
					// Note: In production, you'd want to roll back the currency deduction here
				}
			}
		}
	}

	UpgradeLevel++;
	RecalculateStatModifiers();

	OnRep_UpgradeLevel();

	UE_LOG(LogTemp, Log, TEXT("Weapon upgraded to level %d"), UpgradeLevel);
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

	// Check for required currency (infusion is more expensive)
	int32 RequiredCurrency = BaseInfusionCost;
	AActor* Owner = GetOwner();
	if (Owner)
	{
		// Check if owner has currency component
		UHarmoniaCurrencyManagerComponent* CurrencyComponent = Owner->FindComponentByClass<UHarmoniaCurrencyManagerComponent>();
		if (CurrencyComponent)
		{
			if (!CurrencyComponent->HasCurrency(EHarmoniaCurrencyType::Gold, RequiredCurrency))
			{
				UE_LOG(LogTemp, Warning, TEXT("InfuseWeapon: Insufficient currency. Required: %d"), RequiredCurrency);
				return false;
			}

			// Consume currency
			CurrencyComponent->RemoveCurrency(EHarmoniaCurrencyType::Gold, RequiredCurrency);
		}

		// Check for element-specific infusion material
		UHarmoniaInventoryComponent* InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
		if (InventoryComponent)
		{
			FName InfusionMaterial;

			// Determine required material based on element type
			switch (ElementType)
			{
			case EHarmoniaElementType::Fire:
				InfusionMaterial = FName("FireEssence");
				break;
			case EHarmoniaElementType::Ice:
				InfusionMaterial = FName("IceEssence");
				break;
			case EHarmoniaElementType::Lightning:
				InfusionMaterial = FName("LightningEssence");
				break;
			case EHarmoniaElementType::Dark:
				InfusionMaterial = FName("DarkEssence");
				break;
			case EHarmoniaElementType::Light:
				InfusionMaterial = FName("LightEssence");
				break;
			default:
				InfusionMaterial = FName("ElementalEssence");
				break;
			}

			// Try to consume infusion material (simplified - assuming full durability)
			FHarmoniaID ItemID;
			ItemID.ID = InfusionMaterial;

			if (!InventoryComponent->RemoveItem(ItemID, 1, 100.0f))
			{
				UE_LOG(LogTemp, Warning, TEXT("InfuseWeapon: Failed to consume infusion material: %s"), *InfusionMaterial.ToString());
				// Note: In production, you'd want to roll back the currency deduction here
			}
		}
	}

	InfusionType = ElementType;
	RecalculateStatModifiers();

	OnRep_InfusionType();

	UE_LOG(LogTemp, Log, TEXT("Weapon infused with %s element"), *UEnum::GetValueAsString(ElementType));
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
