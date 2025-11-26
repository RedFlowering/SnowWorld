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
 * 요리 시스템 컴포넌트
 * 음식 제작, 버프 시스템, 레시피 관리
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
	// 요리 기본 기능
	// ====================================

	/** 요리 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool StartCooking(FName RecipeID);

	/** 요리 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void CancelCooking();

	/** 요리 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsCooking() const { return bIsCooking; }

	/** 현재 요리 진행도 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetCookingProgress() const;

	/** 레시피를 요리할 수 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool CanCookRecipe(FName RecipeID) const;

	// ====================================
	// 음식 소비
	// ====================================

	/** 음식 먹기 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool ConsumeFood(FName FoodID, ECookingQuality Quality);

	/** 현재 활성화된 버프 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FFoodBuffEffect> GetActiveBuffs() const { return ActiveBuffs; }

	/** 특정 버프가 활성화되어 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool HasBuff(FName BuffName) const;

	/** 버프 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveBuff(FName BuffName);

	// ====================================
	// 레시피 관리
	// ====================================

	/** 레시피 발견 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void DiscoverRecipe(FName RecipeID);

	/** 레시피를 알고 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsRecipeKnown(FName RecipeID) const;

	/** 알고 있는 모든 레시피 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FName> GetKnownRecipes() const { return KnownRecipes; }

	/** 레시피 정보 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool GetRecipeData(FName RecipeID, FCookingRecipe& OutRecipe) const;

	// ====================================
	// 레벨 및 경험치 시스템
	// ====================================

	/** 요리 경험치 획득 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddCookingExperience(int32 Amount);

	/** 현재 요리 레벨 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	int32 GetCookingLevel() const { return CookingLevel; }

	// ====================================
	// 특성 시스템
	// ====================================

	/** 특성 추가 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddTrait(const FCookingTrait& Trait);

	/** 특성 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveTrait(FName TraitName);

	/** 모든 특성 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FCookingTrait> GetAllTraits() const { return ActiveTraits; }

	/** 총 요리 속도 보너스 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalCookingSpeedBonus() const;

	/** 총 성공률 보너스 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalSuccessRateBonus() const;

	/** 총 품질 보너스 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalQualityBonus() const;

	// ====================================
	// 이벤트
	// ====================================

	/** 요리 시작 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingStarted OnCookingStarted;

	/** 요리 취소 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCancelled OnCookingCancelled;

	/** 요리 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCompleted OnCookingCompleted;

	/** 레벨업 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingLevelUp OnCookingLevelUp;

	/** 레시피 발견 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnRecipeDiscovered OnRecipeDiscovered;

	/** 음식 섭취 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnFoodConsumed OnFoodConsumed;

	// ====================================
	// 설정
	// ====================================

	/** 레시피 데이터베이스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TMap<FName, FCookingRecipe> RecipeDatabase;

	/** 기본 성공률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float BaseSuccessRate = 70.0f;

private:
	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** 요리 중 플래그 */
	UPROPERTY()
	bool bIsCooking = false;

	/** 현재 요리 중인 레시피 */
	UPROPERTY()
	FName CurrentRecipeID;

	/** 요리 시작 시간 */
	UPROPERTY()
	float CookingStartTime = 0.0f;

	/** 요리 완료까지 필요한 시간 */
	UPROPERTY()
	float RequiredCookingTime = 0.0f;

	/** 요리 레벨 */
	UPROPERTY()
	int32 CookingLevel = 1;

	/** 알려진 레시피 */
	UPROPERTY()
	TArray<FName> KnownRecipes;

	/** 활성화된 버프 목록 */
	UPROPERTY()
	TArray<FFoodBuffEffect> ActiveBuffs;

	/** 버프 타이머 핸들 */
	UPROPERTY()
	TMap<FName, FTimerHandle> BuffTimerHandles;

	/** 활성화된 특성 목록 */
	UPROPERTY()
	TArray<FCookingTrait> ActiveTraits;

	/** 요리 완료 처리 */
	void CompleteCooking();

	/** 요리 결과 계산 */
	FCookingResult CalculateCookingResult(const FCookingRecipe& Recipe);

	/** 요리 품질 결정 */
	ECookingQuality DetermineCookingQuality(int32 Difficulty);

	/** 버프 효과 적용 */
	void ApplyBuffEffect(const FFoodBuffEffect& BuffEffect);

	/** 버프 만료 처리 */
	void OnBuffExpired(FName BuffName);

	/** 레벨 체크 및 처리 */
	void CheckAndProcessLevelUp();

	/** 요리 시간 계산 (보너스 적용) */
	float CalculateCookingTime(float BaseTime) const;

	/** 재료 보유 체크 */
	bool HasRequiredIngredients(const FCookingRecipe& Recipe) const;

	/** 재료 소비 */
	bool ConsumeIngredients(const FCookingRecipe& Recipe);
};
