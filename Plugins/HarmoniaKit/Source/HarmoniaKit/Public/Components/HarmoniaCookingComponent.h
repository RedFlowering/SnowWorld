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
 * ?”ë¦¬ ?œìŠ¤??ì»´í¬?ŒíŠ¸
 * ?Œì‹ ?œì‘, ë²„í”„ ?œìŠ¤?? ?ˆì‹œ??ê´€ë¦?
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
	// ?”ë¦¬ ê¸°ë³¸ ê¸°ëŠ¥
	// ====================================

	/** ?”ë¦¬ ?œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool StartCooking(FName RecipeID);

	/** ?”ë¦¬ ì·¨ì†Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void CancelCooking();

	/** ?”ë¦¬ ì¤‘ì¸ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsCooking() const { return bIsCooking; }

	/** ?„ì¬ ?”ë¦¬ ì§„í–‰??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetCookingProgress() const;

	/** ?ˆì‹œ?¼ë? ?”ë¦¬?????ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool CanCookRecipe(FName RecipeID) const;

	// ====================================
	// ?Œì‹ ?Œë¹„
	// ====================================

	/** ?Œì‹ ë¨¹ê¸° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	bool ConsumeFood(FName FoodID, ECookingQuality Quality);

	/** ?„ì¬ ?œì„±?”ëœ ë²„í”„ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FFoodBuffEffect> GetActiveBuffs() const { return ActiveBuffs; }

	/** ?¹ì • ë²„í”„ê°€ ?œì„±?”ë˜???ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool HasBuff(FName BuffName) const;

	/** ë²„í”„ ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveBuff(FName BuffName);

	// ====================================
	// ?ˆì‹œ??ê´€ë¦?
	// ====================================

	/** ?ˆì‹œ??ë°œê²¬ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void DiscoverRecipe(FName RecipeID);

	/** ?ˆì‹œ?¼ë? ?Œê³  ?ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool IsRecipeKnown(FName RecipeID) const;

	/** ?Œê³  ?ˆëŠ” ëª¨ë“  ?ˆì‹œ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FName> GetKnownRecipes() const { return KnownRecipes; }

	/** ?ˆì‹œ???•ë³´ ê°€?¸ì˜¤ê¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	bool GetRecipeData(FName RecipeID, FCookingRecipe& OutRecipe) const;

	// ====================================
	// ?ˆë²¨ ë°?ê²½í—˜ì¹??œìŠ¤??
	// ====================================

	/** ?”ë¦¬ ê²½í—˜ì¹??ë“ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddCookingExperience(int32 Amount);

	/** ?„ì¬ ?”ë¦¬ ?ˆë²¨ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	int32 GetCookingLevel() const { return CookingLevel; }

	// ====================================
	// ?¹ì„± ?œìŠ¤??
	// ====================================

	/** ?¹ì„± ì¶”ê? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void AddTrait(const FCookingTrait& Trait);

	/** ?¹ì„± ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cooking")
	void RemoveTrait(FName TraitName);

	/** ëª¨ë“  ?¹ì„± ê°€?¸ì˜¤ê¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	TArray<FCookingTrait> GetAllTraits() const { return ActiveTraits; }

	/** ì´??”ë¦¬ ?ë„ ë³´ë„ˆ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalCookingSpeedBonus() const;

	/** ì´??±ê³µë¥?ë³´ë„ˆ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalSuccessRateBonus() const;

	/** ì´??ˆì§ˆ ë³´ë„ˆ??*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cooking")
	float GetTotalQualityBonus() const;

	// ====================================
	// ?´ë²¤??
	// ====================================

	/** ?”ë¦¬ ?œì‘ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingStarted OnCookingStarted;

	/** ?”ë¦¬ ì·¨ì†Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCancelled OnCookingCancelled;

	/** ?”ë¦¬ ?„ë£Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingCompleted OnCookingCompleted;

	/** ?ˆë²¨???´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnCookingLevelUp OnCookingLevelUp;

	/** ?ˆì‹œ??ë°œê²¬ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnRecipeDiscovered OnRecipeDiscovered;

	/** ?Œì‹ ??·¨ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Cooking")
	FOnFoodConsumed OnFoodConsumed;

	// ====================================
	// ?¤ì •
	// ====================================

	/** ?ˆì‹œ???°ì´?°ë² ?´ìŠ¤ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	TMap<FName, FCookingRecipe> RecipeDatabase;

	/** ê¸°ë³¸ ?±ê³µë¥?(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Settings")
	float BaseSuccessRate = 70.0f;

private:
	/** ?¸ë²¤? ë¦¬ ì»´í¬?ŒíŠ¸ ì°¸ì¡° */
	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	/** ?”ë¦¬ ì¤??Œë˜ê·?*/
	UPROPERTY()
	bool bIsCooking = false;

	/** ?„ì¬ ?”ë¦¬ ì¤‘ì¸ ?ˆì‹œ??*/
	UPROPERTY()
	FName CurrentRecipeID;

	/** ?”ë¦¬ ?œì‘ ?œê°„ */
	UPROPERTY()
	float CookingStartTime = 0.0f;

	/** ?”ë¦¬ ?„ë£Œê¹Œì? ?„ìš”???œê°„ */
	UPROPERTY()
	float RequiredCookingTime = 0.0f;

	/** ?”ë¦¬ ?ˆë²¨ */
	UPROPERTY()
	int32 CookingLevel = 1;

	/** ?Œë ¤ì§??ˆì‹œ??*/
	UPROPERTY()
	TArray<FName> KnownRecipes;

	/** ?œì„±?”ëœ ë²„í”„ ëª©ë¡ */
	UPROPERTY()
	TArray<FFoodBuffEffect> ActiveBuffs;

	/** ë²„í”„ ?€?´ë¨¸ ?¸ë“¤ */
	UPROPERTY()
	TMap<FName, FTimerHandle> BuffTimerHandles;

	/** ?œì„±?”ëœ ?¹ì„± ëª©ë¡ */
	UPROPERTY()
	TArray<FCookingTrait> ActiveTraits;

	/** ?”ë¦¬ ?„ë£Œ ì²˜ë¦¬ */
	void CompleteCooking();

	/** ?”ë¦¬ ê²°ê³¼ ê³„ì‚° */
	FCookingResult CalculateCookingResult(const FCookingRecipe& Recipe);

	/** ?”ë¦¬ ?ˆì§ˆ ê²°ì • */
	ECookingQuality DetermineCookingQuality(int32 Difficulty);

	/** ë²„í”„ ?¨ê³¼ ?ìš© */
	void ApplyBuffEffect(const FFoodBuffEffect& BuffEffect);

	/** ë²„í”„ ë§Œë£Œ ì²˜ë¦¬ */
	void OnBuffExpired(FName BuffName);

	/** ?ˆë²¨ ì²´í¬ ë°?ì²˜ë¦¬ */
	void CheckAndProcessLevelUp();

	/** ?”ë¦¬ ?œê°„ ê³„ì‚° (ë³´ë„ˆ???ìš©) */
	float CalculateCookingTime(float BaseTime) const;

	/** ?¬ë£Œ ë³´ìœ  ì²´í¬ */
	bool HasRequiredIngredients(const FCookingRecipe& Recipe) const;

	/** ?¬ë£Œ ?Œë¹„ */
	bool ConsumeIngredients(const FCookingRecipe& Recipe);
};
