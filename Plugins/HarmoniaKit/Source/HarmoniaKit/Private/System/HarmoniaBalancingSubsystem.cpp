// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaBalancingSubsystem.h"
#include "Curves/CurveFloat.h"

void UHarmoniaBalancingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default area difficulties
	RegisterAreaDifficulty(FName("StartingArea"), 0.8f);
	RegisterAreaDifficulty(FName("MidArea"), 1.0f);
	RegisterAreaDifficulty(FName("LateArea"), 1.5f);
	RegisterAreaDifficulty(FName("EndGame"), 2.0f);
	RegisterAreaDifficulty(FName("BossArea"), 2.5f);
}

void UHarmoniaBalancingSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ============================================================================
// Difficulty Management
// ============================================================================

void UHarmoniaBalancingSubsystem::SetDifficultyLevel(int32 NewDifficulty)
{
	DifficultyLevel = FMath::Clamp(NewDifficulty, 0, 3);
}

// ============================================================================
// Damage Scaling
// ============================================================================

float UHarmoniaBalancingSubsystem::GetPlayerDamageMultiplier() const
{
	if (PlayerDamageMultipliers.IsValidIndex(DifficultyLevel))
	{
		return PlayerDamageMultipliers[DifficultyLevel];
	}
	return 1.0f;
}

float UHarmoniaBalancingSubsystem::GetEnemyDamageMultiplier() const
{
	if (EnemyDamageMultipliers.IsValidIndex(DifficultyLevel))
	{
		return EnemyDamageMultipliers[DifficultyLevel];
	}
	return 1.0f;
}

void UHarmoniaBalancingSubsystem::SetDamageMultipliers(float PlayerDamage, float EnemyDamage)
{
	if (PlayerDamageMultipliers.IsValidIndex(DifficultyLevel))
	{
		PlayerDamageMultipliers[DifficultyLevel] = PlayerDamage;
	}

	if (EnemyDamageMultipliers.IsValidIndex(DifficultyLevel))
	{
		EnemyDamageMultipliers[DifficultyLevel] = EnemyDamage;
	}
}

// ============================================================================
// Level Scaling
// ============================================================================

float UHarmoniaBalancingSubsystem::CalculateScaledDamage(float BaseDamage, int32 AttackerLevel, int32 DefenderLevel) const
{
	float ScaledDamage = BaseDamage;

	// Apply level difference scaling
	int32 LevelDifference = AttackerLevel - DefenderLevel;

	if (DamageScalingCurve)
	{
		float ScaleMultiplier = DamageScalingCurve->GetFloatValue(static_cast<float>(LevelDifference));
		ScaledDamage *= ScaleMultiplier;
	}
	else
	{
		// Default scaling: +5% per level above, -5% per level below
		float LevelScaling = 1.0f + (LevelDifference * 0.05f);
		LevelScaling = FMath::Clamp(LevelScaling, 0.5f, 2.0f);
		ScaledDamage *= LevelScaling;
	}

	return ScaledDamage;
}

float UHarmoniaBalancingSubsystem::CalculateScaledHealth(float BaseHealth, int32 Level) const
{
	if (HealthScalingCurve)
	{
		return BaseHealth * HealthScalingCurve->GetFloatValue(static_cast<float>(Level));
	}

	// Default: +10% per level
	return BaseHealth * (1.0f + (Level - 1) * 0.1f);
}

float UHarmoniaBalancingSubsystem::CalculateScaledStamina(float BaseStamina, int32 Level) const
{
	if (StaminaScalingCurve)
	{
		return BaseStamina * StaminaScalingCurve->GetFloatValue(static_cast<float>(Level));
	}

	// Default: +5% per level
	return BaseStamina * (1.0f + (Level - 1) * 0.05f);
}

// ============================================================================
// Weapon Balancing
// ============================================================================

void UHarmoniaBalancingSubsystem::GetBalancedWeaponStats(
	EHarmoniaMeleeWeaponType WeaponType,
	int32 WeaponLevel,
	float& OutDamage,
	float& OutStaminaCost,
	float& OutAttackSpeed) const
{
	// Base stats from weapon type
	float BaseDamage = 10.0f;
	float BaseStamina = 10.0f;
	float BaseSpeed = 1.0f;

	// Apply weapon type multipliers (should come from data table)
	switch (WeaponType)
	{
	case EHarmoniaMeleeWeaponType::Sword:
		BaseDamage = 15.0f;
		BaseStamina = 10.0f;
		BaseSpeed = 1.0f;
		break;
	case EHarmoniaMeleeWeaponType::GreatSword:
		BaseDamage = 30.0f;
		BaseStamina = 20.0f;
		BaseSpeed = 0.7f;
		break;
	case EHarmoniaMeleeWeaponType::Dagger:
		BaseDamage = 8.0f;
		BaseStamina = 6.0f;
		BaseSpeed = 1.4f;
		break;
	// Add other weapon types...
	default:
		break;
	}

	// Apply level scaling
	float UpgradeMultiplier = GetWeaponUpgradeMultiplier(WeaponLevel);

	OutDamage = BaseDamage * UpgradeMultiplier;
	OutStaminaCost = BaseStamina;
	OutAttackSpeed = BaseSpeed;
}

float UHarmoniaBalancingSubsystem::GetWeaponUpgradeMultiplier(int32 UpgradeLevel) const
{
	if (WeaponUpgradeCurve)
	{
		return WeaponUpgradeCurve->GetFloatValue(static_cast<float>(UpgradeLevel));
	}

	// Default: +10% per upgrade level
	return 1.0f + (UpgradeLevel * 0.1f);
}

// ============================================================================
// Enemy Balancing
// ============================================================================

float UHarmoniaBalancingSubsystem::GetAreaDifficultyMultiplier(FName AreaName) const
{
	if (const float* Multiplier = AreaDifficultyMap.Find(AreaName))
	{
		return *Multiplier;
	}
	return 1.0f;
}

void UHarmoniaBalancingSubsystem::RegisterAreaDifficulty(FName AreaName, float Multiplier)
{
	AreaDifficultyMap.Add(AreaName, Multiplier);
}
