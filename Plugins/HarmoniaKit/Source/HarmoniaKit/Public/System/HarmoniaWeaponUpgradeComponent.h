// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "System/HarmoniaCombatPowerCalculator.h"
#include "HarmoniaWeaponUpgradeComponent.generated.h"

/**
 * Weapon Upgrade Component
 * Manages weapon enhancement, refinement, and infusion
 *
 * Features:
 * - Weapon upgrade levels (+1, +2, ... +10)
 * - Elemental infusions (Fire, Ice, Lightning, etc.)
 * - Special effects and enchantments
 * - Material requirements
 * - Upgrade costs
 */
UCLASS(ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaWeaponUpgradeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaWeaponUpgradeComponent();

	// ============================================================================
	// Upgrade Management
	// ============================================================================

	/** Get current upgrade level */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade")
	int32 GetUpgradeLevel() const { return UpgradeLevel; }

	/** Get maximum upgrade level */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade")
	int32 GetMaxUpgradeLevel() const { return MaxUpgradeLevel; }

	/** Can upgrade weapon? */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade")
	bool CanUpgrade() const;

	/** Upgrade weapon to next level */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade")
	bool UpgradeWeapon();

	/** Get upgrade damage bonus */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade")
	float GetUpgradeDamageBonus() const;

	// ============================================================================
	// Infusion System
	// ============================================================================

	/** Get current infusion type */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade|Infusion")
	EHarmoniaElementType GetInfusionType() const { return InfusionType; }

	/** Can infuse weapon? */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade|Infusion")
	bool CanInfuse(EHarmoniaElementType ElementType) const;

	/** Infuse weapon with element */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade|Infusion")
	bool InfuseWeapon(EHarmoniaElementType ElementType);

	/** Remove infusion */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade|Infusion")
	void RemoveInfusion();

	/** Get infusion damage bonus */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade|Infusion")
	float GetInfusionDamageBonus() const;

	// ============================================================================
	// Requirements
	// ============================================================================

	/** Get required materials for upgrade */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade")
	TArray<FName> GetRequiredMaterials(int32 TargetLevel) const;

	/** Get required currency for upgrade */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade")
	int32 GetRequiredCurrency(int32 TargetLevel) const;

	// ============================================================================
	// Stats
	// ============================================================================

	/** Get total damage multiplier (base + upgrade + infusion) */
	UFUNCTION(BlueprintPure, Category = "Weapon Upgrade")
	float GetTotalDamageMultiplier() const;

	/** Get total stat modifiers */
	UFUNCTION(BlueprintCallable, Category = "Weapon Upgrade")
	void GetTotalStatModifiers(TArray<FEquipmentStatModifier>& OutModifiers) const;

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/** Maximum upgrade level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Upgrade")
	int32 MaxUpgradeLevel = 10;

	/** Damage bonus per upgrade level (percentage) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Upgrade")
	float DamageBonusPerLevel = 0.1f; // 10% per level

	/** Infusion damage bonus (percentage) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Upgrade|Infusion")
	float InfusionDamageBonus = 0.15f; // 15% bonus

	/** Currency cost multiplier per level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Upgrade|Cost")
	float CostMultiplierPerLevel = 1.5f;

	/** Base upgrade cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Upgrade|Cost")
	int32 BaseUpgradeCost = 100;

	// ============================================================================
	// State
	// ============================================================================

	/** Current upgrade level */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Upgrade|State", Replicated)
	int32 UpgradeLevel = 0;

	/** Current infusion type */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Upgrade|State", Replicated)
	EHarmoniaElementType InfusionType = EHarmoniaElementType::None;

	/** Custom stat modifiers from upgrades */
	UPROPERTY()
	TArray<FEquipmentStatModifier> UpgradeStatModifiers;

	// ============================================================================
	// Replication
	// ============================================================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Called when upgrade level changes */
	UFUNCTION()
	void OnRep_UpgradeLevel();

	/** Called when infusion changes */
	UFUNCTION()
	void OnRep_InfusionType();

private:
	/** Recalculate stat modifiers */
	void RecalculateStatModifiers();
};

/**
 * Weapon Upgrade Data
 * Data table row for weapon upgrade requirements
 */
USTRUCT(BlueprintType)
struct FHarmoniaWeaponUpgradeData : public FTableRowBase
{
	GENERATED_BODY()

	/** Upgrade level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	int32 UpgradeLevel = 1;

	/** Required materials */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TMap<FName, int32> RequiredMaterials;

	/** Required currency amount */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	int32 RequiredCurrency = 100;

	/** Damage bonus multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	float DamageMultiplier = 1.1f;

	/** Additional stat modifiers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Upgrade")
	TArray<FEquipmentStatModifier> StatModifiers;
};
