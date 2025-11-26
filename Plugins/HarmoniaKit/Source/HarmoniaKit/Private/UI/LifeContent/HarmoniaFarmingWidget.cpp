// Copyright 2025 Snow Game Studio.

#include "UI/LifeContent/HarmoniaFarmingWidget.h"
#include "Components/HarmoniaFarmingComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/UniformGridPanel.h"

UHarmoniaFarmingWidget::UHarmoniaFarmingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaFarmingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideCropInfo();
	HideHarvestResult();
}

void UHarmoniaFarmingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UHarmoniaFarmingWidget::SetFarmingComponent(UHarmoniaFarmingComponent* InComponent)
{
	FarmingComponent = InComponent;
	SetLifeContentComponent(InComponent);
}

void UHarmoniaFarmingWidget::RefreshDisplay()
{
	Super::RefreshDisplay();
}

void UHarmoniaFarmingWidget::UpdatePlotDisplay(FName PlotID, bool bHasCrop, float GrowthProgress, float MoistureLevel)
{
	// Plot grid would be updated here
	// Actual implementation would find the plot widget and update it
}

void UHarmoniaFarmingWidget::ShowPlotDetails(FName PlotID, ECropGrowthStage GrowthStage, float Moisture,
											 ESoilQuality SoilQuality, bool bHasFertilizer)
{
	if (PlotDetailsPanel)
	{
		PlotDetailsPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (PlotIDText)
	{
		PlotIDText->SetText(FText::FromName(PlotID));
	}

	if (GrowthStageText)
	{
		GrowthStageText->SetText(GetGrowthStageText(GrowthStage));
	}

	if (MoistureBar)
	{
		MoistureBar->SetPercent(FMath::Clamp(Moisture, 0.0f, 1.0f));

		// Color based on moisture level
		if (Moisture < 0.2f)
		{
			MoistureBar->SetFillColorAndOpacity(FLinearColor::Red); // Dry
		}
		else if (Moisture < 0.4f)
		{
			MoistureBar->SetFillColorAndOpacity(FLinearColor::Yellow); // Low
		}
		else if (Moisture > 0.9f)
		{
			MoistureBar->SetFillColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f)); // Overwatered
		}
		else
		{
			MoistureBar->SetFillColorAndOpacity(FLinearColor::Green); // Good
		}
	}

	if (MoistureText)
	{
		MoistureText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFarming", "MoistureLevel", "수분: {0}%"),
			FText::AsNumber(FMath::RoundToInt(Moisture * 100.0f))));
	}

	if (SoilQualityText)
	{
		SoilQualityText->SetText(GetSoilQualityText(SoilQuality));
		SoilQualityText->SetColorAndOpacity(GetSoilQualityColor(SoilQuality));
	}

	if (FertilizerStatusText)
	{
		if (bHasFertilizer)
		{
			FertilizerStatusText->SetText(NSLOCTEXT("HarmoniaFarming", "HasFertilizer", "비료 적용됨"));
			FertilizerStatusText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			FertilizerStatusText->SetText(NSLOCTEXT("HarmoniaFarming", "NoFertilizer", "비료 없음"));
			FertilizerStatusText->SetColorAndOpacity(FLinearColor::Gray);
		}
	}
}

void UHarmoniaFarmingWidget::ShowCropInfo(const FText& CropName, ECropType CropType, ESeason RequiredSeason,
											  float GrowthTime, bool bReharvestable)
{
	if (CropInfoPanel)
	{
		CropInfoPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (CropNameText)
	{
		CropNameText->SetText(CropName);
	}

	if (CropTypeText)
	{
		CropTypeText->SetText(GetCropTypeText(CropType));
	}

	if (CropSeasonText)
	{
		CropSeasonText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFarming", "CropSeason", "계절: {0}"),
			GetSeasonText(RequiredSeason)));
		CropSeasonText->SetColorAndOpacity(GetSeasonColor(RequiredSeason));
	}

	if (CropGrowthTimeText)
	{
		const int32 Hours = FMath::RoundToInt(GrowthTime / 3600.0f);
		CropGrowthTimeText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFarming", "GrowthTime", "성장 시간: {0}시간"),
			FText::AsNumber(Hours)));
	}

	if (CropReharvestText)
	{
		if (bReharvestable)
		{
			CropReharvestText->SetText(NSLOCTEXT("HarmoniaFarming", "Reharvestable", "재수확 가능"));
			CropReharvestText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			CropReharvestText->SetText(NSLOCTEXT("HarmoniaFarming", "SingleHarvest", "1회 수확"));
			CropReharvestText->SetColorAndOpacity(FLinearColor::Gray);
		}
	}
}

void UHarmoniaFarmingWidget::HideCropInfo()
{
	if (CropInfoPanel)
	{
		CropInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaFarmingWidget::ShowHarvestResult(const FText& CropName, int32 Yield, float QualityScore,
											   bool bGotSeeds, int32 SeedAmount)
{
	if (HarvestResultPanel)
	{
		HarvestResultPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (HarvestCropNameText)
	{
		HarvestCropNameText->SetText(CropName);
	}

	if (HarvestYieldText)
	{
		HarvestYieldText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFarming", "HarvestYield", "수확량: x{0}"),
			FText::AsNumber(Yield)));
	}

	if (HarvestQualityText)
	{
		FText QualityRating;
		FLinearColor QualityColor;

		if (QualityScore >= 90.0f)
		{
			QualityRating = NSLOCTEXT("HarmoniaFarming", "QualityPerfect", "최고급");
			QualityColor = FLinearColor(1.0f, 0.84f, 0.0f); // Gold
		}
		else if (QualityScore >= 70.0f)
		{
			QualityRating = NSLOCTEXT("HarmoniaFarming", "QualityHigh", "고급");
			QualityColor = FLinearColor(0.5f, 0.0f, 0.5f); // Purple
		}
		else if (QualityScore >= 50.0f)
		{
			QualityRating = NSLOCTEXT("HarmoniaFarming", "QualityNormal", "보통");
			QualityColor = FLinearColor::Green;
		}
		else
		{
			QualityRating = NSLOCTEXT("HarmoniaFarming", "QualityLow", "저급");
			QualityColor = FLinearColor::Gray;
		}

		HarvestQualityText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFarming", "QualityFormat", "품질: {0} ({1}점)"),
			QualityRating,
			FText::AsNumber(FMath::RoundToInt(QualityScore))));
		HarvestQualityText->SetColorAndOpacity(QualityColor);
	}

	if (HarvestSeedText)
	{
		if (bGotSeeds && SeedAmount > 0)
		{
			HarvestSeedText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaFarming", "SeedObtained", "씨앗 획득: x{0}"),
				FText::AsNumber(SeedAmount)));
			HarvestSeedText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			HarvestSeedText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UHarmoniaFarmingWidget::HideHarvestResult()
{
	if (HarvestResultPanel)
	{
		HarvestResultPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaFarmingWidget::UpdateSeasonDisplay(ESeason CurrentSeason)
{
	if (CurrentSeasonText)
	{
		CurrentSeasonText->SetText(GetSeasonText(CurrentSeason));
		CurrentSeasonText->SetColorAndOpacity(GetSeasonColor(CurrentSeason));
	}
}

void UHarmoniaFarmingWidget::UpdateWeatherStatus(bool bIsRaining)
{
	if (WeatherStatusText)
	{
		if (bIsRaining)
		{
			WeatherStatusText->SetText(NSLOCTEXT("HarmoniaFarming", "WeatherRaining", "비"));
			WeatherStatusText->SetColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f));
		}
		else
		{
			WeatherStatusText->SetText(NSLOCTEXT("HarmoniaFarming", "WeatherClear", "맑음"));
			WeatherStatusText->SetColorAndOpacity(FLinearColor::Yellow);
		}
	}
}

FText UHarmoniaFarmingWidget::GetGrowthStageText(ECropGrowthStage Stage) const
{
	switch (Stage)
	{
	case ECropGrowthStage::Seed:
		return NSLOCTEXT("HarmoniaFarming", "StageSeed", "씨앗");
	case ECropGrowthStage::Sprout:
		return NSLOCTEXT("HarmoniaFarming", "StageSprout", "싹");
	case ECropGrowthStage::Growing:
		return NSLOCTEXT("HarmoniaFarming", "StageGrowing", "성장 중");
	case ECropGrowthStage::Mature:
		return NSLOCTEXT("HarmoniaFarming", "StageMature", "성숙");
	case ECropGrowthStage::Harvest:
		return NSLOCTEXT("HarmoniaFarming", "StageHarvest", "수확 가능");
	case ECropGrowthStage::Withered:
		return NSLOCTEXT("HarmoniaFarming", "StageWithered", "시들음");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaFarmingWidget::GetSoilQualityText(ESoilQuality Quality) const
{
	switch (Quality)
	{
	case ESoilQuality::Poor:
		return NSLOCTEXT("HarmoniaFarming", "SoilPoor", "척박함");
	case ESoilQuality::Normal:
		return NSLOCTEXT("HarmoniaFarming", "SoilNormal", "보통");
	case ESoilQuality::Good:
		return NSLOCTEXT("HarmoniaFarming", "SoilGood", "비옥함");
	case ESoilQuality::Excellent:
		return NSLOCTEXT("HarmoniaFarming", "SoilExcellent", "훌륭함");
	case ESoilQuality::Perfect:
		return NSLOCTEXT("HarmoniaFarming", "SoilPerfect", "완벽함");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaFarmingWidget::GetSoilQualityColor(ESoilQuality Quality) const
{
	switch (Quality)
	{
	case ESoilQuality::Poor:
		return FLinearColor::Gray;
	case ESoilQuality::Normal:
		return FLinearColor::White;
	case ESoilQuality::Good:
		return FLinearColor::Green;
	case ESoilQuality::Excellent:
		return FLinearColor(0.0f, 0.5f, 1.0f);
	case ESoilQuality::Perfect:
		return FLinearColor(1.0f, 0.84f, 0.0f);
	default:
		return FLinearColor::White;
	}
}

FText UHarmoniaFarmingWidget::GetCropTypeText(ECropType Type) const
{
	switch (Type)
	{
	case ECropType::Vegetable:
		return NSLOCTEXT("HarmoniaFarming", "TypeVegetable", "채소");
	case ECropType::Fruit:
		return NSLOCTEXT("HarmoniaFarming", "TypeFruit", "과일");
	case ECropType::Grain:
		return NSLOCTEXT("HarmoniaFarming", "TypeGrain", "곡물");
	case ECropType::Herb:
		return NSLOCTEXT("HarmoniaFarming", "TypeHerb", "약초");
	case ECropType::Flower:
		return NSLOCTEXT("HarmoniaFarming", "TypeFlower", "꽃");
	case ECropType::Tree:
		return NSLOCTEXT("HarmoniaFarming", "TypeTree", "나무");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaFarmingWidget::GetSeasonText(ESeason Season) const
{
	switch (Season)
	{
	case ESeason::Spring:
		return NSLOCTEXT("HarmoniaFarming", "SeasonSpring", "봄");
	case ESeason::Summer:
		return NSLOCTEXT("HarmoniaFarming", "SeasonSummer", "여름");
	case ESeason::Autumn:
		return NSLOCTEXT("HarmoniaFarming", "SeasonAutumn", "가을");
	case ESeason::Winter:
		return NSLOCTEXT("HarmoniaFarming", "SeasonWinter", "겨울");
	case ESeason::AllSeasons:
		return NSLOCTEXT("HarmoniaFarming", "SeasonAll", "모든 계절");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaFarmingWidget::GetSeasonColor(ESeason Season) const
{
	switch (Season)
	{
	case ESeason::Spring:
		return FLinearColor(0.5f, 1.0f, 0.5f); // Light green
	case ESeason::Summer:
		return FLinearColor(1.0f, 0.8f, 0.2f); // Yellow-orange
	case ESeason::Autumn:
		return FLinearColor(1.0f, 0.5f, 0.2f); // Orange
	case ESeason::Winter:
		return FLinearColor(0.7f, 0.9f, 1.0f); // Light blue
	case ESeason::AllSeasons:
		return FLinearColor::White;
	default:
		return FLinearColor::White;
	}
}
