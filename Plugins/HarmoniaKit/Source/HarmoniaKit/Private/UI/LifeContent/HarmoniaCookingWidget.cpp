// Copyright 2025 Snow Game Studio.

#include "UI/LifeContent/HarmoniaCookingWidget.h"
#include "Components/HarmoniaCookingComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"

UHarmoniaCookingWidget::UHarmoniaCookingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaCookingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideRecipeDetails();
	HideCookingResult();

	if (CookingProgressPanel)
	{
		CookingProgressPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaCookingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UHarmoniaCookingWidget::SetCookingComponent(UHarmoniaCookingComponent* InComponent)
{
	CookingComponent = InComponent;
	SetLifeContentComponent(InComponent);
}

void UHarmoniaCookingWidget::RefreshDisplay()
{
	Super::RefreshDisplay();
	RefreshRecipeList();
}

void UHarmoniaCookingWidget::RefreshRecipeList()
{
	// Recipe list would be populated here using CookingComponent's discovered recipes
	// This is a placeholder - actual implementation would iterate through recipes
	// and create entry widgets
}

void UHarmoniaCookingWidget::ShowRecipeDetails(FName RecipeID, const FText& RecipeName, ECookingMethod Method,
											   float CookingTime, const TArray<FName>& Ingredients)
{
	CurrentSelectedRecipe = RecipeID;

	if (RecipeDetailsPanel)
	{
		RecipeDetailsPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (RecipeNameText)
	{
		RecipeNameText->SetText(RecipeName);
	}

	if (CookingMethodText)
	{
		CookingMethodText->SetText(GetCookingMethodText(Method));
	}

	if (CookingTimeText)
	{
		CookingTimeText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaCooking", "CookingTime", "조리 시간: {0}초"),
			FText::AsNumber(FMath::RoundToInt(CookingTime))));
	}

	if (IngredientsText && Ingredients.Num() > 0)
	{
		FString IngredientsStr;
		for (int32 i = 0; i < Ingredients.Num(); ++i)
		{
			if (i > 0) IngredientsStr += TEXT(", ");
			IngredientsStr += Ingredients[i].ToString();
		}
		IngredientsText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaCooking", "Ingredients", "재료: {0}"),
			FText::FromString(IngredientsStr)));
	}
}

void UHarmoniaCookingWidget::HideRecipeDetails()
{
	CurrentSelectedRecipe = NAME_None;

	if (RecipeDetailsPanel)
	{
		RecipeDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaCookingWidget::UpdateCookingProgress(float Progress, float RemainingTime)
{
	if (CookingProgressPanel)
	{
		CookingProgressPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (CookingProgressBar)
	{
		CookingProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}

	if (CookingProgressText)
	{
		CookingProgressText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaCooking", "ProgressPercent", "{0}%"),
			FText::AsNumber(FMath::RoundToInt(Progress * 100.0f))));
	}

	if (RemainingTimeText)
	{
		const int32 Seconds = FMath::RoundToInt(RemainingTime);
		const int32 Minutes = Seconds / 60;
		const int32 Secs = Seconds % 60;

		if (Minutes > 0)
		{
			RemainingTimeText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaCooking", "RemainingTimeMinSec", "{0}분 {1}초"),
				FText::AsNumber(Minutes),
				FText::AsNumber(Secs)));
		}
		else
		{
			RemainingTimeText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaCooking", "RemainingTimeSec", "{0}초"),
				FText::AsNumber(Secs)));
		}
	}
}

void UHarmoniaCookingWidget::ShowCookingResult(const FText& FoodName, ECookingQuality Quality, const FFoodBuffEffect& BuffEffect)
{
	if (CookingProgressPanel)
	{
		CookingProgressPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CookingResultPanel)
	{
		CookingResultPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (ResultFoodNameText)
	{
		ResultFoodNameText->SetText(FoodName);
	}

	if (ResultQualityText)
	{
		ResultQualityText->SetText(GetQualityText(Quality));
		ResultQualityText->SetColorAndOpacity(GetQualityColor(Quality));
	}

	if (ResultBuffText)
	{
		ResultBuffText->SetText(FormatBuffEffects(BuffEffect));
	}
}

void UHarmoniaCookingWidget::HideCookingResult()
{
	if (CookingResultPanel)
	{
		CookingResultPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FText UHarmoniaCookingWidget::GetCookingMethodText(ECookingMethod Method) const
{
	switch (Method)
	{
	case ECookingMethod::Boiling:
		return NSLOCTEXT("HarmoniaCooking", "MethodBoiling", "끓이기");
	case ECookingMethod::Frying:
		return NSLOCTEXT("HarmoniaCooking", "MethodFrying", "굽기");
	case ECookingMethod::Grilling:
		return NSLOCTEXT("HarmoniaCooking", "MethodGrilling", "그릴");
	case ECookingMethod::Steaming:
		return NSLOCTEXT("HarmoniaCooking", "MethodSteaming", "찌기");
	case ECookingMethod::Baking:
		return NSLOCTEXT("HarmoniaCooking", "MethodBaking", "베이킹");
	case ECookingMethod::Mixing:
		return NSLOCTEXT("HarmoniaCooking", "MethodMixing", "섞기");
	case ECookingMethod::Roasting:
		return NSLOCTEXT("HarmoniaCooking", "MethodRoasting", "로스팅");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaCookingWidget::GetQualityText(ECookingQuality Quality) const
{
	switch (Quality)
	{
	case ECookingQuality::Failed:
		return NSLOCTEXT("HarmoniaCooking", "QualityFailed", "실패작");
	case ECookingQuality::Poor:
		return NSLOCTEXT("HarmoniaCooking", "QualityPoor", "조악함");
	case ECookingQuality::Normal:
		return NSLOCTEXT("HarmoniaCooking", "QualityNormal", "보통");
	case ECookingQuality::Good:
		return NSLOCTEXT("HarmoniaCooking", "QualityGood", "좋음");
	case ECookingQuality::Excellent:
		return NSLOCTEXT("HarmoniaCooking", "QualityExcellent", "훌륭함");
	case ECookingQuality::Masterpiece:
		return NSLOCTEXT("HarmoniaCooking", "QualityMasterpiece", "걸작");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaCookingWidget::GetQualityColor(ECookingQuality Quality) const
{
	switch (Quality)
	{
	case ECookingQuality::Failed:
		return FLinearColor::Red;
	case ECookingQuality::Poor:
		return FLinearColor::Gray;
	case ECookingQuality::Normal:
		return FLinearColor::White;
	case ECookingQuality::Good:
		return FLinearColor::Green;
	case ECookingQuality::Excellent:
		return FLinearColor(0.0f, 0.5f, 1.0f); // Blue
	case ECookingQuality::Masterpiece:
		return FLinearColor(1.0f, 0.84f, 0.0f); // Gold
	default:
		return FLinearColor::White;
	}
}

FText UHarmoniaCookingWidget::FormatBuffEffects(const FFoodBuffEffect& BuffEffect) const
{
	TArray<FString> Effects;

	if (BuffEffect.HealthRestore > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("체력 +%.0f"), BuffEffect.HealthRestore));
	}
	if (BuffEffect.ManaRestore > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("마나 +%.0f"), BuffEffect.ManaRestore));
	}
	if (BuffEffect.StaminaRestore > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("스태미나 +%.0f"), BuffEffect.StaminaRestore));
	}
	if (BuffEffect.AttackBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("공격력 +%.0f%%"), BuffEffect.AttackBonus));
	}
	if (BuffEffect.DefenseBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("방어력 +%.0f%%"), BuffEffect.DefenseBonus));
	}
	if (BuffEffect.SpeedBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("이동속도 +%.0f%%"), BuffEffect.SpeedBonus));
	}
	if (BuffEffect.CriticalChanceBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("크리티컬 +%.0f%%"), BuffEffect.CriticalChanceBonus));
	}
	if (BuffEffect.ExperienceBonus > 0.0f)
	{
		Effects.Add(FString::Printf(TEXT("경험치 +%.0f%%"), BuffEffect.ExperienceBonus));
	}

	return FText::FromString(FString::Join(Effects, TEXT("\n")));
}
