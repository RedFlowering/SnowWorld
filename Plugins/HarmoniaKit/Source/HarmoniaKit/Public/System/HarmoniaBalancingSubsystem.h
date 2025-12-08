// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaBalancingSubsystem.generated.h"

class UDataTable;

/**
 * Balancing Subsystem
 * Runtime balancing and difficulty scaling for melee combat
 *
 * Features:
 * - Global damage multipliers
 * - Difficulty presets
 * - Player level scaling
 * - Enemy level scaling
 * - Loot quality scaling
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaBalancingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	// ============================================================================
	// Difficulty Management
	// ============================================================================

	/** Get current difficulty level */
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	int32 GetDifficultyLevel() const { return DifficultyLevel; }

	/** Set difficulty level (0 = Easy, 1 = Normal, 2 = Hard, 3 = Very Hard) */
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void SetDifficultyLevel(int32 NewDifficulty);

	// ============================================================================
	// Damage Scaling
	// ============================================================================

	/** Get player damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Balancing")
	float GetPlayerDamageMultiplier() const;

	/** Get enemy damage multiplier */
	UFUNCTION(BlueprintPure, Category = "Balancing")
	float GetEnemyDamageMultiplier() const;

	/** Set custom damage multipliers */
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void SetDamageMultipliers(float PlayerDamage, float EnemyDamage);

	// ============================================================================
	// Level Scaling
	// ============================================================================

	/** Calculate scaled damage based on level difference */
	UFUNCTION(BlueprintPure, Category = "Balancing")
	float CalculateScaledDamage(float BaseDamage, int32 AttackerLevel, int32 DefenderLevel) const;

	/** Calculate scaled health based on level */
	UFUNCTION(BlueprintPure, Category = "Balancing")
	float CalculateScaledHealth(float BaseHealth, int32 Level) const;

	/** Calculate scaled stamina based on level */
	UFUNCTION(BlueprintPure, Category = "Balancing")
	float CalculateScaledStamina(float BaseStamina, int32 Level) const;

	// ============================================================================
	// Weapon Balancing
	// ============================================================================

	/** Get balanced weapon stats for level */
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void GetBalancedWeaponStats(
		FGameplayTag WeaponTypeTag,
		int32 WeaponLevel,
		float& OutDamage,
		float& OutStaminaCost,
		float& OutAttackSpeed
	) const;

	/** Apply weapon upgrade */
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	float GetWeaponUpgradeMultiplier(int32 UpgradeLevel) const;

	// ============================================================================
	// Enemy Balancing
	// ============================================================================

	/** Get enemy stat multiplier based on area/zone */
	UFUNCTION(BlueprintPure, Category = "Balancing")
	float GetAreaDifficultyMultiplier(FName AreaName) const;

	/** Register area difficulty */
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void RegisterAreaDifficulty(FName AreaName, float Multiplier);

protected:
	// ============================================================================
	// Difficulty Settings
	// ============================================================================

	/** Current difficulty level */
	UPROPERTY()
	int32 DifficultyLevel = 1; // 0=Easy, 1=Normal, 2=Hard, 3=Very Hard

	/** Player damage multiplier per difficulty */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing")
	TArray<float> PlayerDamageMultipliers = {1.2f, 1.0f, 0.8f, 0.6f};

	/** Enemy damage multiplier per difficulty */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing")
	TArray<float> EnemyDamageMultipliers = {0.7f, 1.0f, 1.3f, 1.8f};

	/** Enemy health multiplier per difficulty */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing")
	TArray<float> EnemyHealthMultipliers = {0.8f, 1.0f, 1.5f, 2.0f};

	// ============================================================================
	// Scaling Curves
	// ============================================================================

	/** Level scaling curve for damage */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing|Curves")
	TObjectPtr<UCurveFloat> DamageScalingCurve;

	/** Level scaling curve for health */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing|Curves")
	TObjectPtr<UCurveFloat> HealthScalingCurve;

	/** Level scaling curve for stamina */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing|Curves")
	TObjectPtr<UCurveFloat> StaminaScalingCurve;

	/** Weapon upgrade scaling curve */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing|Curves")
	TObjectPtr<UCurveFloat> WeaponUpgradeCurve;

	// ============================================================================
	// Area Difficulty
	// ============================================================================

	/** Area-based difficulty multipliers */
	UPROPERTY()
	TMap<FName, float> AreaDifficultyMap;

	// ============================================================================
	// Data Tables
	// ============================================================================

	/** Weapon balancing data table */
	UPROPERTY(EditDefaultsOnly, Category = "Balancing|Data")
	TObjectPtr<UDataTable> WeaponBalancingTable;
};
