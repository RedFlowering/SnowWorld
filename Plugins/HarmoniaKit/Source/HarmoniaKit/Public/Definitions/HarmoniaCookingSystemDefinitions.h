// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaCookingSystemDefinitions.generated.h"

/**
 * ?�식 ?�??
 */
UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Meal		UMETA(DisplayName = "Meal"),		// ?�사
	Soup		UMETA(DisplayName = "Soup"),		// ?�프
	Dessert		UMETA(DisplayName = "Dessert"),		// ?��???
	Beverage	UMETA(DisplayName = "Beverage"),	// ?�료
	Snack		UMETA(DisplayName = "Snack"),		// 간식
	Elixir		UMETA(DisplayName = "Elixir"),		// ?�릭??
	Salad		UMETA(DisplayName = "Salad")		// ?�러??
};

/**
 * ?�리 ?�질
 */
UENUM(BlueprintType)
enum class ECookingQuality : uint8
{
	Failed		UMETA(DisplayName = "Failed"),		// ?�패??
	Poor		UMETA(DisplayName = "Poor"),		// 조악??
	Normal		UMETA(DisplayName = "Normal"),		// 보통
	Good		UMETA(DisplayName = "Good"),		// 좋음
	Excellent	UMETA(DisplayName = "Excellent"),	// ?��???
	Masterpiece	UMETA(DisplayName = "Masterpiece")	// 걸작
};

/**
 * ?�리 방법
 */
UENUM(BlueprintType)
enum class ECookingMethod : uint8
{
	Boiling		UMETA(DisplayName = "Boiling"),		// ?�이�?
	Frying		UMETA(DisplayName = "Frying"),		// 굽기
	Grilling	UMETA(DisplayName = "Grilling"),	// 그릴
	Steaming	UMETA(DisplayName = "Steaming"),	// 찌기
	Baking		UMETA(DisplayName = "Baking"),		// 베이??
	Mixing		UMETA(DisplayName = "Mixing"),		// ?�기
	Roasting	UMETA(DisplayName = "Roasting")		// 로스??
};

/**
 * ?�식 버프 ?�과
 */
USTRUCT(BlueprintType)
struct FFoodBuffEffect
{
	GENERATED_BODY()

	/** 버프 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** 버프 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** 체력 ?�복??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRestore = 0.0f;

	/** 마나 ?�복??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRestore = 0.0f;

	/** ?�태미나 ?�복??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRestore = 0.0f;

	/** 공격??증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** 방어??증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** ?�동?�도 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** ?�리?�컬 ?�률 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** 경험�??�득??증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** 버프 지?�시�?(�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 300.0f;

	/** ?�택 가???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bStackable = false;

	/** 최�? ?�택 ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	int32 MaxStacks = 1;
};

/**
 * ?�리 ?�료
 */
USTRUCT(BlueprintType)
struct FCookingIngredient
{
	GENERATED_BODY()

	/** ?�료 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	FName IngredientID;

	/** ?�요 ?�량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	int32 Quantity = 1;

	/** ?�택???�료 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	bool bOptional = false;
};

/**
 * ?�리 ?�시??
 */
USTRUCT(BlueprintType)
struct FCookingRecipe
{
	GENERATED_BODY()

	/** ?�시??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** ?�시???�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText RecipeName;

	/** ?�시???�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** ?�식 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** ?�리 방법 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** ?�요 ?�료 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FCookingIngredient> RequiredIngredients;

	/** ?�리 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float CookingTime = 10.0f;

	/** 최소 ?�리 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 MinCookingLevel = 1;

	/** ?�이??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 Difficulty = 5;

	/** 기본 버프 ?�과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FFoodBuffEffect BaseBuffEffect;

	/** ?�질�?버프 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TMap<ECookingQuality, float> QualityMultipliers;

	/** ?�시???�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?�성???�식 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UStaticMesh> FoodMesh;

	/** 경험�?보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 ExperienceReward = 20;

	/** ?�겨�??�시???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bHidden = false;
};

/**
 * ?�리 결과
 */
USTRUCT(BlueprintType)
struct FCookingResult
{
	GENERATED_BODY()

	/** ?�시??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName RecipeID;

	/** ?�리 ?�질 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	ECookingQuality Quality = ECookingQuality::Normal;

	/** ?�용??버프 ?�과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FFoodBuffEffect BuffEffect;

	/** ?�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ?�공 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess = true;

	/** ?�벽???�리 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * ?�리 ?�성
 */
USTRUCT(BlueprintType)
struct FCookingTrait
{
	GENERATED_BODY()

	/** ?�성 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** ?�성 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** ?�리 ?�도 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CookingSpeedBonus = 0.0f;

	/** ?�공?�률 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float SuccessRateBonus = 0.0f;

	/** ?�질 ?�상 ?�률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** 버프 ?�과 증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** 버프 지?�시�?증�? (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** ?�료 ?�약 ?�률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float IngredientSaveChance = 0.0f;

	/** 경험�?보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;
};

/**
 * ?�리 ?�시???�이???�이�?Row
 * ?�디?�에??DataTable�??�시?��? 관리할 ???�용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FRecipeDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?�시??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** ?�시 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText DisplayName;

	/** ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** ?�식 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** ?�리 방법 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** ?�요 ?�료 (?�이??ID -> ?�량) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Ingredients")
	TMap<FName, int32> Ingredients;

	/** ?�리 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Cooking")
	float CookingTime = 5.0f;

	/** 최소 ?�리 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Requirement")
	int32 MinCookingLevel = 1;

	/** ?�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Economy")
	int32 ExperienceGain = 10;

	/** ?�이??(1-10, ?�질 ?�률???�향) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Cooking")
	int32 Difficulty = 1;

	/** 기본 버프 ?�과 (?�질???�라 배율 ?�용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Buff")
	FFoodBuffEffect BuffEffect;

	/** 버프 지?�시�?(�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Buff")
	float BuffDuration = 300.0f;

	/** ?�겨�??�시???��? (?�험???�해 발견) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bIsHidden = false;

	/** ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 기본 ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Economy")
	int32 BasePrice = 20;
};
