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
 * ?�사 ?�스??컴포?�트
 * ?�물 ?�배, ?�장 관�? ?�확 처리
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
	// ?�사 기본 기능
	// ====================================

	/** ?�물 ?�기 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool PlantCrop(FName CropID, FName PlotID);

	/** ?�물 ?�확 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	FHarvestResult HarvestCrop(FName PlotID);

	/** ?�물??물주�?*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool WaterCrop(FName PlotID);

	/** 비료 ?�용 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool ApplyFertilizer(FName PlotID, const FFertilizerData& Fertilizer);

	/** ?�물 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveCrop(FName PlotID);

	/** ?�물 ?�보 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool GetCropInstance(FName PlotID, FCropInstance& OutCrop) const;

	/** ?�확 가???��? ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanHarvest(FName PlotID) const;

	// ====================================
	// �?관�?
	// ====================================

	/** �?추�? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmPlot(FName PlotID, const FSoilData& SoilData);

	/** �??�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveFarmPlot(FName PlotID);

	/** 모든 �?가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetAllPlots() const;

	/** �?�?가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetEmptyPlots() const;

	// ====================================
	// ?�벨 �?경험�??�스??
	// ====================================

	/** ?�사 경험�??�득 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmingExperience(int32 Amount);

	/** ?�재 ?�사 ?�벨 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	int32 GetFarmingLevel() const { return FarmingLevel; }

	// ====================================
	// 계절 ?�스??
	// ====================================

	/** ?�재 계절 ?�정 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void SetCurrentSeason(ESeason Season);

	/** ?�재 계절 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	ESeason GetCurrentSeason() const { return CurrentSeason; }

	/** ?�물???�재 계절???�랄 ???�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanGrowInCurrentSeason(FName CropID) const;

	// ====================================
	// ?�벤??
	// ====================================

	/** ?�물 ?�기 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropPlanted OnCropPlanted;

	/** ?�물 ?�장 ?�계 변�??�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropGrowthStageChanged OnCropGrowthStageChanged;

	/** ?�물 ?�확 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropHarvested OnCropHarvested;

	/** ?�물 물주�??�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropWatered OnCropWatered;

	/** 비료 ?�용 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFertilizerApplied OnFertilizerApplied;

	/** ?�벨???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFarmingLevelUp OnFarmingLevelUp;

	// ====================================
	// ?�정
	// ====================================

	/** ?�물 ?�이?�베?�스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	TMap<FName, FCropData> CropDatabase;

	/** 물주�??�분 증�???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float WaterMoistureIncrease = 30.0f;

	/** ?�간???�분 감소??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float MoistureDecayPerHour = 10.0f;

	/** �?부�??�널??(?�장 ?�도 감소, %) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float LowMoisturePenalty = 50.0f;

	/** ?�앗 ?�득 ?�률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float SeedDropChance = 30.0f;

private:
	/** ?�사 ?�벨 */
	UPROPERTY()
	int32 FarmingLevel = 1;

	/** �?목록 (?�롯 ID -> ?�물 ?�스?�스) */
	UPROPERTY()
	TMap<FName, FCropInstance> FarmPlots;

	/** �??�양 ?�이??*/
	UPROPERTY()
	TMap<FName, FSoilData> PlotSoilData;

	/** ?�재 계절 */
	UPROPERTY()
	ESeason CurrentSeason = ESeason::Spring;

	/** 마�?�??�데?�트 ?�간 */
	UPROPERTY()
	FDateTime LastUpdateTime;

	/** ?�물 ?�장 ?�데?�트 */
	void UpdateCropGrowth(FName PlotID, FCropInstance& Crop);

	/** ?�물 ?�질 계산 */
	float CalculateCropQuality(const FCropInstance& Crop, const FCropData& CropData) const;

	/** ?�확 결과 계산 */
	FHarvestResult CalculateHarvestResult(const FCropInstance& Crop, const FCropData& CropData);

	/** ?�벨 체크 �?처리 */
	void CheckAndProcessLevelUp();

	/** ?�물???�장 가?�한지 ?�인 */
	bool CanCropGrow(const FCropInstance& Crop, const FCropData& CropData) const;
};
