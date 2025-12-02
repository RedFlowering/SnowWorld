// Copyright Epic Games, Inc. All Rights Reserved.

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
 * ?çÏÇ¨ ?úÏä§??Ïª¥Ìè¨?åÌä∏
 * ?ëÎ¨º ?¨Î∞∞, ?±Ïû• Í¥ÄÎ¶? ?òÌôï Ï≤òÎ¶¨
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
	// ?çÏÇ¨ Í∏∞Î≥∏ Í∏∞Îä•
	// ====================================

	/** ?ëÎ¨º ?¨Í∏∞ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool PlantCrop(FName CropID, FName PlotID);

	/** ?ëÎ¨º ?òÌôï */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	FHarvestResult HarvestCrop(FName PlotID);

	/** ?ëÎ¨º??Î¨ºÏ£ºÍ∏?*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool WaterCrop(FName PlotID);

	/** ÎπÑÎ£å ?¨Ïö© */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool ApplyFertilizer(FName PlotID, const FFertilizerData& Fertilizer);

	/** ?ëÎ¨º ?úÍ±∞ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveCrop(FName PlotID);

	/** ?ëÎ¨º ?ïÎ≥¥ Í∞Ä?∏Ïò§Í∏?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool GetCropInstance(FName PlotID, FCropInstance& OutCrop) const;

	/** ?òÌôï Í∞Ä???¨Î? ?ïÏù∏ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanHarvest(FName PlotID) const;

	// ====================================
	// Î∞?Í¥ÄÎ¶?
	// ====================================

	/** Î∞?Ï∂îÍ? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmPlot(FName PlotID, const FSoilData& SoilData);

	/** Î∞??úÍ±∞ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveFarmPlot(FName PlotID);

	/** Î™®Îì† Î∞?Í∞Ä?∏Ïò§Í∏?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetAllPlots() const;

	/** Îπ?Î∞?Í∞Ä?∏Ïò§Í∏?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetEmptyPlots() const;

	// ====================================
	// ?àÎ≤® Î∞?Í≤ΩÌóòÏπ??úÏä§??
	// ====================================

	/** ?çÏÇ¨ Í≤ΩÌóòÏπ??çÎìù */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmingExperience(int32 Amount);

	/** ?ÑÏû¨ ?çÏÇ¨ ?àÎ≤® */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	int32 GetFarmingLevel() const { return FarmingLevel; }

	// ====================================
	// Í≥ÑÏ†à ?úÏä§??
	// ====================================

	/** ?ÑÏû¨ Í≥ÑÏ†à ?§Ï†ï */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void SetCurrentSeason(ESeason Season);

	/** ?ÑÏû¨ Í≥ÑÏ†à Í∞Ä?∏Ïò§Í∏?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	ESeason GetCurrentSeason() const { return CurrentSeason; }

	/** ?ëÎ¨º???ÑÏû¨ Í≥ÑÏ†à???êÎûÑ ???àÎäîÏßÄ ?ïÏù∏ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanGrowInCurrentSeason(FName CropID) const;

	// ====================================
	// ?¥Î≤§??
	// ====================================

	/** ?ëÎ¨º ?¨Í∏∞ ?¥Î≤§??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropPlanted OnCropPlanted;

	/** ?ëÎ¨º ?±Ïû• ?®Í≥Ñ Î≥ÄÍ≤??¥Î≤§??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropGrowthStageChanged OnCropGrowthStageChanged;

	/** ?ëÎ¨º ?òÌôï ?¥Î≤§??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropHarvested OnCropHarvested;

	/** ?ëÎ¨º Î¨ºÏ£ºÍ∏??¥Î≤§??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropWatered OnCropWatered;

	/** ÎπÑÎ£å ?¨Ïö© ?¥Î≤§??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFertilizerApplied OnFertilizerApplied;

	/** ?àÎ≤®???¥Î≤§??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFarmingLevelUp OnFarmingLevelUp;

	// ====================================
	// ?§Ï†ï
	// ====================================

	/** ?ëÎ¨º ?∞Ïù¥?∞Î≤†?¥Ïä§ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	TMap<FName, FCropData> CropDatabase;

	/** Î¨ºÏ£ºÍ∏??òÎ∂Ñ Ï¶ùÍ???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float WaterMoistureIncrease = 30.0f;

	/** ?úÍ∞Ñ???òÎ∂Ñ Í∞êÏÜå??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float MoistureDecayPerHour = 10.0f;

	/** Î¨?Î∂ÄÏ°??òÎÑê??(?±Ïû• ?çÎèÑ Í∞êÏÜå, %) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float LowMoisturePenalty = 50.0f;

	/** ?®Ïïó ?çÎìù ?ïÎ•† (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float SeedDropChance = 30.0f;

private:
	/** ?çÏÇ¨ ?àÎ≤® */
	UPROPERTY()
	int32 FarmingLevel = 1;

	/** Î∞?Î™©Î°ù (?åÎ°Ø ID -> ?ëÎ¨º ?∏Ïä§?¥Ïä§) */
	UPROPERTY()
	TMap<FName, FCropInstance> FarmPlots;

	/** Î∞??†Ïñë ?∞Ïù¥??*/
	UPROPERTY()
	TMap<FName, FSoilData> PlotSoilData;

	/** ?ÑÏû¨ Í≥ÑÏ†à */
	UPROPERTY()
	ESeason CurrentSeason = ESeason::Spring;

	/** ÎßàÏ?Îß??ÖÎç∞?¥Ìä∏ ?úÍ∞Ñ */
	UPROPERTY()
	FDateTime LastUpdateTime;

	/** ?ëÎ¨º ?±Ïû• ?ÖÎç∞?¥Ìä∏ */
	void UpdateCropGrowth(FName PlotID, FCropInstance& Crop);

	/** ?ëÎ¨º ?àÏßà Í≥ÑÏÇ∞ */
	float CalculateCropQuality(const FCropInstance& Crop, const FCropData& CropData) const;

	/** ?òÌôï Í≤∞Í≥º Í≥ÑÏÇ∞ */
	FHarvestResult CalculateHarvestResult(const FCropInstance& Crop, const FCropData& CropData);

	/** ?àÎ≤® Ï≤¥ÌÅ¨ Î∞?Ï≤òÎ¶¨ */
	void CheckAndProcessLevelUp();

	/** ?ëÎ¨º???±Ïû• Í∞Ä?•ÌïúÏßÄ ?ïÏù∏ */
	bool CanCropGrow(const FCropInstance& Crop, const FCropData& CropData) const;
};
