// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaCookingSystemDefinitions.h
 * @brief Cooking system type definitions
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaCookingSystemDefinitions.generated.h"

/**
 * @enum EFoodType
 * @brief Food type
 */
UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Meal		UMETA(DisplayName = "Meal"),		// Meal
	Soup		UMETA(DisplayName = "Soup"),		// Soup
	Dessert		UMETA(DisplayName = "Dessert"),		// Dessert
	Beverage	UMETA(DisplayName = "Beverage"),	// Beverage
	Snack		UMETA(DisplayName = "Snack"),		// Snack
	Elixir		UMETA(DisplayName = "Elixir"),		// Elixir
	Salad		UMETA(DisplayName = "Salad")		// Salad
};

/**
 * @enum ECookingQuality
 * @brief Cooking quality
 */
UENUM(BlueprintType)
enum class ECookingQuality : uint8
{
	Failed		UMETA(DisplayName = "Failed"),		// Failed
	Poor		UMETA(DisplayName = "Poor"),		// Poor
	Normal		UMETA(DisplayName = "Normal"),		// Normal
	Good		UMETA(DisplayName = "Good"),		// Good
	Excellent	UMETA(DisplayName = "Excellent"),	// Excellent
	Masterpiece	UMETA(DisplayName = "Masterpiece")	// Masterpiece
};

/**
 * @enum ECookingMethod
 * @brief Cooking method
 */
UENUM(BlueprintType)
enum class ECookingMethod : uint8
{
	Boiling		UMETA(DisplayName = "Boiling"),		// Boiling
	Frying		UMETA(DisplayName = "Frying"),		// Frying
	Grilling	UMETA(DisplayName = "Grilling"),	// Grilling
	Steaming	UMETA(DisplayName = "Steaming"),	// Steaming
	Baking		UMETA(DisplayName = "Baking"),		// Baking
	Mixing		UMETA(DisplayName = "Mixing"),		// Mixing
	Roasting	UMETA(DisplayName = "Roasting")		// Roasting
};

/**
 * @struct FFoodBuffEffect
 * @brief Food buff effect
 */
USTRUCT(BlueprintType)
struct FFoodBuffEffect
{
	GENERATED_BODY()

	/** Buff name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** Buff description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** Health restore amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRestore = 0.0f;

	/** Mana restore amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRestore = 0.0f;

	/** Stamina restore amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRestore = 0.0f;

	/** Attack bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** Defense bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** Movement speed bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** Critical chance bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** Experience gain bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** Buff duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 300.0f;

	/** Is stackable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bStackable = false;

	/** Max stacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	int32 MaxStacks = 1;
};

/**
 * @struct FCookingIngredient
 * @brief Cooking ingredient
 */
USTRUCT(BlueprintType)
struct FCookingIngredient
{
	GENERATED_BODY()

	/** Ingredient ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	FName IngredientID;

	/** Required quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	int32 Quantity = 1;

	/** Is optional ingredient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	bool bOptional = false;
};

/**
 * @struct FCookingRecipe
 * @brief Cooking recipe
 */
USTRUCT(BlueprintType)
struct FCookingRecipe
{
	GENERATED_BODY()

	/** Recipe ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** Recipe name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText RecipeName;

	/** Recipe description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** Food type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** Cooking method */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** Required ingredients */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FCookingIngredient> RequiredIngredients;

	/** Cooking time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float CookingTime = 10.0f;

	/** Minimum cooking level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 MinCookingLevel = 1;

	/** Difficulty (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 Difficulty = 5;

	/** Base buff effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FFoodBuffEffect BaseBuffEffect;

	/** Quality multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TMap<ECookingQuality, float> QualityMultipliers;

	/** Recipe icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Created food mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UStaticMesh> FoodMesh;

	/** Experience reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 ExperienceReward = 20;

	/** Is hidden recipe */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bHidden = false;
};

/**
 * @struct FCookingResult
 * @brief Cooking result
 */
USTRUCT(BlueprintType)
struct FCookingResult
{
	GENERATED_BODY()

	/** Recipe ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName RecipeID;

	/** Cooking quality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	ECookingQuality Quality = ECookingQuality::Normal;

	/** Applied buff effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FFoodBuffEffect BuffEffect;

	/** Gained experience */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** Success flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess = true;

	/** Perfect cooking flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * @struct FCookingTrait
 * @brief Cooking trait
 */
USTRUCT(BlueprintType)
struct FCookingTrait
{
	GENERATED_BODY()

	/** Trait name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** Trait description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** Cooking speed bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CookingSpeedBonus = 0.0f;

	/** Success rate bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float SuccessRateBonus = 0.0f;

	/** Quality upgrade chance (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** Buff effect bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** Buff duration bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** Ingredient save chance (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float IngredientSaveChance = 0.0f;

	/** Experience bonus (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;
};

/**
 * @struct FRecipeDataTableRow
 * @brief Cooking recipe DataTable Row
 * 
 * Used when managing recipes as DataTable in editor.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FRecipeDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Recipe ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** Food type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** Cooking method */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** Required ingredients (Item ID -> Quantity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Ingredients")
	TMap<FName, int32> Ingredients;

	/** Cooking time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Cooking")
	float CookingTime = 5.0f;

	/** Minimum cooking level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Requirement")
	int32 MinCookingLevel = 1;

	/** Gained experience */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Economy")
	int32 ExperienceGain = 10;

	/** Difficulty (1-10, affects quality chance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Cooking")
	int32 Difficulty = 1;

	/** Base buff effect (quality multiplier applied) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Buff")
	FFoodBuffEffect BuffEffect;

	/** Buff duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Buff")
	float BuffDuration = 300.0f;

	/** Is hidden recipe (discovered through experimentation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bIsHidden = false;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Base sell price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Economy")
	int32 BasePrice = 20;
};
