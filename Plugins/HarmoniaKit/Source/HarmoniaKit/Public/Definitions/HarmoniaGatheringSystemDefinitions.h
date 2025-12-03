// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaGatheringSystemDefinitions.generated.h"

/**
 * 채집 ?�원 ?�??
 */
UENUM(BlueprintType)
enum class EGatheringResourceType : uint8
{
	Mineral		UMETA(DisplayName = "Mineral"),		// 광물
	Herb		UMETA(DisplayName = "Herb"),		// ?�초
	Wood		UMETA(DisplayName = "Wood"),		// 목재
	Fiber		UMETA(DisplayName = "Fiber"),		// ?�유
	Stone		UMETA(DisplayName = "Stone"),		// ?�재
	Crystal		UMETA(DisplayName = "Crystal"),		// ?�리?�탈
	Flower		UMETA(DisplayName = "Flower"),		// �?
	Mushroom	UMETA(DisplayName = "Mushroom")		// 버섯
};

/**
 * 채집 ?�구 ?�??
 */
UENUM(BlueprintType)
enum class EGatheringToolType : uint8
{
	None		UMETA(DisplayName = "None"),
	Pickaxe		UMETA(DisplayName = "Pickaxe"),		// 곡괭??
	Axe			UMETA(DisplayName = "Axe"),			// ?�끼
	Sickle		UMETA(DisplayName = "Sickle"),		// ??
	Hammer		UMETA(DisplayName = "Hammer")		// 망치
};

/**
 * 채집 ?�원 ?��???
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
 * 채집 ?�원 ?�이??
 */
USTRUCT(BlueprintType)
struct FGatheringResourceData
{
	GENERATED_BODY()

	/** ?�원 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceName;

	/** ?�원 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** ?�원 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** ?��???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** ?�요 ?�구 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** 최소 ?�구 ?�급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinToolTier = 1;

	/** 최소 채집 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinGatheringLevel = 1;

	/** 채집 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float GatheringTime = 3.0f;

	/** 최소 ?�득??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinYield = 1;

	/** 최�? ?�득??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxYield = 3;

	/** ?�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ExperienceReward = 10;

	/** ?�원 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UStaticMesh> ResourceMesh;

	/** ?�원 ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * 채집 ?�드 ?�이??
 */
USTRUCT(BlueprintType)
struct FGatheringNodeData
{
	GENERATED_BODY()

	/** ?�드 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeID;

	/** ?�드 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EGatheringResourceType NodeType = EGatheringResourceType::Mineral;

	/** ?�득 가?�한 ?�원 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TArray<FName> PossibleResources;

	/** 채집 가???�수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 MaxHarvestCount = 3;

	/** 리스???�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float RespawnTime = 300.0f;

	/** ?�드 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float MaxHealth = 100.0f;

	/** ?�드 메시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TSoftObjectPtr<UStaticMesh> NodeMesh;
};

/**
 * 채집 ?�구 ?�이??
 */
USTRUCT(BlueprintType)
struct FGatheringToolData
{
	GENERATED_BODY()

	/** ?�구 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FName ToolName;

	/** ?�구 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	EGatheringToolType ToolType = EGatheringToolType::None;

	/** ?�구 ?�급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 ToolTier = 1;

	/** 채집 ?�도 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float GatheringSpeedMultiplier = 1.0f;

	/** ?�득??보너??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float YieldBonus = 0.0f;

	/** ?�구??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 Durability = 100;

	/** 최�? ?�구??*/
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

	/** ?�득???�원 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName ResourceID;

	/** ?�득??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** ?�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ?�리?�컬 채집 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bCriticalGather = false;

	/** 보너???�이??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;
};

/**
 * 채집 ?�성 (?�시�??�킬)
 */
USTRUCT(BlueprintType)
struct FGatheringTrait
{
	GENERATED_BODY()

	/** ?�성 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** ?�성 ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** 채집 ?�도 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float GatheringSpeedBonus = 0.0f;

	/** ?�득??보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float YieldBonus = 0.0f;

	/** ?�리?�컬 ?�률 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CriticalChanceBonus = 0.0f;

	/** 경험�?보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** ?�구???�모 감소 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float DurabilityReduction = 0.0f;

	/** ?��? ?�원 발견 ?�률 보너??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RareResourceBonus = 0.0f;
};

/**
 * 채집 ?�원 ?�이???�이�?Row
 * ?�디?�에??DataTable�?채집 ?�원??관리할 ???�용
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FGatheringResourceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?�원 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceID;

	/** ?�시 ?�름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText DisplayName;

	/** ?�명 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** ?�원 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** ?��???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** ?�요 ?�구 ?�??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** 최소 ?�구 ?�급 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinToolTier = 1;

	/** 최소 채집 ?�벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinGatheringLevel = 1;

	/** 채집 ?�간 (�? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float GatheringTime = 3.0f;

	/** 최소 ?�득??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MinYield = 1;

	/** 최�? ?�득??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MaxYield = 3;

	/** ?�득 경험�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 ExperienceGain = 5;

	/** 기본 ?�매 가�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 BasePrice = 5;

	/** ?�이�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?�구 ?�구???�모??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float ToolDurabilityConsume = 1.0f;
};
