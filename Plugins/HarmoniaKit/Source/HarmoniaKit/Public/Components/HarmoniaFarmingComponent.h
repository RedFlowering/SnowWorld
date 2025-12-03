// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaFarmingComponent.h
 * @brief Farming system component for crop cultivation and harvest management
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "Definitions/HarmoniaFarmingSystemDefinitions.h"
#include "HarmoniaFarmingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCropPlanted, FName, CropID, const FCropInstance&, CropInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCropGrowthStageChanged, FName, CropID, ECropGrowthStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCropHarvested, const FHarvestResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCropWatered, FName, CropID, float, MoistureLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFertilizerApplied, FName, CropID, const FFertilizerData&, Fertilizer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFarmingLevelUp, int32, NewLevel, int32, SkillPoints);

/**
 * @class UHarmoniaFarmingComponent
 * @brief Farming system component for crop cultivation
 * 
 * Handles crop planting, growth management, and harvest processing.
 * Supports seasons, soil quality, and fertilizer systems.
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaFarmingComponent : public UHarmoniaBaseLifeContentComponent
{
	GENERATED_BODY()

public:	
	UHarmoniaFarmingComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// Farming Basic Functions
	// ====================================

	/** Plant a crop in the specified plot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool PlantCrop(FName CropID, FName PlotID);

	/** Harvest crop from plot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	FHarvestResult HarvestCrop(FName PlotID);

	/** Water crop in plot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool WaterCrop(FName PlotID);

	/** Apply fertilizer to crop */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool ApplyFertilizer(FName PlotID, const FFertilizerData& Fertilizer);

	/** Remove crop from plot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveCrop(FName PlotID);

	/** Get crop instance from plot */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool GetCropInstance(FName PlotID, FCropInstance& OutCrop) const;

	/** Check if crop can be harvested */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanHarvest(FName PlotID) const;

	// ====================================
	// Plot Management
	// ====================================

	/** Add farm plot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmPlot(FName PlotID, const FSoilData& SoilData);

	/** Remove farm plot */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveFarmPlot(FName PlotID);

	/** Get all plot IDs */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetAllPlots() const;

	/** Get empty (unoccupied) plots */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetEmptyPlots() const;

	// ====================================
	// Level & Experience System
	// ====================================

	/** Add farming experience */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmingExperience(int32 Amount);

	/** Get current farming level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	int32 GetFarmingLevel() const { return FarmingLevel; }

	// ====================================
	// Season System
	// ====================================
	// ====================================

	/** Set current season */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void SetCurrentSeason(ESeason Season);

	/** Get current season */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	ESeason GetCurrentSeason() const { return CurrentSeason; }

	/** Check if crop can grow in current season */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanGrowInCurrentSeason(FName CropID) const;

	// ====================================
	// Events
	// ====================================

	/** Event fired when crop is planted */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropPlanted OnCropPlanted;

	/** Event fired when crop growth stage changes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropGrowthStageChanged OnCropGrowthStageChanged;

	/** Event fired when crop is harvested */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropHarvested OnCropHarvested;

	/** Event fired when crop is watered */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropWatered OnCropWatered;

	/** Event fired when fertilizer is applied */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFertilizerApplied OnFertilizerApplied;

	/** Event fired on level up */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFarmingLevelUp OnFarmingLevelUp;

	// ====================================
	// Settings
	// ====================================

	/** Crop database */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	TMap<FName, FCropData> CropDatabase;

	/** Moisture increase amount per watering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float WaterMoistureIncrease = 30.0f;

	/** Moisture decay rate per hour */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float MoistureDecayPerHour = 10.0f;

	/** Low moisture penalty (growth speed reduction, %) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float LowMoisturePenalty = 50.0f;

	/** Seed drop chance (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float SeedDropChance = 30.0f;

private:
	/** Farming level */
	UPROPERTY()
	int32 FarmingLevel = 1;

	/** Farm plots map (Plot ID -> Crop Instance) */
	UPROPERTY()
	TMap<FName, FCropInstance> FarmPlots;

	/** Plot soil data map */
	UPROPERTY()
	TMap<FName, FSoilData> PlotSoilData;

	/** Current season */
	UPROPERTY()
	ESeason CurrentSeason = ESeason::Spring;

	/** Last update time */
	UPROPERTY()
	FDateTime LastUpdateTime;

	/** Update crop growth state */
	void UpdateCropGrowth(FName PlotID, FCropInstance& Crop);

	/** Calculate crop quality */
	float CalculateCropQuality(const FCropInstance& Crop, const FCropData& CropData) const;

	/** Calculate harvest result */
	FHarvestResult CalculateHarvestResult(const FCropInstance& Crop, const FCropData& CropData);

	/** Check and process level up */
	void CheckAndProcessLevelUp();

	/** Check if crop can grow */
	bool CanCropGrow(const FCropInstance& Crop, const FCropData& CropData) const;
};
