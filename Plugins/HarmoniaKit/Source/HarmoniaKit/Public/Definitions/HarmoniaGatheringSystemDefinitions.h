// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaGatheringSystemDefinitions.generated.h"

/**
 * 채집 자원 타입
 */
UENUM(BlueprintType)
enum class EGatheringResourceType : uint8
{
	Mineral		UMETA(DisplayName = "Mineral"),		// 광물
	Herb		UMETA(DisplayName = "Herb"),		// 약초
	Wood		UMETA(DisplayName = "Wood"),		// 목재
	Fiber		UMETA(DisplayName = "Fiber"),		// 섬유
	Stone		UMETA(DisplayName = "Stone"),		// 석재
	Crystal		UMETA(DisplayName = "Crystal"),		// 크리스탈
	Flower		UMETA(DisplayName = "Flower"),		// 꽃
	Mushroom	UMETA(DisplayName = "Mushroom")		// 버섯
};

/**
 * 채집 도구 타입
 */
UENUM(BlueprintType)
enum class EGatheringToolType : uint8
{
	None		UMETA(DisplayName = "None"),
	Pickaxe		UMETA(DisplayName = "Pickaxe"),		// 곡괭이
	Axe			UMETA(DisplayName = "Axe"),			// 도끼
	Sickle		UMETA(DisplayName = "Sickle"),		// 낫
	Hammer		UMETA(DisplayName = "Hammer")		// 망치
};

/**
 * 채집 자원 희귀도
 */
UENUM(BlueprintType)
enum class EGatheringRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Epic		UMETA(DisplayName = "Epic"),
	Legendary	UMETA(DisplayName = "Legendary")
};

/**
 * 채집 자원 데이터
 */
USTRUCT(BlueprintType)
struct FGatheringResourceData
{
	GENERATED_BODY()

	/** 자원 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceName;

	/** 자원 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** 자원 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** 희귀도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** 필요 도구 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** 최소 도구 등급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinToolTier = 1;

	/** 최소 채집 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinGatheringLevel = 1;

	/** 채집 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float GatheringTime = 3.0f;

	/** 최소 획득량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinYield = 1;

	/** 최대 획득량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxYield = 3;

	/** 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ExperienceReward = 10;

	/** 자원 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UStaticMesh> ResourceMesh;

	/** 자원 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * 채집 노드 데이터
 */
USTRUCT(BlueprintType)
struct FGatheringNodeData
{
	GENERATED_BODY()

	/** 노드 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeID;

	/** 노드 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EGatheringResourceType NodeType = EGatheringResourceType::Mineral;

	/** 획득 가능한 자원 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TArray<FName> PossibleResources;

	/** 채집 가능 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 MaxHarvestCount = 3;

	/** 리스폰 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float RespawnTime = 300.0f;

	/** 노드 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float MaxHealth = 100.0f;

	/** 노드 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TSoftObjectPtr<UStaticMesh> NodeMesh;
};

/**
 * 채집 도구 데이터
 */
USTRUCT(BlueprintType)
struct FGatheringToolData
{
	GENERATED_BODY()

	/** 도구 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FName ToolName;

	/** 도구 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	EGatheringToolType ToolType = EGatheringToolType::None;

	/** 도구 등급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 ToolTier = 1;

	/** 채집 속도 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float GatheringSpeedMultiplier = 1.0f;

	/** 획득량 보너스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float YieldBonus = 0.0f;

	/** 내구도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 Durability = 100;

	/** 최대 내구도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 MaxDurability = 100;
};

/**
 * 채집 결과
 */
USTRUCT(BlueprintType)
struct FGatheringResult
{
	GENERATED_BODY()

	/** 획득한 자원 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName ResourceID;

	/** 획득량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** 크리티컬 채집 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bCriticalGather = false;

	/** 보너스 아이템 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;
};

/**
 * 채집 특성 (패시브 스킬)
 */
USTRUCT(BlueprintType)
struct FGatheringTrait
{
	GENERATED_BODY()

	/** 특성 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** 특성 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** 채집 속도 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float GatheringSpeedBonus = 0.0f;

	/** 획득량 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float YieldBonus = 0.0f;

	/** 크리티컬 확률 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CriticalChanceBonus = 0.0f;

	/** 경험치 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** 내구도 소모 감소 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float DurabilityReduction = 0.0f;

	/** 희귀 자원 발견 확률 보너스 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RareResourceBonus = 0.0f;
};

/**
 * 채집 자원 데이터 테이블 Row
 * 에디터에서 DataTable로 채집 자원을 관리할 때 사용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FGatheringResourceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 자원 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceID;

	/** 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** 자원 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** 희귀도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** 필요 도구 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** 최소 도구 등급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinToolTier = 1;

	/** 최소 채집 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinGatheringLevel = 1;

	/** 채집 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float GatheringTime = 3.0f;

	/** 최소 획득량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MinYield = 1;

	/** 최대 획득량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MaxYield = 3;

	/** 획득 경험치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 ExperienceGain = 5;

	/** 기본 판매 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 BasePrice = 5;

	/** 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 도구 내구도 소모량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float ToolDurabilityConsume = 1.0f;
};
