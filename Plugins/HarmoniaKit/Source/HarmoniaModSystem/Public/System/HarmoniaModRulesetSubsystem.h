// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaModSystemDefinitions.h"
#include "HarmoniaModRulesetSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHarmoniaModRuleset, Log, All);

/**
 * Delegate fired when a ruleset is activated
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRulesetActivated, const FHarmoniaCustomRuleset&, Ruleset);

/**
 * Delegate fired when a ruleset is deactivated
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRulesetDeactivated, const FHarmoniaCustomRuleset&, Ruleset);

/**
 * Custom ruleset subsystem
 * Handles custom game rules and difficulty modifiers (e.g., Nuzlocke mode)
 */
UCLASS()
class HARMONIAMODSYSTEM_API UHarmoniaModRulesetSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem interface

	/**
	 * Register a custom ruleset
	 * @param Ruleset - Ruleset to register
	 * @param ModId - ID of the mod registering this ruleset
	 * @return True if registered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Ruleset")
	bool RegisterRuleset(const FHarmoniaCustomRuleset& Ruleset, FName ModId);

	/**
	 * Unregister a ruleset
	 * @param RulesetId - ID of the ruleset to unregister
	 * @return True if unregistered successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Ruleset")
	bool UnregisterRuleset(FName RulesetId);

	/**
	 * Activate a ruleset
	 * @param RulesetId - ID of the ruleset to activate
	 * @return True if activated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Ruleset")
	bool ActivateRuleset(FName RulesetId);

	/**
	 * Deactivate a ruleset
	 * @param RulesetId - ID of the ruleset to deactivate
	 * @return True if deactivated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Ruleset")
	bool DeactivateRuleset(FName RulesetId);

	/**
	 * Deactivate all active rulesets
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Ruleset")
	void DeactivateAllRulesets();

	/**
	 * Get all registered rulesets
	 * @return Array of all rulesets
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	TArray<FHarmoniaCustomRuleset> GetAllRulesets() const;

	/**
	 * Get all active rulesets
	 * @return Array of active rulesets
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	TArray<FHarmoniaCustomRuleset> GetActiveRulesets() const;

	/**
	 * Get ruleset by ID
	 * @param RulesetId - ID of the ruleset
	 * @param OutRuleset - Output ruleset
	 * @return True if ruleset found
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	bool GetRuleset(FName RulesetId, FHarmoniaCustomRuleset& OutRuleset) const;

	/**
	 * Check if a ruleset is active
	 * @param RulesetId - ID of the ruleset
	 * @return True if ruleset is active
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	bool IsRulesetActive(FName RulesetId) const;

	/**
	 * Check if any rule tag is active
	 * @param Tag - Rule tag to check
	 * @return True if any active ruleset has this tag
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	bool IsRuleTagActive(FGameplayTag Tag) const;

	/**
	 * Get config value from active rulesets
	 * @param ConfigKey - Config key to look up
	 * @param OutValue - Output value
	 * @return True if value found
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	bool GetRuleConfigValue(FName ConfigKey, FString& OutValue) const;

	/**
	 * Get all active rule tags
	 * @return Container with all active rule tags
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	FGameplayTagContainer GetActiveRuleTags() const;

	/**
	 * Get difficulty multiplier from active rulesets
	 * @return Combined difficulty multiplier (1.0 = normal)
	 */
	UFUNCTION(BlueprintPure, Category = "Mod Ruleset")
	float GetDifficultyMultiplier() const;

	/**
	 * Apply ruleset config to game settings
	 * @param RulesetId - ID of the ruleset
	 * @return True if applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Mod Ruleset")
	bool ApplyRulesetConfig(FName RulesetId);

	// ========================================
	// Events
	// ========================================

	/** Event fired when a ruleset is activated */
	UPROPERTY(BlueprintAssignable, Category = "Mod Ruleset|Events")
	FOnRulesetActivated OnRulesetActivated;

	/** Event fired when a ruleset is deactivated */
	UPROPERTY(BlueprintAssignable, Category = "Mod Ruleset|Events")
	FOnRulesetDeactivated OnRulesetDeactivated;

protected:
	/**
	 * Validate a ruleset
	 * @param Ruleset - Ruleset to validate
	 * @return True if valid
	 */
	bool ValidateRuleset(const FHarmoniaCustomRuleset& Ruleset) const;

private:
	/** All registered rulesets (RulesetId -> Ruleset) */
	UPROPERTY()
	TMap<FName, FHarmoniaCustomRuleset> RegisteredRulesets;

	/** Active rulesets */
	UPROPERTY()
	TSet<FName> ActiveRulesetIds;

	/** Rulesets by mod (ModId -> RulesetIds) */
	UPROPERTY()
	TMap<FName, TArray<FName>> RulesetsByMod;

	/** Combined active rule tags */
	UPROPERTY()
	FGameplayTagContainer ActiveRuleTags;
};
