// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "Engine/DataTable.h"
#include "HarmoniaCraftingComponent.generated.h"

// Forward declarations
class UHarmoniaInventoryComponent;
class UDataTable;
class UAnimMontage;

/**
 * Delegate for crafting events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingStarted, FHarmoniaID, RecipeId, float, CastingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCraftingProgress, FHarmoniaID, RecipeId, float, Progress, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCraftingCompleted, FHarmoniaID, RecipeId, ECraftingResult, Result, const TArray<FCraftingResultItem>&, ResultItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingCancelled, FHarmoniaID, RecipeId, float, ProgressLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRecipeLearned, FHarmoniaID, RecipeId, FText, RecipeName);

/**
 * HarmoniaCraftingComponent
 *
 * Manages crafting system with:
 * - Recipe management and learning
 * - Material requirement checking
 * - Casting time with animation support
 * - Success/failure probability system
 * - Integration with inventory system
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaCraftingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	//~==============================================
	//~ Recipe Data Tables
	//~==============================================
public:
	/** Recipe data table reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
	UDataTable* RecipeDataTable;

	/** Grade configuration data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
	UDataTable* GradeConfigDataTable;

	/** Category configuration data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
	UDataTable* CategoryDataTable;

	/** Station configuration data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting|Config")
	UDataTable* StationDataTable;

	//~==============================================
	//~ Crafting State
	//~==============================================
protected:
	/** Current active crafting session */
	UPROPERTY(ReplicatedUsing = OnRep_ActiveSession, BlueprintReadOnly, Category = "Crafting")
	FActiveCraftingSession ActiveSession;

	/** Learned recipes (for recipes that require learning) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crafting")
	TArray<FHarmoniaID> LearnedRecipes;

	/** Current crafting station type (None = hand crafting) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStation, BlueprintReadOnly, Category = "Crafting")
	ECraftingStationType CurrentStation;

	/** Current station tags (for custom stations) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Crafting")
	FGameplayTagContainer CurrentStationTags;

	/** Cached inventory component reference */
	UPROPERTY()
	UHarmoniaInventoryComponent* InventoryComponent;

	UFUNCTION()
	void OnRep_ActiveSession();

	UFUNCTION()
	void OnRep_CurrentStation();

	//~==============================================
	//~ Crafting Operations
	//~==============================================
public:
	/**
	 * Start crafting a recipe
	 * @param RecipeId - Recipe to craft
	 * @return True if crafting started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool StartCrafting(FHarmoniaID RecipeId);

	/**
	 * Cancel current crafting session
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CancelCrafting();

	/**
	 * Check if player can craft a recipe (has materials, meets requirements, etc.)
	 * @param RecipeId - Recipe to check
	 * @param OutMissingMaterials - Materials that are missing (if any)
	 * @return True if player can craft this recipe
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool CanCraftRecipe(FHarmoniaID RecipeId, TArray<FCraftingMaterial>& OutMissingMaterials) const;

	/**
	 * Get recipe data by ID
	 * @param RecipeId - Recipe ID to lookup
	 * @param OutRecipeData - Output recipe data
	 * @return True if recipe was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool GetRecipeData(FHarmoniaID RecipeId, FCraftingRecipeData& OutRecipeData) const;

	/**
	 * Get all available recipes (optionally filtered by category)
	 * @param CategoryTag - Category to filter by (optional)
	 * @return Array of recipe data
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FCraftingRecipeData> GetAvailableRecipes(FGameplayTag CategoryTag = FGameplayTag()) const;

	/**
	 * Get current crafting progress (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	float GetCraftingProgress() const { return ActiveSession.GetProgress(); }

	/**
	 * Check if currently crafting
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	bool IsCrafting() const { return ActiveSession.bIsActive; }

	/**
	 * Get active crafting session
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	FActiveCraftingSession GetActiveCraftingSession() const { return ActiveSession; }

	//~==============================================
	//~ Crafting Station System
	//~==============================================
public:
	/**
	 * Set current crafting station
	 * @param StationType - Type of station to use
	 * @param StationTags - Additional tags for custom stations
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Station")
	void SetCurrentStation(ECraftingStationType StationType, FGameplayTagContainer StationTags = FGameplayTagContainer());

	/**
	 * Clear current crafting station (return to hand crafting)
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Station")
	void ClearCurrentStation();

	/**
	 * Get current crafting station type
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting|Station")
	ECraftingStationType GetCurrentStation() const { return CurrentStation; }

	/**
	 * Check if currently at a crafting station
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting|Station")
	bool IsAtCraftingStation() const { return CurrentStation != ECraftingStationType::None; }

	/**
	 * Get station data
	 * @param StationType - Station type to lookup
	 * @param OutStationData - Output station data
	 * @return True if station data was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Station")
	bool GetStationData(ECraftingStationType StationType, FCraftingStationData& OutStationData) const;

	/**
	 * Check if current station meets recipe requirements
	 * @param RecipeData - Recipe to check
	 * @return True if station requirements are met
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Station")
	bool CheckStationRequirement(const FCraftingRecipeData& RecipeData) const;

	/**
	 * Get all recipes available at current station
	 * @param CategoryTag - Optional category filter
	 * @return Array of recipe data
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Station")
	TArray<FCraftingRecipeData> GetRecipesForCurrentStation(FGameplayTag CategoryTag = FGameplayTag()) const;

	//~==============================================
	//~ Recipe Learning System
	//~==============================================
public:
	/**
	 * Learn a new recipe
	 * @param RecipeId - Recipe to learn
	 * @return True if recipe was learned (false if already known)
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Learning")
	bool LearnRecipe(FHarmoniaID RecipeId);

	/**
	 * Check if player knows a recipe
	 * @param RecipeId - Recipe to check
	 * @return True if recipe is learned
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting|Learning")
	bool HasLearnedRecipe(FHarmoniaID RecipeId) const;

	/**
	 * Get all learned recipes
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting|Learning")
	TArray<FHarmoniaID> GetLearnedRecipes() const { return LearnedRecipes; }

	/**
	 * Forget a recipe (for testing or game mechanics)
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Learning")
	void ForgetRecipe(FHarmoniaID RecipeId);

	//~==============================================
	//~ Grade System
	//~==============================================
public:
	/**
	 * Get grade configuration
	 * @param Grade - Grade to lookup
	 * @param OutGradeConfig - Output grade config
	 * @return True if grade config was found
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Grade")
	bool GetGradeConfig(EItemGrade Grade, FItemGradeConfig& OutGradeConfig) const;

	/**
	 * Get grade color for UI
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting|Grade")
	FLinearColor GetGradeColor(EItemGrade Grade) const;

	/**
	 * Get grade display name
	 */
	UFUNCTION(BlueprintPure, Category = "Crafting|Grade")
	FText GetGradeDisplayName(EItemGrade Grade) const;

	//~==============================================
	//~ Internal Crafting Logic
	//~==============================================
protected:
	/**
	 * Complete the current crafting session
	 * Determines success/failure and distributes rewards
	 */
	void CompleteCrafting();

	/**
	 * Consume materials for crafting
	 * @param Materials - Materials to consume
	 * @return True if materials were successfully consumed
	 */
	bool ConsumeMaterials(const TArray<FCraftingMaterial>& Materials);

	/**
	 * Determine crafting result based on probabilities
	 * @param Recipe - Recipe being crafted
	 * @return Crafting result (Success, Failure, CriticalSuccess)
	 */
	ECraftingResult DetermineCraftingResult(const FCraftingRecipeData& Recipe) const;

	/**
	 * Distribute crafting results to inventory
	 * @param ResultItems - Items to distribute
	 */
	void DistributeResults(const TArray<FCraftingResultItem>& ResultItems);

	/**
	 * Play crafting animation montage
	 * @param Montage - Montage to play
	 */
	void PlayCraftingAnimation(UAnimMontage* Montage);

	/**
	 * Stop crafting animation
	 */
	void StopCraftingAnimation();

	/**
	 * Check if player meets recipe requirements (level, skill, etc.)
	 * @param Recipe - Recipe to check
	 * @return True if player meets requirements
	 */
	bool MeetsRecipeRequirements(const FCraftingRecipeData& Recipe) const;

	//~==============================================
	//~ Server/Client RPCs
	//~==============================================
protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartCrafting(FHarmoniaID RecipeId);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCancelCrafting();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLearnRecipe(FHarmoniaID RecipeId);

	UFUNCTION(Client, Reliable)
	void ClientCraftingCompleted(FHarmoniaID RecipeId, ECraftingResult Result, const TArray<FCraftingResultItem>& ResultItems);

	UFUNCTION(Client, Reliable)
	void ClientCraftingCancelled(FHarmoniaID RecipeId, float ProgressLost);

	//~==============================================
	//~ Delegates
	//~==============================================
public:
	/** Called when crafting starts */
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftingStarted OnCraftingStarted;

	/** Called during crafting progress (can be used for UI updates) */
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftingProgress OnCraftingProgress;

	/** Called when crafting completes (success or failure) */
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftingCompleted OnCraftingCompleted;

	/** Called when crafting is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnCraftingCancelled OnCraftingCancelled;

	/** Called when a recipe is learned */
	UPROPERTY(BlueprintAssignable, Category = "Crafting|Events")
	FOnRecipeLearned OnRecipeLearned;

	//~==============================================
	//~ Save/Load System
	//~==============================================
public:
	/**
	 * Get learned recipes save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Save")
	FLearnedRecipeSaveData GetLearnedRecipesSaveData() const;

	/**
	 * Load learned recipes from save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Save")
	void LoadLearnedRecipesFromSaveData(const FLearnedRecipeSaveData& SaveData);

	//~==============================================
	//~ Utility Functions
	//~==============================================
protected:
	/**
	 * Get inventory component (cached)
	 */
	UHarmoniaInventoryComponent* GetInventoryComponent();

	/**
	 * Get character level (override in subclass if you have level system)
	 */
	virtual int32 GetCharacterLevel() const;

	/**
	 * Get crafting skill level (override in subclass if you have skill system)
	 */
	virtual int32 GetCraftingSkillLevel() const;

	//~==============================================
	//~ Debug
	//~==============================================
public:
#if WITH_EDITOR
	/**
	 * Debug: Learn all recipes
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Debug")
	void Debug_LearnAllRecipes();

	/**
	 * Debug: Complete current crafting instantly
	 */
	UFUNCTION(BlueprintCallable, Category = "Crafting|Debug")
	void Debug_InstantCraft();
#endif
};
