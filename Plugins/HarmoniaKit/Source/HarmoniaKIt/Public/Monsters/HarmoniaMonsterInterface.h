// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "HarmoniaMonsterInterface.generated.h"

class UHarmoniaMonsterData;
struct FHarmoniaLootTableRow;

UINTERFACE(MinimalAPI, BlueprintType)
class UHarmoniaMonsterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IHarmoniaMonsterInterface
 *
 * Interface that all monsters must implement
 * Provides common functionality for monsters of any type (humanoid, creature, mechanical, etc.)
 * Can be implemented in C++ or Blueprint
 */
class HARMONIAKIT_API IHarmoniaMonsterInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Monster Data
	// ============================================================================

	/**
	 * Get the monster data asset
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster")
	UHarmoniaMonsterData* GetMonsterData() const;

	/**
	 * Get the monster's current level
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster")
	int32 GetMonsterLevel() const;

	/**
	 * Get the monster's display name
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster")
	FText GetMonsterName() const;

	/**
	 * Get the monster's faction
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster")
	EHarmoniaMonsterFaction GetFaction() const;

	// ============================================================================
	// Monster State
	// ============================================================================

	/**
	 * Get the monster's current state
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|State")
	EHarmoniaMonsterState GetMonsterState() const;

	/**
	 * Set the monster's state
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|State")
	void SetMonsterState(EHarmoniaMonsterState NewState);

	/**
	 * Check if monster is in combat
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|State")
	bool IsInCombat() const;

	/**
	 * Check if monster is dead
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|State")
	bool IsDead() const;

	// ============================================================================
	// Combat
	// ============================================================================

	/**
	 * Called when monster takes damage
	 * @param DamageAmount Amount of damage taken
	 * @param DamageInstigator Actor who dealt the damage
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|Combat")
	void OnDamageTaken(float DamageAmount, AActor* DamageInstigator);

	/**
	 * Called when monster's health reaches zero
	 * @param Killer Actor who dealt the killing blow
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|Combat")
	void OnDeath(AActor* Killer);

	/**
	 * Get the current target actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|Combat")
	AActor* GetCurrentTarget() const;

	/**
	 * Set the current target actor
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|Combat")
	void SetCurrentTarget(AActor* NewTarget);

	// ============================================================================
	// Loot & Rewards
	// ============================================================================

	/**
	 * Generate loot from the monster's loot table
	 * @param Killer Actor who killed the monster
	 * @return Array of loot items generated
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|Loot")
	TArray<FHarmoniaLootTableRow> GenerateLoot(AActor* Killer);

	/**
	 * Spawn loot in the world
	 * @param LootItems Items to spawn
	 * @param SpawnLocation Location to spawn loot
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|Loot")
	void SpawnLoot(const TArray<FHarmoniaLootTableRow>& LootItems, const FVector& SpawnLocation);

	// ============================================================================
	// AI Behavior
	// ============================================================================

	/**
	 * Get the monster's aggro type
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|AI")
	EHarmoniaMonsterAggroType GetAggroType() const;

	/**
	 * Get the monster's aggro range
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|AI")
	float GetAggroRange() const;

	/**
	 * Called when monster detects a potential target
	 * @param DetectedActor Actor that was detected
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|AI")
	void OnTargetDetected(AActor* DetectedActor);

	/**
	 * Called when monster loses sight of target
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster|AI")
	void OnTargetLost();
};
