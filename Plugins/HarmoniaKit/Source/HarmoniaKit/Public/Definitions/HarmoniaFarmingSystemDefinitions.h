// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFarmingSystemDefinitions.generated.h"

/**
 * ?‘ë¬¼ ?€??
 */
UENUM(BlueprintType)
enum class ECropType : uint8
{
	Vegetable	UMETA(DisplayName = "Vegetable"),	// ì±„ì†Œ
	Fruit		UMETA(DisplayName = "Fruit"),		// ê³¼ì¼
	Grain		UMETA(DisplayName = "Grain"),		// ê³¡ë¬¼
	Herb		UMETA(DisplayName = "Herb"),		// ?½ì´ˆ
	Flower		UMETA(DisplayName = "Flower"),		// ê½?
	Tree		UMETA(DisplayName = "Tree")			// ?˜ë¬´
};

/**
 * ?‘ë¬¼ ?±ì¥ ?¨ê³„
 */
UENUM(BlueprintType)
enum class ECropGrowthStage : uint8
{
	Seed		UMETA(DisplayName = "Seed"),		// ?¨ì•—
	Sprout		UMETA(DisplayName = "Sprout"),		// ??
	Growing		UMETA(DisplayName = "Growing"),		// ?±ì¥
	Mature		UMETA(DisplayName = "Mature"),		// ?±ìˆ™
	Harvest		UMETA(DisplayName = "Harvest"),		// ?˜í™• ê°€??
	Withered	UMETA(DisplayName = "Withered")		// ?œë“¤??
};

/**
 * ? ì–‘ ?ˆì§ˆ
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
 * ê³„ì ˆ
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
 * ?‘ë¬¼ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FCropData
{
	GENERATED_BODY()

	/** ?‘ë¬¼ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropName;

	/** ?‘ë¬¼ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** ?‘ë¬¼ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** ?±ì¥ ê°€?¥í•œ ê³„ì ˆ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TArray<ESeason> GrowingSeasons;

	/** ???¨ê³„ ?±ì¥ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float SproutTime = 300.0f;

	/** ?±ì¥ ?¨ê³„ ?±ì¥ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowingTime = 600.0f;

	/** ?±ìˆ™ ?¨ê³„ ?±ì¥ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float MaturityTime = 900.0f;

	/** ?˜í™• ê°€???œê°„ (ì´? ???œê°„??ì§€?˜ë©´ ?œë“¦) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float HarvestWindow = 600.0f;

	/** ìµœì†Œ ?˜í™•??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MinYield = 1;

	/** ìµœë? ?˜í™•??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 MaxYield = 3;

	/** ?¬ìˆ˜??ê°€???¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	bool bReharvestablle = false;

	/** ?¬ìˆ˜???œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float ReharvestTime = 300.0f;

	/** ?¨ì•— ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 SeedPrice = 10;

	/** ?‘ë¬¼ ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 CropPrice = 30;

	/** ê²½í—˜ì¹?ë³´ìƒ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 ExperienceReward = 15;

	/** ?‘ë¬¼ ë©”ì‹œ (ê°??±ì¥ ?¨ê³„ë³? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TMap<ECropGrowthStage, TSoftObjectPtr<UStaticMesh>> StageMeshes;

	/** ?‘ë¬¼ ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * ? ì–‘ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FSoilData
{
	GENERATED_BODY()

	/** ? ì–‘ ?ˆì§ˆ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	ESoilQuality Quality = ESoilQuality::Normal;

	/** ?˜ë¶„ ?ˆë²¨ (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float MoistureLevel = 50.0f;

	/** ë¹„ì˜¥??(0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilityLevel = 50.0f;

	/** ë¹„ë£Œ ?¬ìš© ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	bool bFertilized = false;

	/** ë¹„ë£Œ ?¨ê³¼ ì§€???œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float FertilizerDuration = 0.0f;

	/** ?±ì¥ ?ë„ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float GrowthSpeedBonus = 0.0f;

	/** ?˜í™•??ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Soil")
	float YieldBonus = 0.0f;
};

/**
 * ?‘ë¬¼ ?¸ìŠ¤?´ìŠ¤
 */
USTRUCT(BlueprintType)
struct FCropInstance
{
	GENERATED_BODY()

	/** ?‘ë¬¼ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** ?„ì¬ ?±ì¥ ?¨ê³„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropGrowthStage CurrentStage = ECropGrowthStage::Seed;

	/** ?±ì¥ ?œì‘ ?œê°„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime PlantedTime;

	/** ?„ì¬ ?¨ê³„ ì§„ì… ?œê°„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime StageStartTime;

	/** ë§ˆì?ë§?ë¬¼ì£¼ê¸??œê°„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FDateTime LastWateredTime;

	/** ? ì–‘ ?°ì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FSoilData Soil;

	/** ?˜í™• ?Ÿìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	int32 HarvestCount = 0;

	/** ?ˆì§ˆ ?ìˆ˜ (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	float QualityScore = 50.0f;
};

/**
 * ë¹„ë£Œ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FFertilizerData
{
	GENERATED_BODY()

	/** ë¹„ë£Œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FName FertilizerName;

	/** ë¹„ë£Œ ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	FText Description;

	/** ?±ì¥ ?ë„ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float GrowthSpeedBonus = 20.0f;

	/** ?˜í™•??ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float YieldBonus = 10.0f;

	/** ?ˆì§ˆ ?¥ìƒ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float QualityBonus = 15.0f;

	/** ì§€???œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	float Duration = 1800.0f;

	/** ë¹„ë£Œ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fertilizer")
	int32 Price = 50;
};

/**
 * ?˜í™• ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FHarvestResult
{
	GENERATED_BODY()

	/** ?˜í™•???‘ë¬¼ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName CropID;

	/** ?˜í™•??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** ?ˆì§ˆ ?ìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float Quality = 50.0f;

	/** ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ë³´ë„ˆ???„ì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;

	/** ?¨ì•— ?ë“ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bGotSeeds = false;

	/** ?ë“???¨ì•— ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 SeedCount = 0;
};

/**
 * ?‘ë¬¼ ?°ì´???Œì´ë¸?Row
 * ?ë””?°ì—??DataTableë¡??‘ë¬¼??ê´€ë¦¬í•  ???¬ìš©
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FCropDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?‘ë¬¼ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FName CropID;

	/** ?œì‹œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText DisplayName;

	/** ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	FText Description;

	/** ?‘ë¬¼ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop")
	ECropType CropType = ECropType::Vegetable;

	/** ?„ìš” ê³„ì ˆ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	ESeason RequiredSeason = ESeason::AllSeasons;

	/** ?±ì¥ ?œê°„ (ê²Œì„ ?œê°„ ê¸°ì?, ?œê°„) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float GrowthTimeHours = 24.0f;

	/** ?œê°„??ë¬??Œëª¨??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Growth")
	float WaterConsumptionRate = 0.05f;

	/** ìµœì†Œ ?ì‚¬ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Requirement")
	int32 MinFarmingLevel = 1;

	/** ìµœì†Œ ?˜í™•??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MinYield = 1;

	/** ìµœë? ?˜í™•??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxYield = 3;

	/** ?¬ìˆ˜??ê°€???¬ë? (?¤ì‹œ ?¬ì? ?Šì•„???? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	bool bReharvestable = false;

	/** ?¬ìˆ˜?•ê¹Œì§€ ê±¸ë¦¬???œê°„ (?¬ìˆ˜??ê°€?¥í•  ê²½ìš°) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float RegrowthTimeHours = 12.0f;

	/** ìµœë? ?˜í™• ?Ÿìˆ˜ (0 = ë¬´ì œ?? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	int32 MaxHarvests = 0;

	/** ?˜í™• ???¨ì•— ?ë“ ?•ë¥  (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Harvest")
	float SeedDropChance = 0.3f;

	/** ?˜í™•???ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 ExperienceGain = 5;

	/** ?¨ì•— ?„ì´??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName SeedItemID;

	/** ?˜í™• ?„ì´??ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Item")
	FName HarvestedItemID;

	/** ê¸°ë³¸ ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Economy")
	int32 BasePrice = 10;

	/** ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crop|Visual")
	TSoftObjectPtr<UTexture2D> Icon;
};
