// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaCookingComponent.h
 * @brief Cooking system component for food preparation and buff management
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "Definitions/HarmoniaCookingSystemDefinitions.h"
#include "HarmoniaCookingComponent.generated.h"

class UHarmoniaInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCookingStarted, FName, RecipeID, float, CookingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCookingCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCookingCompleted, const FCookingResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCookingLevelUp, int32, NewLevel, int32, SkillPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeDiscovered, FName, RecipeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFoodConsumed, FName, FoodID, const FFoodBuffEffect&, BuffEffect);

/**
 * @class UHarmoniaCookingComponent
 * @brief Cooking system component for food creation and buff management
 * 
 * Handles food preparation, recipe discovery, buff application and management.
 * Provides experience-based skill progression and cooking trait system.
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaCookingComponent : public UHarmoniaBaseLifeContentComponent
{
	GENERATED_BODY()

public:	
	UHarmoniaCookingComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// Cooking Basic Functions
	// ====================================

	/** Start cooking a recipe */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool StartCooking(FName RecipeID);

	/** Cancel current cooking process */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void CancelCooking();

	/** Check if currently cooking */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsCooking() const { return bIsCooking; }

	/** Get current cooking progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetCookingProgress() const;

	/** Check if recipe can be cooked now */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool CanCookRecipe(FName RecipeID) const;

	// ====================================
	// Food Consumption
	// ====================================

	/** Consume food item and apply effects */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool ConsumeFood(FName FoodID, ECookingQuality Quality);

	/** Get currently active food buffs */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FFoodBuffEffect> GetActiveBuffs() const { return ActiveBuffs; }

	/** Check if specific buff is active */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool HasBuff(FName BuffName) const;

	/** Remove specific buff */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveBuff(FName BuffName);

	// ====================================
	// Recipe Management
	// ====================================

	/** Discover a new recipe */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void DiscoverRecipe(FName RecipeID);

	/** Check if recipe is known */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsRecipeKnown(FName RecipeID) const;

	/** Get all known recipes */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FName> GetKnownRecipes() const { return KnownRecipes; }

	/** Get recipe data by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool GetRecipeData(FName RecipeID, FCookingRecipe& OutRecipe) const;

	// ====================================
	// Level & Experience System
	// ====================================

	/** Add cooking experience */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddCookingExperience(int32 Amount);

	/** Get current cooking level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	int32 GetCookingLevel() const { return CookingLevel; }

	// ====================================
	// Trait System
	// ====================================

	/** Add cooking trait */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddTrait(const FCookingTrait& Trait);

	/** Remove cooking trait */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveTrait(FName TraitName);

	/** Get all active traits */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FCookingTrait> GetAllTraits() const { return ActiveTraits; }

	/** Get total cooking speed bonus from traits */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalCookingSpeedBonus() const;

	/** Get total success rate bonus from traits */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalSuccessRateBonus() const;

	/** Get total quality bonus from traits */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalQualityBonus() const;

	// ====================================
	// Events
	// ====================================

	/** Event fired when cooking starts */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingStarted OnCookingStarted;

	/** Event fired when cooking is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCancelled OnCookingCancelled;

	/** Event fired when cooking completes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCompleted OnCookingCompleted;

	/** Event fired on level up */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingLevelUp OnCookingLevelUp;

	/** Event fired when recipe is discovered */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnRecipeDiscovered OnRecipeDiscovered;

	/** Event fired when food is consumed */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnFoodConsumed OnFoodConsumed;

	// ====================================
	// Settings
	// ====================================

	/** Recipe database */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TMap<FName, FCookingRecipe> RecipeDatabase;

	/** Base success rate (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float BaseSuccessRate = 70.0f;

private:
	/** Reference to inventory component */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** Cooking status flag */
	UPROPERTY()
	bool bIsCooking = false;

	/** Currently cooking recipe ID */
	UPROPERTY()
	FName CurrentRecipeID;

	/** Cooking start time */
	UPROPERTY()
	float CookingStartTime = 0.0f;

	/** Required time to complete cooking */
	UPROPERTY()
	float RequiredCookingTime = 0.0f;

	/** Current cooking level */
	UPROPERTY()
	int32 CookingLevel = 1;

	/** List of known recipes */
	UPROPERTY()
	TArray<FName> KnownRecipes;

	/** List of active food buffs */
	UPROPERTY()
	TArray<FFoodBuffEffect> ActiveBuffs;

	/** Timer handles for buff expiration */
	UPROPERTY()
	TMap<FName, FTimerHandle> BuffTimerHandles;

	/** List of active cooking traits */
	UPROPERTY()
	TArray<FCookingTrait> ActiveTraits;

	/** Process cooking completion */
	void CompleteCooking();

	/** Calculate cooking result */
	FCookingResult CalculateCookingResult(const FCookingRecipe& Recipe);

	/** Determine cooking quality */
	ECookingQuality DetermineCookingQuality(int32 Difficulty);

	/** Apply buff effect to character */
	void ApplyBuffEffect(const FFoodBuffEffect& BuffEffect);

	/** Handle buff expiration */
	void OnBuffExpired(FName BuffName);

	/** Check and process level up */
	void CheckAndProcessLevelUp();

	/** Calculate cooking time with bonuses */
	float CalculateCookingTime(float BaseTime) const;

	/** Check if required ingredients are available */
	bool HasRequiredIngredients(const FCookingRecipe& Recipe) const;

	/** Consume required ingredients */
	bool ConsumeIngredients(const FCookingRecipe& Recipe);
};
