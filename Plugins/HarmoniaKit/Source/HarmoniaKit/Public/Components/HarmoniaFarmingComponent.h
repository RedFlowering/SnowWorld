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
 * 농사 시스템 컴포넌트
 * 작물 재배, 성장 관리, 수확 처리
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
	// 농사 기본 기능
	// ====================================

	/** 작물 심기 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool PlantCrop(FName CropID, FName PlotID);

	/** 작물 수확 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	FHarvestResult HarvestCrop(FName PlotID);

	/** 작물에 물주기 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool WaterCrop(FName PlotID);

	/** 비료 사용 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	bool ApplyFertilizer(FName PlotID, const FFertilizerData& Fertilizer);

	/** 작물 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveCrop(FName PlotID);

	/** 작물 정보 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool GetCropInstance(FName PlotID, FCropInstance& OutCrop) const;

	/** 수확 가능 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanHarvest(FName PlotID) const;

	// ====================================
	// 밭 관리
	// ====================================

	/** 밭 추가 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmPlot(FName PlotID, const FSoilData& SoilData);

	/** 밭 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void RemoveFarmPlot(FName PlotID);

	/** 모든 밭 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetAllPlots() const;

	/** 빈 밭 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	TArray<FName> GetEmptyPlots() const;

	// ====================================
	// 레벨 및 경험치 시스템
	// ====================================

	/** 농사 경험치 획득 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void AddFarmingExperience(int32 Amount);

	/** 현재 농사 레벨 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	int32 GetFarmingLevel() const { return FarmingLevel; }

	/** 현재 경험치 */
	virtual int32 GetCurrentExperience() const override { return CurrentExperience; }

	/** 다음 레벨까지 필요한 경험치 */
	virtual int32 GetExperienceForNextLevel() const override;

	// ====================================
	// 계절 시스템
	// ====================================

	/** 현재 계절 설정 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Farming")
	void SetCurrentSeason(ESeason Season);

	/** 현재 계절 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	ESeason GetCurrentSeason() const { return CurrentSeason; }

	/** 작물이 현재 계절에 자랄 수 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Farming")
	bool CanGrowInCurrentSeason(FName CropID) const;

	// ====================================
	// 이벤트
	// ====================================

	/** 작물 심기 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropPlanted OnCropPlanted;

	/** 작물 성장 단계 변경 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropGrowthStageChanged OnCropGrowthStageChanged;

	/** 작물 수확 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropHarvested OnCropHarvested;

	/** 작물 물주기 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnCropWatered OnCropWatered;

	/** 비료 사용 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFertilizerApplied OnFertilizerApplied;

	/** 레벨업 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Farming")
	FOnFarmingLevelUp OnFarmingLevelUp;

	// ====================================
	// 설정
	// ====================================

	/** 작물 데이터베이스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	TMap<FName, FCropData> CropDatabase;

	/** 물주기 수분 증가량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float WaterMoistureIncrease = 30.0f;

	/** 시간당 수분 감소율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float MoistureDecayPerHour = 10.0f;

	/** 물 부족 페널티 (성장 속도 감소, %) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float LowMoisturePenalty = 50.0f;

	/** 씨앗 획득 확률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farming Settings")
	float SeedDropChance = 30.0f;

private:
	/** 농사 레벨 */
	UPROPERTY()
	int32 FarmingLevel = 1;

	/** 밭 목록 (플롯 ID -> 작물 인스턴스) */
	UPROPERTY()
	TMap<FName, FCropInstance> FarmPlots;

	/** 밭 토양 데이터 */
	UPROPERTY()
	TMap<FName, FSoilData> PlotSoilData;

	/** 현재 계절 */
	UPROPERTY()
	ESeason CurrentSeason = ESeason::Spring;

	/** 마지막 업데이트 시간 */
	UPROPERTY()
	FDateTime LastUpdateTime;

	/** 작물 성장 업데이트 */
	void UpdateCropGrowth(FName PlotID, FCropInstance& Crop);

	/** 작물 품질 계산 */
	float CalculateCropQuality(const FCropInstance& Crop, const FCropData& CropData) const;

	/** 수확 결과 계산 */
	FHarvestResult CalculateHarvestResult(const FCropInstance& Crop, const FCropData& CropData);

	/** 레벨 체크 및 처리 */
	void CheckAndProcessLevelUp();

	/** 작물이 성장 가능한지 확인 */
	bool CanCropGrow(const FCropInstance& Crop, const FCropData& CropData) const;
};
