// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFishingSystemDefinitions.generated.h"

/**
 * 물고�??��???
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
 * ?�시 미니게임 ?�??
 */
UENUM(BlueprintType)
enum class EFishingMinigameType : uint8
{
	TimingBased		UMETA(DisplayName = "Timing Based"),		// ?�?�밍 맞추�?
	BarBalance		UMETA(DisplayName = "Bar Balance"),		// �?균형 ?��?
	QuickTimeEvent	UMETA(DisplayName = "Quick Time Event"),	// QTE ?�벤??
	ReelTension		UMETA(DisplayName = "Reel Tension")		// �??�력 관�?
};

/**
 * ?�시???�??
 */
UENUM(BlueprintType)
enum class EFishingSpotType : uint8
{
	River		UMETA(DisplayName = "River"),
	Lake		UMETA(DisplayName = "Lake"),
	Ocean		UMETA(DisplayName = "Ocean"),
	Pond		UMETA(DisplayName = "Pond"),
	Special		UMETA(DisplayName = "Special")		// ?�별???�시??
};

/**
 * 물고�??�이??
 */
USTRUCT(BlueprintType)
struct FFishData
{
	GENERATED_BODY()

	/** 물고�??�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishName;

	/** 물고�??�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** ?��???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** 최소 ?�기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinSize = 10.0f;

	/** 최�? ?�기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxSize = 50.0f;

	/** 최소 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinWeight = 0.5f;

	/** 최�? 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxWeight = 5.0f;

	/** ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 BasePrice = 10;

	/** 물고�?메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UStaticMesh> FishMesh;

	/** 물고�??�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 출현 ?�간 (24?�간 기�?, -1?�면 ??��) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearStartHour = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearEndHour = -1;

	/** 출현 ?�씨 (비어?�으�?모든 ?�씨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredWeather;

	/** 출현 계절 (비어?�으�?모든 계절) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredSeasons;
};

/**
 * ?�시???�폰 ?�이�?
 */
USTRUCT(BlueprintType)
struct FFishingSpotSpawnEntry
{
	GENERATED_BODY()

	/** 물고�?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName FishID;

	/** ?�폰 ?�률 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnChance = 10.0f;

	/** 최소 ?�벨 ?�구?�항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MinFishingLevel = 1;
};

/**
 * ?�시 미니게임 ?�정
 */
USTRUCT(BlueprintType)
struct FFishingMinigameSettings
{
	GENERATED_BODY()

	/** 미니게임 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	EFishingMinigameType MinigameType = EFishingMinigameType::TimingBased;

	/** ?�이??(1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	int32 Difficulty = 5;

	/** ?�한?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float TimeLimit = 30.0f;

	/** ?�공 보너??배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float SuccessBonus = 1.2f;

	/** ?�벽 ?�공 보너??배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float PerfectBonus = 1.5f;
};

/**
 * ?�시???�이???�셋
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UFishingSpotData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** ?�시???�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FText SpotName;

	/** ?�시???�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	EFishingSpotType SpotType = EFishingSpotType::River;

	/** ?�폰 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	TArray<FFishingSpotSpawnEntry> SpawnTable;

	/** 최소 ?�벨 ?�구?�항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	int32 MinimumFishingLevel = 1;

	/** 미니게임 ?�정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FFishingMinigameSettings MinigameSettings;
};

/**
 * ?��? 물고�??�보
 */
USTRUCT(BlueprintType)
struct FCaughtFish
{
	GENERATED_BODY()

	/** 물고�?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FName FishID;

	/** ?�기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Size = 0.0f;

	/** 무게 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Weight = 0.0f;

	/** ?��? ?�간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FDateTime CaughtTime;

	/** ?��? ?�치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FVector CaughtLocation;

	/** ?�질 ?�수 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float QualityScore = 50.0f;
};

/**
 * 물고�??�이???�이�?Row
 * ?�디?�에??DataTable�?물고�??�이?��? 관리할 ???�용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FFishDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 물고�?ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishID;

	/** ?�시 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText DisplayName;

	/** ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** ?��???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** 최소 ?�기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinSize = 10.0f;

	/** 최�? ?�기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxSize = 50.0f;

	/** 최소 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinWeight = 0.5f;

	/** 최�? 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxWeight = 5.0f;

	/** 기본 ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 BasePrice = 10;

	/** ?�았?????�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 ExperienceGain = 5;

	/** ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 3D 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** 출현 가?�한 ?�시???�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<EFishingSpotType> ValidSpotTypes;

	/** 최소 ?�시 ?�벨 ?�구?�항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 MinFishingLevel = 1;

	/** 출현 ?�작 ?�간 (0-23, -1?�면 ??��) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearStartHour = -1;

	/** 출현 종료 ?�간 (0-23, -1?�면 ??��) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearEndHour = -1;

	/** ?�요 ?�씨 조건 (비어?�으�?모든 ?�씨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredWeather;

	/** ?�요 계절 (비어?�으�?모든 계절) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredSeasons;
};
