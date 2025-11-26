// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "HarmoniaAbilitySystemLibrary.generated.h"

class UAbilitySystemComponent;
class UHarmoniaAttributeSet;

/**
 * Blueprint function library for Harmonia Ability System
 * Provides helper functions for stamina, health, and stat management
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Stamina Functions
	// ============================================================================

	/**
	 * Check if the character has enough stamina
	 * @param AbilitySystemComponent The ability system component to check
	 * @param StaminaCost The amount of stamina required
	 * @return True if character has enough stamina
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Stamina")
	static bool HasEnoughStamina(UAbilitySystemComponent* AbilitySystemComponent, float StaminaCost);

	/**
	 * Consume stamina instantly
	 * @param AbilitySystemComponent The ability system component
	 * @param StaminaCost The amount of stamina to consume
	 * @return True if stamina was consumed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Stamina")
	static bool ConsumeStamina(UAbilitySystemComponent* AbilitySystemComponent, float StaminaCost);

	/**
	 * Get current stamina value
	 * @param AbilitySystemComponent The ability system component
	 * @return Current stamina value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Stamina")
	static float GetStamina(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get max stamina value
	 * @param AbilitySystemComponent The ability system component
	 * @return Max stamina value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Stamina")
	static float GetMaxStamina(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get stamina as a percentage (0-1)
	 * @param AbilitySystemComponent The ability system component
	 * @return Stamina percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Stamina")
	static float GetStaminaPercent(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Restore stamina
	 * @param AbilitySystemComponent The ability system component
	 * @param Amount The amount to restore
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Stamina")
	static void RestoreStamina(UAbilitySystemComponent* AbilitySystemComponent, float Amount);

	// ============================================================================
	// Health Functions
	// ============================================================================

	/**
	 * Get current health value
	 * @param AbilitySystemComponent The ability system component
	 * @return Current health value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Health")
	static float GetHealth(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get max health value
	 * @param AbilitySystemComponent The ability system component
	 * @return Max health value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Health")
	static float GetMaxHealth(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get health as a percentage (0-1)
	 * @param AbilitySystemComponent The ability system component
	 * @return Health percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Health")
	static float GetHealthPercent(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Restore health
	 * @param AbilitySystemComponent The ability system component
	 * @param Amount The amount to restore
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Health")
	static void RestoreHealth(UAbilitySystemComponent* AbilitySystemComponent, float Amount);

	// ============================================================================
	// Poise Functions
	// ============================================================================

	/**
	 * Get current poise value
	 * @param AbilitySystemComponent The ability system component
	 * @return Current poise value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Poise")
	static float GetPoise(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get max poise value
	 * @param AbilitySystemComponent The ability system component
	 * @return Max poise value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Poise")
	static float GetMaxPoise(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Check if poise is broken
	 * @param AbilitySystemComponent The ability system component
	 * @return True if poise is at zero
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Poise")
	static bool IsPoiseBroken(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Reset poise to maximum
	 * @param AbilitySystemComponent The ability system component
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Poise")
	static void ResetPoise(UAbilitySystemComponent* AbilitySystemComponent);

	// ============================================================================
	// Primary Stats Functions
	// ============================================================================

	/**
	 * Get a primary stat value (Vitality, Endurance, Strength, etc.)
	 * @param AbilitySystemComponent The ability system component
	 * @param StatName The name of the stat (Vitality, Endurance, Strength, Dexterity, Intelligence, Faith, Luck)
	 * @return The stat value
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Stats")
	static float GetPrimaryStat(UAbilitySystemComponent* AbilitySystemComponent, FName StatName);

	/**
	 * Set a primary stat value
	 * @param AbilitySystemComponent The ability system component
	 * @param StatName The name of the stat
	 * @param NewValue The new value (clamped to 1-99)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Stats")
	static void SetPrimaryStat(UAbilitySystemComponent* AbilitySystemComponent, FName StatName, float NewValue);

	// ============================================================================
	// Equipment Load Functions
	// ============================================================================

	/**
	 * Get equipment load percentage (0-1+)
	 * @param AbilitySystemComponent The ability system component
	 * @return Equipment load percentage
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	static float GetEquipLoadPercent(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get equipment load roll type
	 * @param AbilitySystemComponent The ability system component
	 * @return Roll type: "Fast" (0-30%), "Medium" (30-70%), "Fat" (70-100%), "Overburdened" (100%+)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Equipment")
	static FString GetEquipLoadRollType(UAbilitySystemComponent* AbilitySystemComponent);

	// ============================================================================
	// Utility Functions
	// ============================================================================

	/**
	 * Get the Harmonia Attribute Set from an Ability System Component
	 * @param AbilitySystemComponent The ability system component
	 * @return The Harmonia Attribute Set, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Utility")
	static UHarmoniaAttributeSet* GetHarmoniaAttributeSet(UAbilitySystemComponent* AbilitySystemComponent);

	/**
	 * Get a GameplayAttribute from an attribute name string
	 * @param AttributeName The name of the attribute (e.g., "MaxHealth", "AttackPower", "Vitality")
	 * @return The corresponding GameplayAttribute, or invalid if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Utility")
	static FGameplayAttribute GetAttributeByName(const FString& AttributeName);

	/**
	 * Get a GameplayAttribute from a stat tag
	 * @param StatTag The gameplay tag representing the stat
	 * @return The corresponding GameplayAttribute, or invalid if not found
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Harmonia|Utility")
	static FGameplayAttribute GetAttributeByTag(const FGameplayTag& StatTag);

private:
	/**
	 * Helper function to get attribute set
	 */
	static const UHarmoniaAttributeSet* GetAttributeSetChecked(UAbilitySystemComponent* AbilitySystemComponent);
};
