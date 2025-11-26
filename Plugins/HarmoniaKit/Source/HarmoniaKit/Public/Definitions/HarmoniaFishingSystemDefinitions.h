// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaFishingSystemDefinitions.generated.h"

/**
 * 물고기 희귀도
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
 * 낚시 미니게임 타입
 */
UENUM(BlueprintType)
enum class EFishingMinigameType : uint8
{
	TimingBased		UMETA(DisplayName = "Timing Based"),		// 타이밍 맞추기
	BarBalance		UMETA(DisplayName = "Bar Balance"),		// 바 균형 유지
	QuickTimeEvent	UMETA(DisplayName = "Quick Time Event"),	// QTE 이벤트
	ReelTension		UMETA(DisplayName = "Reel Tension")		// 릴 장력 관리
};

/**
 * 낚시터 타입
 */
UENUM(BlueprintType)
enum class EFishingSpotType : uint8
{
	River		UMETA(DisplayName = "River"),
	Lake		UMETA(DisplayName = "Lake"),
	Ocean		UMETA(DisplayName = "Ocean"),
	Pond		UMETA(DisplayName = "Pond"),
	Special		UMETA(DisplayName = "Special")		// 특별한 낚시터
};

/**
 * 물고기 데이터
 */
USTRUCT(BlueprintType)
struct FFishData
{
	GENERATED_BODY()

	/** 물고기 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishName;

	/** 물고기 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** 희귀도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** 최소 크기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinSize = 10.0f;

	/** 최대 크기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxSize = 50.0f;

	/** 최소 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MinWeight = 0.5f;

	/** 최대 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxWeight = 5.0f;

	/** 판매 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 BasePrice = 10;

	/** 물고기 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UStaticMesh> FishMesh;

	/** 물고기 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 출현 시간 (24시간 기준, -1이면 항상) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearStartHour = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 AppearEndHour = -1;

	/** 출현 날씨 (비어있으면 모든 날씨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredWeather;

	/** 출현 계절 (비어있으면 모든 계절) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	TArray<FName> RequiredSeasons;
};

/**
 * 낚시터 스폰 테이블
 */
USTRUCT(BlueprintType)
struct FFishingSpotSpawnEntry
{
	GENERATED_BODY()

	/** 물고기 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName FishID;

	/** 스폰 확률 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnChance = 10.0f;

	/** 최소 레벨 요구사항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 MinFishingLevel = 1;
};

/**
 * 낚시 미니게임 설정
 */
USTRUCT(BlueprintType)
struct FFishingMinigameSettings
{
	GENERATED_BODY()

	/** 미니게임 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	EFishingMinigameType MinigameType = EFishingMinigameType::TimingBased;

	/** 난이도 (1-10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	int32 Difficulty = 5;

	/** 제한시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float TimeLimit = 30.0f;

	/** 성공 보너스 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float SuccessBonus = 1.2f;

	/** 완벽 성공 보너스 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minigame")
	float PerfectBonus = 1.5f;
};

/**
 * 낚시터 데이터 에셋
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UFishingSpotData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 낚시터 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FText SpotName;

	/** 낚시터 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	EFishingSpotType SpotType = EFishingSpotType::River;

	/** 스폰 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	TArray<FFishingSpotSpawnEntry> SpawnTable;

	/** 최소 레벨 요구사항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	int32 MinimumFishingLevel = 1;

	/** 미니게임 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Spot")
	FFishingMinigameSettings MinigameSettings;
};

/**
 * 잡은 물고기 정보
 */
USTRUCT(BlueprintType)
struct FCaughtFish
{
	GENERATED_BODY()

	/** 물고기 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FName FishID;

	/** 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Size = 0.0f;

	/** 무게 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float Weight = 0.0f;

	/** 잡은 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FDateTime CaughtTime;

	/** 잡은 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	FVector CaughtLocation;

	/** 품질 점수 (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caught Fish")
	float QualityScore = 50.0f;
};

/**
 * 물고기 데이터 테이블 Row
 * 에디터에서 DataTable로 물고기 데이터를 관리할 때 사용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FFishDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 물고기 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishID;

	/** 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText Description;

	/** 희귀도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	EFishRarity Rarity = EFishRarity::Common;

	/** 최소 크기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinSize = 10.0f;

	/** 최대 크기 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxSize = 50.0f;

	/** 최소 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MinWeight = 0.5f;

	/** 최대 무게 (kg) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Size")
	float MaxWeight = 5.0f;

	/** 기본 판매 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 BasePrice = 10;

	/** 낚았을 때 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Economy")
	int32 ExperienceGain = 5;

	/** 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 3D 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Visual")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** 출현 가능한 낚시터 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<EFishingSpotType> ValidSpotTypes;

	/** 최소 낚시 레벨 요구사항 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 MinFishingLevel = 1;

	/** 출현 시작 시간 (0-23, -1이면 항상) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearStartHour = -1;

	/** 출현 종료 시간 (0-23, -1이면 항상) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	int32 AppearEndHour = -1;

	/** 필요 날씨 조건 (비어있으면 모든 날씨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredWeather;

	/** 필요 계절 (비어있으면 모든 계절) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Spawn")
	TArray<FName> RequiredSeasons;
};
