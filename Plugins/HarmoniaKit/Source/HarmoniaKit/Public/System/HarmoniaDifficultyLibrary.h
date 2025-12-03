// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Definitions/HarmoniaDifficultySystemDefinitions.h"
#include "HarmoniaDifficultyLibrary.generated.h"

class UHarmoniaDifficultySubsystem;

/**
 * Harmonia Difficulty Library
 * Blueprint function library for accessing difficulty system
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDifficultyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================
	// Subsystem Access
	// ========================================

	/**
	 * Get the Difficulty Subsystem from a world context object
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaDifficultySubsystem* GetDifficultySubsystem(const UObject* WorldContextObject);

	// ========================================
	// Pact Management
	// ========================================

	/**
	 * Toggle a difficulty pact
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty", meta = (WorldContext = "WorldContextObject"))
	static bool ToggleDifficultyPact(const UObject* WorldContextObject, EHarmoniaDifficultyPactType PactType, bool bActivate);

	/**
	 * Check if a pact is active
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty", meta = (WorldContext = "WorldContextObject"))
	static bool IsDifficultyPactActive(const UObject* WorldContextObject, EHarmoniaDifficultyPactType PactType);

	/**
	 * Get all active pacts
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty", meta = (WorldContext = "WorldContextObject"))
	static TArray<FHarmoniaDifficultyPactConfig> GetActiveDifficultyPacts(const UObject* WorldContextObject);

	/**
	 * Get all available pacts
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty", meta = (WorldContext = "WorldContextObject"))
	static TArray<FHarmoniaDifficultyPactConfig> GetAllDifficultyPacts(const UObject* WorldContextObject);

	/**
	 * Get total difficulty multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty", meta = (WorldContext = "WorldContextObject"))
	static float GetTotalDifficultyMultiplier(const UObject* WorldContextObject);

	// ========================================
	// Combat Modifiers
	// ========================================

	/**
	 * Apply difficulty modifiers to player damage
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Combat", meta = (WorldContext = "WorldContextObject"))
	static float ApplyPlayerDamageModifier(const UObject* WorldContextObject, float BaseDamage);

	/**
	 * Apply difficulty modifiers to damage taken by player
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Combat", meta = (WorldContext = "WorldContextObject"))
	static float ApplyPlayerDamageTakenModifier(const UObject* WorldContextObject, float BaseDamage);

	/**
	 * Apply difficulty modifiers to enemy damage
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Combat", meta = (WorldContext = "WorldContextObject"))
	static float ApplyEnemyDamageModifier(const UObject* WorldContextObject, float BaseDamage);

	/**
	 * Apply difficulty modifiers to enemy health
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Combat", meta = (WorldContext = "WorldContextObject"))
	static float ApplyEnemyHealthModifier(const UObject* WorldContextObject, float BaseHealth);

	/**
	 * Check if player can block (not disabled by pacts)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Combat", meta = (WorldContext = "WorldContextObject"))
	static bool CanPlayerBlock(const UObject* WorldContextObject);

	/**
	 * Check if player can parry (not disabled by pacts)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Combat", meta = (WorldContext = "WorldContextObject"))
	static bool CanPlayerParry(const UObject* WorldContextObject);

	// ========================================
	// Economy Modifiers
	// ========================================

	/**
	 * Apply difficulty modifiers to soul gain
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Economy", meta = (WorldContext = "WorldContextObject"))
	static int32 ApplySoulGainModifier(const UObject* WorldContextObject, int32 BaseSouls);

	/**
	 * Apply difficulty modifiers to experience gain
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Economy", meta = (WorldContext = "WorldContextObject"))
	static int32 ApplyExperienceGainModifier(const UObject* WorldContextObject, int32 BaseExperience);

	/**
	 * Apply difficulty modifiers to vendor prices
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Economy", meta = (WorldContext = "WorldContextObject"))
	static int32 ApplyVendorPriceModifier(const UObject* WorldContextObject, int32 BasePrice);

	/**
	 * Apply difficulty modifiers to healing effectiveness
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Economy", meta = (WorldContext = "WorldContextObject"))
	static float ApplyHealingModifier(const UObject* WorldContextObject, float BaseHealing);

	/**
	 * Check if item should drop based on difficulty modifiers
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Economy", meta = (WorldContext = "WorldContextObject"))
	static bool ShouldItemDrop(const UObject* WorldContextObject, float BaseDropChance, bool bIsRare = false, bool bIsLegendary = false);

	// ========================================
	// New Game Plus
	// ========================================

	/**
	 * Start New Game Plus
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|NewGamePlus", meta = (WorldContext = "WorldContextObject"))
	static void StartNewGamePlus(const UObject* WorldContextObject);

	/**
	 * Complete current playthrough
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|NewGamePlus", meta = (WorldContext = "WorldContextObject"))
	static void CompletePlaythrough(const UObject* WorldContextObject, float CompletionTime);

	/**
	 * Get New Game Plus tier
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|NewGamePlus", meta = (WorldContext = "WorldContextObject"))
	static EHarmoniaNewGamePlusTier GetNewGamePlusTier(const UObject* WorldContextObject);

	/**
	 * Get New Game Plus legacy data
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|NewGamePlus", meta = (WorldContext = "WorldContextObject"))
	static FHarmoniaNewGamePlusLegacy GetNewGamePlusLegacy(const UObject* WorldContextObject);

	/**
	 * Check if item is unlocked in New Game Plus
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|NewGamePlus", meta = (WorldContext = "WorldContextObject"))
	static bool IsNewGamePlusItemUnlocked(const UObject* WorldContextObject, FName ItemName, const FString& ItemType);

	// ========================================
	// Dynamic Difficulty
	// ========================================

	/**
	 * Set dynamic difficulty mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic", meta = (WorldContext = "WorldContextObject"))
	static void SetDynamicDifficultyMode(const UObject* WorldContextObject, EHarmoniaDynamicDifficultyMode Mode);

	/**
	 * Report player death
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic", meta = (WorldContext = "WorldContextObject"))
	static void ReportPlayerDeath(const UObject* WorldContextObject);

	/**
	 * Report player victory
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic", meta = (WorldContext = "WorldContextObject"))
	static void ReportPlayerVictory(const UObject* WorldContextObject, float HealthRemainingPercent, float CombatDuration);

	/**
	 * Report perfect defense action
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic", meta = (WorldContext = "WorldContextObject"))
	static void ReportPerfectDefense(const UObject* WorldContextObject, bool bWasParry);

	// ========================================
	// Utility Functions
	// ========================================

	/**
	 * Get localized pact name
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Utility")
	static FText GetPactTypeName(EHarmoniaDifficultyPactType PactType);

	/**
	 * Get localized New Game Plus tier name
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Utility")
	static FText GetNewGamePlusTierName(EHarmoniaNewGamePlusTier Tier);

	/**
	 * Calculate recommended level for New Game Plus tier
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Utility")
	static int32 GetRecommendedLevelForTier(EHarmoniaNewGamePlusTier Tier);
};
