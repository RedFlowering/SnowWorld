// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFishingSystemDefinitions.generated.h"

/**
 * ë¬¼ê³ ê¸??¬ê???
 */
UENUM(BlueprintType)
enum class EFishRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * ?šì‹œ ë¯¸ë‹ˆê²Œì„ ?€??
 */
UENUM(BlueprintType)
enum class EFishingMinigameType : uint8
{
	TimingBased		UMETA(DisplayName = "Timing Based"),		// ?€?´ë° ë§ì¶”ê¸?
	BarBalance		UMETA(DisplayName = "Bar Balance"),		// ë°?ê· í˜• ? ì?
	QuickTimeEvent	UMETA(DisplayName = "Quick Time Event"),	// QTE ?´ë²¤??
	ReelTension		UMETA(DisplayName = "Reel Tension")		// ë¦??¥ë ¥ ê´€ë¦?
};

/**
 * ?šì‹œ???€??
 */
UENUM(BlueprintType)
enum class EFishingSpotType : uint8
{
	River		UMETA(DisplayName = "River"),
	Lake		UMETA(DisplayName = "Lake"),
	Ocean		UMETA(DisplayName = "Ocean"),
	Pond		UMETA(DisplayName = "Pond"),
	Special		UMETA(DisplayName = "Special")		// ?¹ë³„???šì‹œ??
};

/**
 * ë¬¼ê³ ê¸??°ì´??
 */
USTRUCT(BlueprintType)
struct FFishData
{
	GENERATED_BODY()

	/** ë¬¼ê³ ê¸??´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishName;

	/** ë¬¼ê³ ê¸??¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** ?¬ê???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** ìµœì†Œ ?¬ê¸° (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinSize = 10.0f;

	/** ìµœë? ?¬ê¸° (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxSize = 50.0f;

	/** ìµœì†Œ ë¬´ê²Œ (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinWeight = 0.5f;

	/** ìµœë? ë¬´ê²Œ (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxWeight = 5.0f;

	/** ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 BasePrice = 10;

	/** ë¬¼ê³ ê¸?ë©”ì‹œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UStaticMesh> FishMesh;

	/** ë¬¼ê³ ê¸??„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ì¶œí˜„ ?œê°„ (24?œê°„ ê¸°ì?, -1?´ë©´ ??ƒ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearStartHour = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearEndHour = -1;

	/** ì¶œí˜„ ? ì”¨ (ë¹„ì–´?ˆìœ¼ë©?ëª¨ë“  ? ì”¨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredWeather;

	/** ì¶œí˜„ ê³„ì ˆ (ë¹„ì–´?ˆìœ¼ë©?ëª¨ë“  ê³„ì ˆ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredSeasons;
};

/**
 * ?šì‹œ???¤í° ?Œì´ë¸?
 */
USTRUCT(BlueprintType)
struct FFishingSpotSpawnEntry
{
	GENERATED_BODY()

	/** ë¬¼ê³ ê¸?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName FishID;

	/** ?¤í° ?•ë¥  (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnChance = 10.0f;

	/** ìµœì†Œ ?ˆë²¨ ?”êµ¬?¬í•­ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MinFishingLevel = 1;
};

/**
 * ?šì‹œ ë¯¸ë‹ˆê²Œì„ ?¤ì •
 */
USTRUCT(BlueprintType)
struct FFishingMinigameSettings
{
	GENERATED_BODY()

	/** ë¯¸ë‹ˆê²Œì„ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	EFishingMinigameType MinigameType = EFishingMinigameType::TimingBased;

	/** ?œì´??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	int32 Difficulty = 5;

	/** ?œí•œ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float TimeLimit = 30.0f;

	/** ?±ê³µ ë³´ë„ˆ??ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float SuccessBonus = 1.2f;

	/** ?„ë²½ ?±ê³µ ë³´ë„ˆ??ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float PerfectBonus = 1.5f;
};

/**
 * ?šì‹œ???°ì´???ì…‹
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UFishingSpotData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** ?šì‹œ???´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FText SpotName;

	/** ?šì‹œ???€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	EFishingSpotType SpotType = EFishingSpotType::River;

	/** ?¤í° ?Œì´ë¸?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	TArray<FFishingSpotSpawnEntry> SpawnTable;

	/** ìµœì†Œ ?ˆë²¨ ?”êµ¬?¬í•­ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	int32 MinimumFishingLevel = 1;

	/** ë¯¸ë‹ˆê²Œì„ ?¤ì • */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FFishingMinigameSettings MinigameSettings;
};

/**
 * ?¡ì? ë¬¼ê³ ê¸??•ë³´
 */
USTRUCT(BlueprintType)
struct FCaughtFish
{
	GENERATED_BODY()

	/** ë¬¼ê³ ê¸?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FName FishID;

	/** ?¬ê¸° */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Size = 0.0f;

	/** ë¬´ê²Œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Weight = 0.0f;

	/** ?¡ì? ?œê°„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FDateTime CaughtTime;

	/** ?¡ì? ?„ì¹˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FVector CaughtLocation;

	/** ?ˆì§ˆ ?ìˆ˜ (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float QualityScore = 50.0f;
};

/**
 * ë¬¼ê³ ê¸??°ì´???Œì´ë¸?Row
 * ?ë””?°ì—??DataTableë¡?ë¬¼ê³ ê¸??°ì´?°ë? ê´€ë¦¬í•  ???¬ìš©
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FFishDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ë¬¼ê³ ê¸?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishID;

	/** ?œì‹œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText DisplayName;

	/** ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** ?¬ê???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** ìµœì†Œ ?¬ê¸° (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinSize = 10.0f;

	/** ìµœë? ?¬ê¸° (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxSize = 50.0f;

	/** ìµœì†Œ ë¬´ê²Œ (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinWeight = 0.5f;

	/** ìµœë? ë¬´ê²Œ (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxWeight = 5.0f;

	/** ê¸°ë³¸ ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 BasePrice = 10;

	/** ?šì•˜?????ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 ExperienceGain = 5;

	/** ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 3D ë©”ì‹œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** ì¶œí˜„ ê°€?¥í•œ ?šì‹œ???€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<EFishingSpotType> ValidSpotTypes;

	/** ìµœì†Œ ?šì‹œ ?ˆë²¨ ?”êµ¬?¬í•­ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 MinFishingLevel = 1;

	/** ì¶œí˜„ ?œì‘ ?œê°„ (0-23, -1?´ë©´ ??ƒ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearStartHour = -1;

	/** ì¶œí˜„ ì¢…ë£Œ ?œê°„ (0-23, -1?´ë©´ ??ƒ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearEndHour = -1;

	/** ?„ìš” ? ì”¨ ì¡°ê±´ (ë¹„ì–´?ˆìœ¼ë©?ëª¨ë“  ? ì”¨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredWeather;

	/** ?„ìš” ê³„ì ˆ (ë¹„ì–´?ˆìœ¼ë©?ëª¨ë“  ê³„ì ˆ) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredSeasons;
};
