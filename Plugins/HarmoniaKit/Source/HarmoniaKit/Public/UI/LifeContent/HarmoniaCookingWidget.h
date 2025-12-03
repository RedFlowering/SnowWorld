// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UI/LifeContent/HarmoniaLifeContentWidget.h"
#include "Definitions/HarmoniaCookingSystemDefinitions.h"
#include "HarmoniaCookingWidget.generated.h"

class UHarmoniaCookingComponent;
class UImage;
class UTextBlock;
class UProgressBar;
class UUniformGridPanel;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeSelected, FName, RecipeID);

/**
 * Cooking UI Widget
 * Displays cooking progress, recipe list, and result information
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCookingWidget : public UHarmoniaLifeContentWidget
{
	GENERATED_BODY()

public:
	UHarmoniaCookingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ====================================
	// Setup
	// ====================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void SetCookingComponent(UHarmoniaCookingComponent* InComponent);

	virtual void RefreshDisplay() override;

	// ====================================
	// Recipe Management
	// ====================================

	/** Refresh recipe list display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void RefreshRecipeList();

	/** Show recipe details */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void ShowRecipeDetails(FName RecipeID, const FText& RecipeName, ECookingMethod Method, 
						   float CookingTime, const TArray<FName>& Ingredients);

	/** Hide recipe details */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void HideRecipeDetails();

	// ====================================
	// Cooking Progress
	// ====================================

	/** Update cooking progress display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void UpdateCookingProgress(float Progress, float RemainingTime);

	/** Show cooking result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void ShowCookingResult(const FText& FoodName, ECookingQuality Quality, const FFoodBuffEffect& BuffEffect);

	/** Hide cooking result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Cooking")
	void HideCookingResult();

	// ====================================
	// Events
	// ====================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UI|Cooking")
	FOnRecipeSelected OnRecipeSelected;

protected:
	// ====================================
	// Recipe List Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> RecipeListPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|UI|Cooking")
	TSubclassOf<UUserWidget> RecipeEntryWidgetClass;

	// ====================================
	// Recipe Details Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> RecipeDetailsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CookingMethodText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CookingTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IngredientsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartCookingButton;

	// ====================================
	// Cooking Progress Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CookingProgressPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> CookingProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CookingProgressText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RemainingTimeText;

	// ====================================
	// Result Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CookingResultPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ResultFoodIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultFoodNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultQualityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultBuffText;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|Cooking")
	TObjectPtr<UHarmoniaCookingComponent> CookingComponent;

	// ====================================
	// State
	// ====================================

	UPROPERTY()
	FName CurrentSelectedRecipe;

private:
	FText GetCookingMethodText(ECookingMethod Method) const;
	FText GetQualityText(ECookingQuality Quality) const;
	FLinearColor GetQualityColor(ECookingQuality Quality) const;
	FText FormatBuffEffects(const FFoodBuffEffect& BuffEffect) const;
};
