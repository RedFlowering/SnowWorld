// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "HarmoniaCraftingSubsystem.generated.h"

class UHarmoniaCraftingConfigDataAsset;

/**
 * Delegate for crafting events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingCompleted, const FHarmoniaCraftingSessionResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRecipeUnlocked, FName, RecipeID, APlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingSkillLevelUp, ECraftingStationType, Station, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCraftingStarted, FName, RecipeID, APlayerController*, Player, float, CraftingTime);

/**
 * Harmonia Crafting Subsystem
 *
 * Central management for crafting recipes, materials, and player crafting skills.
 * Supports queued crafting, skill progression, and recipe discovery.
 *
 * Features:
 * - Recipe management and discovery
 * - Material validation and consumption
 * - Skill-based success/quality calculation
 * - Queued crafting with progress tracking
 * - Data-driven configuration
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCraftingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Set the configuration data asset */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	void SetConfigDataAsset(UHarmoniaCraftingConfigDataAsset* InConfig);

	// ============================================================================
	// Recipe Management
	// ============================================================================

	/** Register a new recipe */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	void RegisterRecipe(const FHarmoniaCraftingRecipeData& Recipe);

	/** Unregister a recipe */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	void UnregisterRecipe(FName RecipeID);

	/** Get recipe by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	bool GetRecipe(FName RecipeID, FHarmoniaCraftingRecipeData& OutRecipe) const;

	/** Get all recipes */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	TArray<FHarmoniaCraftingRecipeData> GetAllRecipes() const;

	/** Get recipes by station type */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	TArray<FHarmoniaCraftingRecipeData> GetRecipesByStation(ECraftingStationType Station) const;

	/** Get recipes by category */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	TArray<FHarmoniaCraftingRecipeData> GetRecipesByCategory(FGameplayTag CategoryTag) const;

	// ============================================================================
	// Player Recipe Unlocks
	// ============================================================================

	/** Unlock a recipe for a player */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	bool UnlockRecipe(APlayerController* Player, FName RecipeID);

	/** Check if player has unlocked a recipe */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	bool IsRecipeUnlocked(APlayerController* Player, FName RecipeID) const;

	/** Get all unlocked recipes for a player */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	TArray<FName> GetUnlockedRecipes(APlayerController* Player) const;

	/** Get available recipes (unlocked + can craft) for a player */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	TArray<FHarmoniaCraftingRecipeData> GetAvailableRecipes(APlayerController* Player, ECraftingStationType Station) const;

	// ============================================================================
	// Crafting Operations
	// ============================================================================

	/** Check if player can craft a recipe */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	bool CanCraft(APlayerController* Player, FName RecipeID, FText& OutReason) const;

	/** Start crafting a recipe (instant or queued based on recipe) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	bool StartCrafting(APlayerController* Player, FName RecipeID, int32 Quantity = 1);

	/** Cancel an in-progress crafting */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	bool CancelCrafting(APlayerController* Player, FName RecipeID);

	/** Get crafting progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	float GetCraftingProgress(APlayerController* Player, FName RecipeID) const;

	/** Process instant craft (for recipes with 0 crafting time) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	FHarmoniaCraftingSessionResult InstantCraft(APlayerController* Player, FName RecipeID, int32 Quantity = 1);

	// ============================================================================
	// Skill Management
	// ============================================================================

	/** Get crafting skill for a player */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	FHarmoniaCraftingCategoryData GetCraftingSkill(APlayerController* Player, ECraftingStationType Station) const;

	/** Add experience to a crafting skill */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Crafting")
	void AddCraftingExperience(APlayerController* Player, ECraftingStationType Station, float Experience);

	/** Get skill level for a station */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	int32 GetSkillLevel(APlayerController* Player, ECraftingStationType Station) const;

	// ============================================================================
	// Material Queries
	// ============================================================================

	/** Check if player has required materials */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	bool HasRequiredMaterials(APlayerController* Player, FName RecipeID, int32 Quantity = 1) const;

	/** Get missing materials for a recipe */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	TArray<FCraftingMaterial> GetMissingMaterials(APlayerController* Player, FName RecipeID, int32 Quantity = 1) const;

	// ============================================================================
	// Success/Quality Calculation
	// ============================================================================

	/** Calculate success chance for a recipe */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	float CalculateSuccessChance(APlayerController* Player, FName RecipeID) const;

	/** Calculate expected quality for a recipe */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Crafting")
	float CalculateExpectedQuality(APlayerController* Player, FName RecipeID) const;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Crafting")
	FOnCraftingCompleted OnCraftingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Crafting")
	FOnRecipeUnlocked OnRecipeUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Crafting")
	FOnCraftingSkillLevelUp OnCraftingSkillLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Crafting")
	FOnCraftingStarted OnCraftingStarted;

protected:
	/** Consume materials from player inventory */
	virtual bool ConsumeMaterials(APlayerController* Player, const FHarmoniaCraftingRecipeData& Recipe, int32 Quantity);

	/** Grant crafted items to player */
	virtual bool GrantCraftedItems(APlayerController* Player, const FHarmoniaCraftingSessionResult& Result);

	/** Calculate difficulty modifier */
	float GetDifficultyModifier(EHarmoniaRecipeDifficulty Difficulty) const;

	/** Process level up if enough experience */
	void ProcessLevelUp(APlayerController* Player, ECraftingStationType Station);

private:
	/** All registered recipes */
	UPROPERTY()
	TMap<FName, FHarmoniaCraftingRecipeData> Recipes;

	/** Player unlocked recipes (PlayerID -> RecipeIDs) */
	TMap<uint32, TSet<FName>> PlayerUnlockedRecipes;

	/** Player crafting skills (PlayerID -> Station -> Skill) */
	TMap<uint32, TMap<ECraftingStationType, FHarmoniaCraftingCategoryData>> PlayerSkills;

	/** Active crafting sessions (PlayerID -> RecipeID -> Progress) */
	struct FCraftingSession
	{
		FName RecipeID;
		int32 Quantity;
		float TotalTime;
		float ElapsedTime;
		FTimerHandle TimerHandle;
	};
	TMap<uint32, TMap<FName, FCraftingSession>> ActiveSessions;

	/** Config data asset */
	UPROPERTY()
	TObjectPtr<UHarmoniaCraftingConfigDataAsset> ConfigAsset;

	/** Get unique player ID */
	uint32 GetPlayerID(APlayerController* Player) const;
};
