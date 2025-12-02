// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaCookingSystemDefinitions.generated.h"

/**
 * ?Œì‹ ?€??
 */
UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Meal		UMETA(DisplayName = "Meal"),		// ?ì‚¬
	Soup		UMETA(DisplayName = "Soup"),		// ?˜í”„
	Dessert		UMETA(DisplayName = "Dessert"),		// ?”ì???
	Beverage	UMETA(DisplayName = "Beverage"),	// ?Œë£Œ
	Snack		UMETA(DisplayName = "Snack"),		// ê°„ì‹
	Elixir		UMETA(DisplayName = "Elixir"),		// ?˜ë¦­??
	Salad		UMETA(DisplayName = "Salad")		// ?ëŸ¬??
};

/**
 * ?”ë¦¬ ?ˆì§ˆ
 */
UENUM(BlueprintType)
enum class ECookingQuality : uint8
{
	Failed		UMETA(DisplayName = "Failed"),		// ?¤íŒ¨??
	Poor		UMETA(DisplayName = "Poor"),		// ì¡°ì•…??
	Normal		UMETA(DisplayName = "Normal"),		// ë³´í†µ
	Good		UMETA(DisplayName = "Good"),		// ì¢‹ìŒ
	Excellent	UMETA(DisplayName = "Excellent"),	// ?Œë???
	Masterpiece	UMETA(DisplayName = "Masterpiece")	// ê±¸ì‘
};

/**
 * ?”ë¦¬ ë°©ë²•
 */
UENUM(BlueprintType)
enum class ECookingMethod : uint8
{
	Boiling		UMETA(DisplayName = "Boiling"),		// ?“ì´ê¸?
	Frying		UMETA(DisplayName = "Frying"),		// êµ½ê¸°
	Grilling	UMETA(DisplayName = "Grilling"),	// ê·¸ë¦´
	Steaming	UMETA(DisplayName = "Steaming"),	// ì°Œê¸°
	Baking		UMETA(DisplayName = "Baking"),		// ë² ì´??
	Mixing		UMETA(DisplayName = "Mixing"),		// ?ê¸°
	Roasting	UMETA(DisplayName = "Roasting")		// ë¡œìŠ¤??
};

/**
 * ?Œì‹ ë²„í”„ ?¨ê³¼
 */
USTRUCT(BlueprintType)
struct FFoodBuffEffect
{
	GENERATED_BODY()

	/** ë²„í”„ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** ë²„í”„ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** ì²´ë ¥ ?Œë³µ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRestore = 0.0f;

	/** ë§ˆë‚˜ ?Œë³µ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRestore = 0.0f;

	/** ?¤íƒœë¯¸ë‚˜ ?Œë³µ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRestore = 0.0f;

	/** ê³µê²©??ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** ë°©ì–´??ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** ?´ë™?ë„ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** ?¬ë¦¬?°ì»¬ ?•ë¥  ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** ê²½í—˜ì¹??ë“??ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** ë²„í”„ ì§€?ì‹œê°?(ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 300.0f;

	/** ?¤íƒ ê°€???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bStackable = false;

	/** ìµœë? ?¤íƒ ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	int32 MaxStacks = 1;
};

/**
 * ?”ë¦¬ ?¬ë£Œ
 */
USTRUCT(BlueprintType)
struct FCookingIngredient
{
	GENERATED_BODY()

	/** ?¬ë£Œ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	FName IngredientID;

	/** ?„ìš” ?˜ëŸ‰ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	int32 Quantity = 1;

	/** ? íƒ???¬ë£Œ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	bool bOptional = false;
};

/**
 * ?”ë¦¬ ?ˆì‹œ??
 */
USTRUCT(BlueprintType)
struct FCookingRecipe
{
	GENERATED_BODY()

	/** ?ˆì‹œ??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** ?ˆì‹œ???´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText RecipeName;

	/** ?ˆì‹œ???¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** ?Œì‹ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** ?”ë¦¬ ë°©ë²• */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** ?„ìš” ?¬ë£Œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FCookingIngredient> RequiredIngredients;

	/** ?”ë¦¬ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float CookingTime = 10.0f;

	/** ìµœì†Œ ?”ë¦¬ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 MinCookingLevel = 1;

	/** ?œì´??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 Difficulty = 5;

	/** ê¸°ë³¸ ë²„í”„ ?¨ê³¼ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FFoodBuffEffect BaseBuffEffect;

	/** ?ˆì§ˆë³?ë²„í”„ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TMap<ECookingQuality, float> QualityMultipliers;

	/** ?ˆì‹œ???„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?„ì„±???Œì‹ ë©”ì‹œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UStaticMesh> FoodMesh;

	/** ê²½í—˜ì¹?ë³´ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 ExperienceReward = 20;

	/** ?¨ê²¨ì§??ˆì‹œ???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bHidden = false;
};

/**
 * ?”ë¦¬ ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FCookingResult
{
	GENERATED_BODY()

	/** ?ˆì‹œ??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName RecipeID;

	/** ?”ë¦¬ ?ˆì§ˆ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	ECookingQuality Quality = ECookingQuality::Normal;

	/** ?ìš©??ë²„í”„ ?¨ê³¼ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FFoodBuffEffect BuffEffect;

	/** ?ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ?±ê³µ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess = true;

	/** ?„ë²½???”ë¦¬ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * ?”ë¦¬ ?¹ì„±
 */
USTRUCT(BlueprintType)
struct FCookingTrait
{
	GENERATED_BODY()

	/** ?¹ì„± ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** ?¹ì„± ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** ?”ë¦¬ ?ë„ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CookingSpeedBonus = 0.0f;

	/** ?±ê³µ?•ë¥  ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float SuccessRateBonus = 0.0f;

	/** ?ˆì§ˆ ?¥ìƒ ?•ë¥  (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** ë²„í”„ ?¨ê³¼ ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** ë²„í”„ ì§€?ì‹œê°?ì¦ê? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** ?¬ë£Œ ?ˆì•½ ?•ë¥  (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float IngredientSaveChance = 0.0f;

	/** ê²½í—˜ì¹?ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;
};

/**
 * ?”ë¦¬ ?ˆì‹œ???°ì´???Œì´ë¸?Row
 * ?ë””?°ì—??DataTableë¡??ˆì‹œ?¼ë? ê´€ë¦¬í•  ???¬ìš©
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FRecipeDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?ˆì‹œ??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** ?œì‹œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText DisplayName;

	/** ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** ?Œì‹ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** ?”ë¦¬ ë°©ë²• */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** ?„ìš” ?¬ë£Œ (?„ì´??ID -> ?˜ëŸ‰) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Ingredients")
	TMap<FName, int32> Ingredients;

	/** ?”ë¦¬ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Cooking")
	float CookingTime = 5.0f;

	/** ìµœì†Œ ?”ë¦¬ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Requirement")
	int32 MinCookingLevel = 1;

	/** ?ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Economy")
	int32 ExperienceGain = 10;

	/** ?œì´??(1-10, ?ˆì§ˆ ?•ë¥ ???í–¥) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Cooking")
	int32 Difficulty = 1;

	/** ê¸°ë³¸ ë²„í”„ ?¨ê³¼ (?ˆì§ˆ???°ë¼ ë°°ìœ¨ ?ìš©) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Buff")
	FFoodBuffEffect BuffEffect;

	/** ë²„í”„ ì§€?ì‹œê°?(ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Buff")
	float BuffDuration = 300.0f;

	/** ?¨ê²¨ì§??ˆì‹œ???¬ë? (?¤í—˜???µí•´ ë°œê²¬) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bIsHidden = false;

	/** ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ê¸°ë³¸ ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Economy")
	int32 BasePrice = 20;
};
