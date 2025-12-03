// Copyright Epic Games, Inc. All Rights Reserved.

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
 * ?�리 ?�스??컴포?�트
 * ?�식 ?�작, 버프 ?�스?? ?�시??관�?
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
	// ?�리 기본 기능
	// ====================================

	/** ?�리 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool StartCooking(FName RecipeID);

	/** ?�리 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void CancelCooking();

	/** ?�리 중인지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsCooking() const { return bIsCooking; }

	/** ?�재 ?�리 진행??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetCookingProgress() const;

	/** ?�시?��? ?�리?????�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool CanCookRecipe(FName RecipeID) const;

	// ====================================
	// ?�식 ?�비
	// ====================================

	/** ?�식 먹기 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool ConsumeFood(FName FoodID, ECookingQuality Quality);

	/** ?�재 ?�성?�된 버프 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FFoodBuffEffect> GetActiveBuffs() const { return ActiveBuffs; }

	/** ?�정 버프가 ?�성?�되???�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool HasBuff(FName BuffName) const;

	/** 버프 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveBuff(FName BuffName);

	// ====================================
	// ?�시??관�?
	// ====================================

	/** ?�시??발견 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void DiscoverRecipe(FName RecipeID);

	/** ?�시?��? ?�고 ?�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsRecipeKnown(FName RecipeID) const;

	/** ?�고 ?�는 모든 ?�시??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FName> GetKnownRecipes() const { return KnownRecipes; }

	/** ?�시???�보 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool GetRecipeData(FName RecipeID, FCookingRecipe& OutRecipe) const;

	// ====================================
	// ?�벨 �?경험�??�스??
	// ====================================

	/** ?�리 경험�??�득 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddCookingExperience(int32 Amount);

	/** ?�재 ?�리 ?�벨 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	int32 GetCookingLevel() const { return CookingLevel; }

	// ====================================
	// ?�성 ?�스??
	// ====================================

	/** ?�성 추�? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddTrait(const FCookingTrait& Trait);

	/** ?�성 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveTrait(FName TraitName);

	/** 모든 ?�성 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FCookingTrait> GetAllTraits() const { return ActiveTraits; }

	/** �??�리 ?�도 보너??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalCookingSpeedBonus() const;

	/** �??�공�?보너??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalSuccessRateBonus() const;

	/** �??�질 보너??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalQualityBonus() const;

	// ====================================
	// ?�벤??
	// ====================================

	/** ?�리 ?�작 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingStarted OnCookingStarted;

	/** ?�리 취소 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCancelled OnCookingCancelled;

	/** ?�리 ?�료 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCompleted OnCookingCompleted;

	/** ?�벨???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingLevelUp OnCookingLevelUp;

	/** ?�시??발견 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnRecipeDiscovered OnRecipeDiscovered;

	/** ?�식 ??�� ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnFoodConsumed OnFoodConsumed;

	// ====================================
	// ?�정
	// ====================================

	/** ?�시???�이?�베?�스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TMap<FName, FCookingRecipe> RecipeDatabase;

	/** 기본 ?�공�?(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float BaseSuccessRate = 70.0f;

private:
	/** ?�벤?�리 컴포?�트 참조 */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** ?�리 �??�래�?*/
	UPROPERTY()
	bool bIsCooking = false;

	/** ?�재 ?�리 중인 ?�시??*/
	UPROPERTY()
	FName CurrentRecipeID;

	/** ?�리 ?�작 ?�간 */
	UPROPERTY()
	float CookingStartTime = 0.0f;

	/** ?�리 ?�료까�? ?�요???�간 */
	UPROPERTY()
	float RequiredCookingTime = 0.0f;

	/** ?�리 ?�벨 */
	UPROPERTY()
	int32 CookingLevel = 1;

	/** ?�려�??�시??*/
	UPROPERTY()
	TArray<FName> KnownRecipes;

	/** ?�성?�된 버프 목록 */
	UPROPERTY()
	TArray<FFoodBuffEffect> ActiveBuffs;

	/** 버프 ?�?�머 ?�들 */
	UPROPERTY()
	TMap<FName, FTimerHandle> BuffTimerHandles;

	/** ?�성?�된 ?�성 목록 */
	UPROPERTY()
	TArray<FCookingTrait> ActiveTraits;

	/** ?�리 ?�료 처리 */
	void CompleteCooking();

	/** ?�리 결과 계산 */
	FCookingResult CalculateCookingResult(const FCookingRecipe& Recipe);

	/** ?�리 ?�질 결정 */
	ECookingQuality DetermineCookingQuality(int32 Difficulty);

	/** 버프 ?�과 ?�용 */
	void ApplyBuffEffect(const FFoodBuffEffect& BuffEffect);

	/** 버프 만료 처리 */
	void OnBuffExpired(FName BuffName);

	/** ?�벨 체크 �?처리 */
	void CheckAndProcessLevelUp();

	/** ?�리 ?�간 계산 (보너???�용) */
	float CalculateCookingTime(float BaseTime) const;

	/** ?�료 보유 체크 */
	bool HasRequiredIngredients(const FCookingRecipe& Recipe) const;

	/** ?�료 ?�비 */
	bool ConsumeIngredients(const FCookingRecipe& Recipe);
};
