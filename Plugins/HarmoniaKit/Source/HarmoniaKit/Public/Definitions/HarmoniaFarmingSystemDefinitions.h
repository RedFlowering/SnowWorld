// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFarmingSystemDefinitions.generated.h"

/**
 * ?�물 ?�??
 */
UENUM(BlueprintType)
enum class ECropType : uint8
{
	Vegetable	UMETA(DisplayName = "Vegetable"),	// 채소
	Fruit		UMETA(DisplayName = "Fruit"),		// 과일
	Grain		UMETA(DisplayName = "Grain"),		// 곡물
	Herb		UMETA(DisplayName = "Herb"),		// ?�초
	Flower		UMETA(DisplayName = "Flower"),		// �?
	Tree		UMETA(DisplayName = "Tree")			// ?�무
};

/**
 * ?�물 ?�장 ?�계
 */
UENUM(BlueprintType)
enum class ECropGrowthStage : uint8
{
	Seed		UMETA(DisplayName = "Seed"),		// ?�앗
	Sprout		UMETA(DisplayName = "Sprout"),		// ??
	Growing		UMETA(DisplayName = "Growing"),		// ?�장
	Mature		UMETA(DisplayName = "Mature"),		// ?�숙
	Harvest		UMETA(DisplayName = "Harvest"),		// ?�확 가??
	Withered	UMETA(DisplayName = "Withered")		// ?�들??
};

/**
 * ?�양 ?�질
 */
UENUM(BlueprintType)
enum class ESoilQuality : uint8
{
	Poor		UMETA(DisplayName = "Poor"),
	Normal		UMETA(DisplayName = "Normal"),
	Good		UMETA(DisplayName = "Good"),
	Excellent	UMETA(DisplayName = "Excellent"),
	Perfect		UMETA(DisplayName = "Perfect")
};

/**
 * 계절
 */
UENUM(BlueprintType)
enum class ESeason : uint8
{
	Spring		UMETA(DisplayName = "Spring"),
	Summer		UMETA(DisplayName = "Summer"),
	Autumn		UMETA(DisplayName = "Autumn"),
	Winter		UMETA(DisplayName = "Winter"),
	AllSeasons	UMETA(DisplayName = "All Seasons")
};

/**
 * ?�물 ?�이??
 */
USTRUCT(BlueprintType)
struct FCropData
{
	GENERATED_BODY()

	/** ?�물 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropName;

	/** ?�물 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** ?�물 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** ?�장 가?�한 계절 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TArray<ESeason> GrowingSeasons;

	/** ???�계 ?�장 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float SproutTime = 300.0f;

	/** ?�장 ?�계 ?�장 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowingTime = 600.0f;

	/** ?�숙 ?�계 ?�장 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float MaturityTime = 900.0f;

	/** ?�확 가???�간 (�? ???�간??지?�면 ?�듦) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float HarvestWindow = 600.0f;

	/** 최소 ?�확??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MinYield = 1;

	/** 최�? ?�확??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MaxYield = 3;

	/** ?�수??가???��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	bool bReharvestablle = false;

	/** ?�수???�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float ReharvestTime = 300.0f;

	/** ?�앗 ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 SeedPrice = 10;

	/** ?�물 ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 CropPrice = 30;

	/** 경험�?보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 ExperienceReward = 15;

	/** ?�물 메시 (�??�장 ?�계�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TMap<ECropGrowthStage, TSoftObjectPtr<UStaticMesh>> StageMeshes;

	/** ?�물 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * ?�양 ?�이??
 */
USTRUCT(BlueprintType)
struct FSoilData
{
	GENERATED_BODY()

	/** ?�양 ?�질 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	ESoilQuality Quality = ESoilQuality::Normal;

	/** ?�분 ?�벨 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float MoistureLevel = 50.0f;

	/** 비옥??(0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilityLevel = 50.0f;

	/** 비료 ?�용 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	bool bFertilized = false;

	/** 비료 ?�과 지???�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilizerDuration = 0.0f;

	/** ?�장 ?�도 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float GrowthSpeedBonus = 0.0f;

	/** ?�확??보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float YieldBonus = 0.0f;
};

/**
 * ?�물 ?�스?�스
 */
USTRUCT(BlueprintType)
struct FCropInstance
{
	GENERATED_BODY()

	/** ?�물 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** ?�재 ?�장 ?�계 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropGrowthStage CurrentStage = ECropGrowthStage::Seed;

	/** ?�장 ?�작 ?�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime PlantedTime;

	/** ?�재 ?�계 진입 ?�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime StageStartTime;

	/** 마�?�?물주�??�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime LastWateredTime;

	/** ?�양 ?�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FSoilData Soil;

	/** ?�확 ?�수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 HarvestCount = 0;

	/** ?�질 ?�수 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float QualityScore = 50.0f;
};

/**
 * 비료 ?�이??
 */
USTRUCT(BlueprintType)
struct FFertilizerData
{
	GENERATED_BODY()

	/** 비료 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FName FertilizerName;

	/** 비료 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FText Description;

	/** ?�장 ?�도 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float GrowthSpeedBonus = 20.0f;

	/** ?�확??보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float YieldBonus = 10.0f;

	/** ?�질 ?�상 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float QualityBonus = 15.0f;

	/** 지???�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float Duration = 1800.0f;

	/** 비료 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	int32 Price = 50;
};

/**
 * ?�확 결과
 */
USTRUCT(BlueprintType)
struct FHarvestResult
{
	GENERATED_BODY()

	/** ?�확???�물 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName CropID;

	/** ?�확??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** ?�질 ?�수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float Quality = 50.0f;

	/** 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** 보너???�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;

	/** ?�앗 ?�득 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bGotSeeds = false;

	/** ?�득???�앗 ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 SeedCount = 0;
};

/**
 * ?�물 ?�이???�이�?Row
 * ?�디?�에??DataTable�??�물??관리할 ???�용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FCropDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?�물 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** ?�시 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText DisplayName;

	/** ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** ?�물 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** ?�요 계절 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	ESeason RequiredSeason = ESeason::AllSeasons;

	/** ?�장 ?�간 (게임 ?�간 기�?, ?�간) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowthTimeHours = 24.0f;

	/** ?�간??�??�모??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float WaterConsumptionRate = 0.05f;

	/** 최소 ?�사 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Requirement")
	int32 MinFarmingLevel = 1;

	/** 최소 ?�확??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MinYield = 1;

	/** 최�? ?�확??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxYield = 3;

	/** ?�수??가???��? (?�시 ?��? ?�아???? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	bool bReharvestable = false;

	/** ?�수?�까지 걸리???�간 (?�수??가?�할 경우) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float RegrowthTimeHours = 12.0f;

	/** 최�? ?�확 ?�수 (0 = 무제?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxHarvests = 0;

	/** ?�확 ???�앗 ?�득 ?�률 (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float SeedDropChance = 0.3f;

	/** ?�확???�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 ExperienceGain = 5;

	/** ?�앗 ?�이??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName SeedItemID;

	/** ?�확 ?�이??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName HarvestedItemID;

	/** 기본 ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 BasePrice = 10;

	/** ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TSoftObjectPtr<UTexture2D> Icon;
};
