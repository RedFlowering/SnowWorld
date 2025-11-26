// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UI/LifeContent/HarmoniaLifeContentWidget.h"
#include "Definitions/HarmoniaFarmingSystemDefinitions.h"
#include "HarmoniaFarmingWidget.generated.h"

class UHarmoniaFarmingComponent;
class UImage;
class UTextBlock;
class UProgressBar;
class UUniformGridPanel;

/**
 * Farming UI Widget
 * Displays farm plot status, crop growth, and harvest information
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaFarmingWidget : public UHarmoniaLifeContentWidget
{
	GENERATED_BODY()

public:
	UHarmoniaFarmingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ====================================
	// Setup
	// ====================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void SetFarmingComponent(UHarmoniaFarmingComponent* InComponent);

	virtual void RefreshDisplay() override;

	// ====================================
	// Plot Display
	// ====================================

	/** Update farm plot display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void UpdatePlotDisplay(FName PlotID, bool bHasCrop, float GrowthProgress, float MoistureLevel);

	/** Show plot details */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void ShowPlotDetails(FName PlotID, ECropGrowthStage GrowthStage, float Moisture, 
						 ESoilQuality SoilQuality, bool bHasFertilizer);

	// ====================================
	// Crop Display
	// ====================================

	/** Show crop info when planted */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void ShowCropInfo(const FText& CropName, ECropType CropType, ESeason RequiredSeason, 
					  float GrowthTime, bool bReharvestable);

	/** Hide crop info */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void HideCropInfo();

	// ====================================
	// Harvest Display
	// ====================================

	/** Show harvest result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void ShowHarvestResult(const FText& CropName, int32 Yield, float QualityScore, 
						   bool bGotSeeds, int32 SeedAmount);

	/** Hide harvest result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void HideHarvestResult();

	// ====================================
	// Status Indicators
	// ====================================

	/** Update current season display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void UpdateSeasonDisplay(ESeason CurrentSeason);

	/** Update weather status */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Farming")
	void UpdateWeatherStatus(bool bIsRaining);

protected:
	// ====================================
	// Plot Status Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> PlotGridPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|UI|Farming")
	TSubclassOf<UUserWidget> PlotSlotWidgetClass;

	// ====================================
	// Plot Details Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PlotDetailsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlotIDText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GrowthStageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> MoistureBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MoistureText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SoilQualityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FertilizerStatusText;

	// ====================================
	// Crop Info Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CropInfoPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CropIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CropNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CropTypeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CropSeasonText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CropGrowthTimeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CropReharvestText;

	// ====================================
	// Harvest Result Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> HarvestResultPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HarvestCropNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HarvestYieldText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HarvestQualityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HarvestSeedText;

	// ====================================
	// Status Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CurrentSeasonText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> WeatherIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeatherStatusText;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|Farming")
	TObjectPtr<UHarmoniaFarmingComponent> FarmingComponent;

private:
	FText GetGrowthStageText(ECropGrowthStage Stage) const;
	FText GetSoilQualityText(ESoilQuality Quality) const;
	FLinearColor GetSoilQualityColor(ESoilQuality Quality) const;
	FText GetCropTypeText(ECropType Type) const;
	FText GetSeasonText(ESeason Season) const;
	FLinearColor GetSeasonColor(ESeason Season) const;
};
