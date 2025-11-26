// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFarmingSystemDefinitions.generated.h"

/**
 * 작물 타입
 */
UENUM(BlueprintType)
enum class ECropType : uint8
{
	Vegetable	UMETA(DisplayName = "Vegetable"),	// 채소
	Fruit		UMETA(DisplayName = "Fruit"),		// 과일
	Grain		UMETA(DisplayName = "Grain"),		// 곡물
	Herb		UMETA(DisplayName = "Herb"),		// 약초
	Flower		UMETA(DisplayName = "Flower"),		// 꽃
	Tree		UMETA(DisplayName = "Tree")			// 나무
};

/**
 * 작물 성장 단계
 */
UENUM(BlueprintType)
enum class ECropGrowthStage : uint8
{
	Seed		UMETA(DisplayName = "Seed"),		// 씨앗
	Sprout		UMETA(DisplayName = "Sprout"),		// 싹
	Growing		UMETA(DisplayName = "Growing"),		// 성장
	Mature		UMETA(DisplayName = "Mature"),		// 성숙
	Harvest		UMETA(DisplayName = "Harvest"),		// 수확 가능
	Withered	UMETA(DisplayName = "Withered")		// 시들음
};

/**
 * 토양 품질
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
 * 작물 데이터
 */
USTRUCT(BlueprintType)
struct FCropData
{
	GENERATED_BODY()

	/** 작물 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropName;

	/** 작물 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** 작물 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** 성장 가능한 계절 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TArray<ESeason> GrowingSeasons;

	/** 싹 단계 성장 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float SproutTime = 300.0f;

	/** 성장 단계 성장 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowingTime = 600.0f;

	/** 성숙 단계 성장 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float MaturityTime = 900.0f;

	/** 수확 가능 시간 (초, 이 시간이 지나면 시듦) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float HarvestWindow = 600.0f;

	/** 최소 수확량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MinYield = 1;

	/** 최대 수확량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MaxYield = 3;

	/** 재수확 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	bool bReharvestablle = false;

	/** 재수확 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float ReharvestTime = 300.0f;

	/** 씨앗 판매 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 SeedPrice = 10;

	/** 작물 판매 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 CropPrice = 30;

	/** 경험치 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 ExperienceReward = 15;

	/** 작물 메시 (각 성장 단계별) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TMap<ECropGrowthStage, TSoftObjectPtr<UStaticMesh>> StageMeshes;

	/** 작물 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * 토양 데이터
 */
USTRUCT(BlueprintType)
struct FSoilData
{
	GENERATED_BODY()

	/** 토양 품질 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	ESoilQuality Quality = ESoilQuality::Normal;

	/** 수분 레벨 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float MoistureLevel = 50.0f;

	/** 비옥도 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilityLevel = 50.0f;

	/** 비료 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	bool bFertilized = false;

	/** 비료 효과 지속 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilizerDuration = 0.0f;

	/** 성장 속도 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float GrowthSpeedBonus = 0.0f;

	/** 수확량 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float YieldBonus = 0.0f;
};

/**
 * 작물 인스턴스
 */
USTRUCT(BlueprintType)
struct FCropInstance
{
	GENERATED_BODY()

	/** 작물 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** 현재 성장 단계 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropGrowthStage CurrentStage = ECropGrowthStage::Seed;

	/** 성장 시작 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime PlantedTime;

	/** 현재 단계 진입 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime StageStartTime;

	/** 마지막 물주기 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime LastWateredTime;

	/** 토양 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FSoilData Soil;

	/** 수확 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 HarvestCount = 0;

	/** 품질 점수 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float QualityScore = 50.0f;
};

/**
 * 비료 데이터
 */
USTRUCT(BlueprintType)
struct FFertilizerData
{
	GENERATED_BODY()

	/** 비료 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FName FertilizerName;

	/** 비료 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FText Description;

	/** 성장 속도 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float GrowthSpeedBonus = 20.0f;

	/** 수확량 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float YieldBonus = 10.0f;

	/** 품질 향상 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float QualityBonus = 15.0f;

	/** 지속 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float Duration = 1800.0f;

	/** 비료 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	int32 Price = 50;
};

/**
 * 수확 결과
 */
USTRUCT(BlueprintType)
struct FHarvestResult
{
	GENERATED_BODY()

	/** 수확한 작물 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName CropID;

	/** 수확량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** 품질 점수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float Quality = 50.0f;

	/** 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** 보너스 아이템 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;

	/** 씨앗 획득 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bGotSeeds = false;

	/** 획득한 씨앗 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 SeedCount = 0;
};

/**
 * 작물 데이터 테이블 Row
 * 에디터에서 DataTable로 작물을 관리할 때 사용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FCropDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 작물 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** 작물 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** 필요 계절 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	ESeason RequiredSeason = ESeason::AllSeasons;

	/** 성장 시간 (게임 시간 기준, 시간) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowthTimeHours = 24.0f;

	/** 시간당 물 소모량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float WaterConsumptionRate = 0.05f;

	/** 최소 농사 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Requirement")
	int32 MinFarmingLevel = 1;

	/** 최소 수확량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MinYield = 1;

	/** 최대 수확량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxYield = 3;

	/** 재수확 가능 여부 (다시 심지 않아도 됨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	bool bReharvestable = false;

	/** 재수확까지 걸리는 시간 (재수확 가능할 경우) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float RegrowthTimeHours = 12.0f;

	/** 최대 수확 횟수 (0 = 무제한) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxHarvests = 0;

	/** 수확 시 씨앗 획득 확률 (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float SeedDropChance = 0.3f;

	/** 수확당 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 ExperienceGain = 5;

	/** 씨앗 아이템 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName SeedItemID;

	/** 수확 아이템 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName HarvestedItemID;

	/** 기본 판매 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 BasePrice = 10;

	/** 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TSoftObjectPtr<UTexture2D> Icon;
};
