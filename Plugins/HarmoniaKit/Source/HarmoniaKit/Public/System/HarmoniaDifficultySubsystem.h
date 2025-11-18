// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Definitions/HarmoniaDifficultySystemDefinitions.h"
#include "HarmoniaDifficultySubsystem.generated.h"

class AController;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDifficultyPactToggled, EHarmoniaDifficultyPactType, PactType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewGamePlusTierChanged, EHarmoniaNewGamePlusTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDynamicDifficultyAdjusted, float, NewAdjustment);

/**
 * Harmonia Difficulty Subsystem
 * Manages the "Pact of Fate" difficulty system with:
 * - Individual difficulty pacts/modifiers
 * - New Game Plus progression
 * - Dynamic difficulty adjustment
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDifficultySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UHarmoniaDifficultySubsystem();

	// UWorldSubsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========================================
	// Difficulty Pacts
	// ========================================

	/**
	 * Initialize default pact configurations
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty")
	void InitializeDefaultPacts();

	/**
	 * Toggle a specific difficulty pact on/off
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty")
	bool TogglePact(EHarmoniaDifficultyPactType PactType, bool bActivate);

	/**
	 * Check if a pact is currently active
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty")
	bool IsPactActive(EHarmoniaDifficultyPactType PactType) const;

	/**
	 * Get configuration for a specific pact
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty")
	FHarmoniaDifficultyPactConfig GetPactConfig(EHarmoniaDifficultyPactType PactType) const;

	/**
	 * Get all active pacts
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty")
	TArray<FHarmoniaDifficultyPactConfig> GetActivePacts() const;

	/**
	 * Get all available pact configurations
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty")
	TArray<FHarmoniaDifficultyPactConfig> GetAllPactConfigs() const;

	/**
	 * Calculate total difficulty multiplier from all active pacts
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty")
	float GetTotalDifficultyMultiplier() const;

	// ========================================
	// Modifier Getters (for gameplay systems)
	// ========================================

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetPlayerDamageMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetPlayerDamageTakenMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetPlayerPoiseMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	bool IsBlockingDisabled() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	bool IsParryingDisabled() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetEnemyHealthMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetEnemyDamageMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetEnemyMovementSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetEnemyAttackSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetEliteSpawnChanceMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	bool IsEnhancedAIEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetHealingEffectivenessMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetStaminaDrainMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetDurabilityLossMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetVendorPriceMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetSoulGainMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetExperienceGainMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetItemDropRateMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetRareDropRateMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	float GetLegendaryDropRateMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	int32 GetMaxCheckpointUses() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	bool IsPermadeathEnabled() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	bool ShouldHideHUD() const;

	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Modifiers")
	bool IsMinimapLimited() const;

	// ========================================
	// New Game Plus
	// ========================================

	/**
	 * Start a new game plus playthrough
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|NewGamePlus")
	void StartNewGamePlus();

	/**
	 * Complete the current playthrough (awards legacy bonuses)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|NewGamePlus")
	void CompletePlaythrough(float CompletionTime);

	/**
	 * Get current New Game Plus tier
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|NewGamePlus")
	EHarmoniaNewGamePlusTier GetNewGamePlusTier() const;

	/**
	 * Get New Game Plus legacy data
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|NewGamePlus")
	FHarmoniaNewGamePlusLegacy GetNewGamePlusLegacy() const;

	/**
	 * Get difficulty multiplier based on NG+ tier
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|NewGamePlus")
	float GetNewGamePlusDifficultyMultiplier() const;

	/**
	 * Award a legacy unlock (cosmetic, weapon, ability)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|NewGamePlus")
	void AwardLegacyUnlock(const FName& UnlockName, const FString& UnlockType);

	// ========================================
	// Dynamic Difficulty
	// ========================================

	/**
	 * Set dynamic difficulty mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic")
	void SetDynamicDifficultyMode(EHarmoniaDynamicDifficultyMode Mode);

	/**
	 * Get current dynamic difficulty mode
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Dynamic")
	EHarmoniaDynamicDifficultyMode GetDynamicDifficultyMode() const;

	/**
	 * Get current dynamic difficulty adjustment
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty|Dynamic")
	float GetDynamicDifficultyAdjustment() const;

	/**
	 * Report player death (for dynamic difficulty tracking)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic")
	void ReportPlayerDeath();

	/**
	 * Report player victory (for dynamic difficulty tracking)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic")
	void ReportPlayerVictory(float HealthRemainingPercent, float CombatDuration);

	/**
	 * Report perfect defensive action (parry/dodge)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic")
	void ReportPerfectDefense(bool bWasParry);

	/**
	 * Update dynamic difficulty based on performance
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty|Dynamic")
	void UpdateDynamicDifficulty();

	// ========================================
	// Configuration Management
	// ========================================

	/**
	 * Get complete difficulty configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Difficulty")
	FHarmoniaDifficultyConfiguration GetDifficultyConfiguration() const;

	/**
	 * Set difficulty configuration (used for loading saves)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty")
	void SetDifficultyConfiguration(const FHarmoniaDifficultyConfiguration& Config);

	/**
	 * Reset difficulty to default
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Difficulty")
	void ResetToDefault();

	// ========================================
	// Events
	// ========================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Difficulty|Events")
	FOnDifficultyPactToggled OnPactToggled;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Difficulty|Events")
	FOnNewGamePlusTierChanged OnNewGamePlusTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Difficulty|Events")
	FOnDynamicDifficultyAdjusted OnDynamicDifficultyAdjusted;

protected:
	// ========================================
	// Internal Data
	// ========================================

	UPROPERTY()
	FHarmoniaDifficultyConfiguration CurrentConfiguration;

	UPROPERTY()
	TMap<EHarmoniaDifficultyPactType, FHarmoniaDifficultyPactConfig> PactConfigs;

	// Dynamic difficulty tracking
	UPROPERTY()
	FTimerHandle DynamicDifficultyUpdateTimer;

	// Helper Functions
	void CreateDefaultPactConfig(EHarmoniaDifficultyPactType PactType, const FText& Name, const FText& Description, float DifficultyRating);
	void SetupGlassCannonPact();
	void SetupIronWillPact();
	void SetupFragileBonesPact();
	void SetupBerserkersRagePact();
	void SetupSwiftFoesPact();
	void SetupEliteUprisingPact();
	void SetupIntelligentEnemiesPact();
	void SetupRelentlessPursuitPact();
	void SetupScarceRecoveryPact();
	void SetupFragileEquipmentPact();
	void SetupHungerOfTheVoidPact();
	void SetupPovertysCursePact();
	void SetupGamblersFatePact();
	void SetupPermadeathThreatPact();
	void SetupNoHUDPact();
	void SetupFogOfWarPact();

	float CalculateCombinedMultiplier(TFunction<float(const FHarmoniaDifficultyPactConfig&)> MultiplierGetter) const;
	bool CheckAnyPactCondition(TFunction<bool(const FHarmoniaDifficultyPactConfig&)> ConditionGetter) const;
};
