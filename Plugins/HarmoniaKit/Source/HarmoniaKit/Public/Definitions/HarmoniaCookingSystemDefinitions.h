// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HarmoniaCookingSystemDefinitions.generated.h"

/**
 * 음식 타입
 */
UENUM(BlueprintType)
enum class EFoodType : uint8
{
	Meal		UMETA(DisplayName = "Meal"),		// 식사
	Soup		UMETA(DisplayName = "Soup"),		// 수프
	Dessert		UMETA(DisplayName = "Dessert"),		// 디저트
	Beverage	UMETA(DisplayName = "Beverage"),	// 음료
	Snack		UMETA(DisplayName = "Snack"),		// 간식
	Elixir		UMETA(DisplayName = "Elixir"),		// 엘릭서
	Salad		UMETA(DisplayName = "Salad")		// 샐러드
};

/**
 * 요리 품질
 */
UENUM(BlueprintType)
enum class ECookingQuality : uint8
{
	Failed		UMETA(DisplayName = "Failed"),		// 실패작
	Poor		UMETA(DisplayName = "Poor"),		// 조악함
	Normal		UMETA(DisplayName = "Normal"),		// 보통
	Good		UMETA(DisplayName = "Good"),		// 좋음
	Excellent	UMETA(DisplayName = "Excellent"),	// 훌륭함
	Masterpiece	UMETA(DisplayName = "Masterpiece")	// 걸작
};

/**
 * 요리 방법
 */
UENUM(BlueprintType)
enum class ECookingMethod : uint8
{
	Boiling		UMETA(DisplayName = "Boiling"),		// 끓이기
	Frying		UMETA(DisplayName = "Frying"),		// 굽기
	Grilling	UMETA(DisplayName = "Grilling"),	// 그릴
	Steaming	UMETA(DisplayName = "Steaming"),	// 찌기
	Baking		UMETA(DisplayName = "Baking"),		// 베이킹
	Mixing		UMETA(DisplayName = "Mixing"),		// 섞기
	Roasting	UMETA(DisplayName = "Roasting")		// 로스팅
};

/**
 * 음식 버프 효과
 */
USTRUCT(BlueprintType)
struct FFoodBuffEffect
{
	GENERATED_BODY()

	/** 버프 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FName BuffName;

	/** 버프 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	FText Description;

	/** 체력 회복량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float HealthRestore = 0.0f;

	/** 마나 회복량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ManaRestore = 0.0f;

	/** 스태미나 회복량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float StaminaRestore = 0.0f;

	/** 공격력 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float AttackBonus = 0.0f;

	/** 방어력 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float DefenseBonus = 0.0f;

	/** 이동속도 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float SpeedBonus = 0.0f;

	/** 크리티컬 확률 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float CriticalChanceBonus = 0.0f;

	/** 경험치 획득량 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float ExperienceBonus = 0.0f;

	/** 버프 지속시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	float Duration = 300.0f;

	/** 스택 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	bool bStackable = false;

	/** 최대 스택 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
	int32 MaxStacks = 1;
};

/**
 * 요리 재료
 */
USTRUCT(BlueprintType)
struct FCookingIngredient
{
	GENERATED_BODY()

	/** 재료 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	FName IngredientID;

	/** 필요 수량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	int32 Quantity = 1;

	/** 선택적 재료 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	bool bOptional = false;
};

/**
 * 요리 레시피
 */
USTRUCT(BlueprintType)
struct FCookingRecipe
{
	GENERATED_BODY()

	/** 레시피 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FName RecipeID;

	/** 레시피 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText RecipeName;

	/** 레시피 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FText Description;

	/** 음식 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	EFoodType FoodType = EFoodType::Meal;

	/** 요리 방법 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	ECookingMethod CookingMethod = ECookingMethod::Boiling;

	/** 필요 재료 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TArray<FCookingIngredient> RequiredIngredients;

	/** 요리 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	float CookingTime = 10.0f;

	/** 최소 요리 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 MinCookingLevel = 1;

	/** 난이도 (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 Difficulty = 5;

	/** 기본 버프 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	FFoodBuffEffect BaseBuffEffect;

	/** 품질별 버프 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TMap<ECookingQuality, float> QualityMultipliers;

	/** 레시피 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 완성된 음식 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TSoftObjectPtr<UStaticMesh> FoodMesh;

	/** 경험치 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	int32 ExperienceReward = 20;

	/** 숨겨진 레시피 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	bool bHidden = false;
};

/**
 * 요리 결과
 */
USTRUCT(BlueprintType)
struct FCookingResult
{
	GENERATED_BODY()

	/** 레시피 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName RecipeID;

	/** 요리 품질 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	ECookingQuality Quality = ECookingQuality::Normal;

	/** 적용된 버프 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FFoodBuffEffect BuffEffect;

	/** 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** 성공 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess = true;

	/** 완벽한 요리 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPerfect = false;
};

/**
 * 요리 특성
 */
USTRUCT(BlueprintType)
struct FCookingTrait
{
	GENERATED_BODY()

	/** 특성 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** 특성 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** 요리 속도 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CookingSpeedBonus = 0.0f;

	/** 성공확률 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float SuccessRateBonus = 0.0f;

	/** 품질 향상 확률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float QualityBonus = 0.0f;

	/** 버프 효과 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffEffectBonus = 0.0f;

	/** 버프 지속시간 증가 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float BuffDurationBonus = 0.0f;

	/** 재료 절약 확률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float IngredientSaveChance = 0.0f;

	/** 경험치 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;
};
