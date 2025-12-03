// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaGatheringSystemDefinitions.h
 * @brief Gathering system type definitions and data structures
 * 
 * Contains enums, structs, and data assets for the resource gathering system
 * including resource types, gathering tools, nodes, and skill traits.
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaGatheringSystemDefinitions.generated.h"

/**
 * @enum EGatheringResourceType
 * @brief Types of gathering resources
 */
UENUM(BlueprintType)
enum class EGatheringResourceType : uint8
{
	Mineral		UMETA(DisplayName = "Mineral"),		// Minerals/Ore
	Herb		UMETA(DisplayName = "Herb"),		// Herbs
	Wood		UMETA(DisplayName = "Wood"),		// Lumber
	Fiber		UMETA(DisplayName = "Fiber"),		// Fiber
	Stone		UMETA(DisplayName = "Stone"),		// Stone
	Crystal		UMETA(DisplayName = "Crystal"),		// Crystals
	Flower		UMETA(DisplayName = "Flower"),		// Flowers
	Mushroom	UMETA(DisplayName = "Mushroom")		// Mushrooms
};

/**
 * @enum EGatheringToolType
 * @brief Types of gathering tools
 */
UENUM(BlueprintType)
enum class EGatheringToolType : uint8
{
	None		UMETA(DisplayName = "None"),
	Pickaxe		UMETA(DisplayName = "Pickaxe"),		// Pickaxe
	Axe			UMETA(DisplayName = "Axe"),			// Axe
	Sickle		UMETA(DisplayName = "Sickle"),		// Sickle
	Hammer		UMETA(DisplayName = "Hammer")		// Hammer
};

/**
 * @enum EGatheringRarity
 * @brief Rarity classification for gathering resources
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
 * @struct FGatheringResourceData
 * @brief Gathering resource data structure
 */
USTRUCT(BlueprintType)
struct FGatheringResourceData
{
	GENERATED_BODY()

	/** Resource name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceName;

	/** Resource description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** Type of resource */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** Rarity tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** Required gathering tool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** Minimum tool tier required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinToolTier = 1;

	/** Minimum gathering level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinGatheringLevel = 1;

	/** Gathering time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float GatheringTime = 3.0f;

	/** Minimum yield amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinYield = 1;

	/** Maximum yield amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxYield = 3;

	/** Experience gained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ExperienceReward = 10;

	/** Resource mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UStaticMesh> ResourceMesh;

	/** Resource icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * @struct FGatheringNodeData
 * @brief Gathering node data structure
 */
USTRUCT(BlueprintType)
struct FGatheringNodeData
{
	GENERATED_BODY()

	/** Node identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeID;

	/** Node resource type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EGatheringResourceType NodeType = EGatheringResourceType::Mineral;

	/** List of possible resources from this node */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TArray<FName> PossibleResources;

	/** Maximum harvest count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 MaxHarvestCount = 3;

	/** Respawn time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float RespawnTime = 300.0f;

	/** Node health points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float MaxHealth = 100.0f;

	/** Node mesh asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TSoftObjectPtr<UStaticMesh> NodeMesh;
};

/**
 * @struct FGatheringToolData
 * @brief Gathering tool data structure
 */
USTRUCT(BlueprintType)
struct FGatheringToolData
{
	GENERATED_BODY()

	/** Tool name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FName ToolName;

	/** Type of tool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	EGatheringToolType ToolType = EGatheringToolType::None;

	/** Tool tier level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 ToolTier = 1;

	/** Gathering speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float GatheringSpeedMultiplier = 1.0f;

	/** Yield bonus amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float YieldBonus = 0.0f;

	/** Current durability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 Durability = 100;

	/** Maximum durability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 MaxDurability = 100;
};

/**
 * @struct FGatheringResult
 * @brief Gathering result data
 */
USTRUCT(BlueprintType)
struct FGatheringResult
{
	GENERATED_BODY()

	/** Gathered resource identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName ResourceID;

	/** Quantity gathered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** Experience gained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** Critical gather success flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bCriticalGather = false;

	/** Bonus items obtained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;
};

/**
 * @struct FGatheringTrait
 * @brief Gathering passive skill/trait
 */
USTRUCT(BlueprintType)
struct FGatheringTrait
{
	GENERATED_BODY()

	/** Trait name identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** Trait description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** Gathering speed bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float GatheringSpeedBonus = 0.0f;

	/** Yield bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float YieldBonus = 0.0f;

	/** Critical chance bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CriticalChanceBonus = 0.0f;

	/** Experience bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** Durability consumption reduction percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float DurabilityReduction = 0.0f;

	/** Rare resource discovery chance bonus percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RareResourceBonus = 0.0f;
};

/**
 * @struct FGatheringResourceTableRow
 * @brief Gathering resource data table row for editor DataTable management
 * 
 * Used for managing gathering resources in DataTable format within the editor.
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FGatheringResourceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Resource unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceID;

	/** Display name of the resource */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText DisplayName;

	/** Description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** Type of resource */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** Rarity tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** Required tool type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** Minimum tool tier required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinToolTier = 1;

	/** Minimum gathering level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinGatheringLevel = 1;

	/** Gathering time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float GatheringTime = 3.0f;

	/** Minimum yield amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MinYield = 1;

	/** Maximum yield amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MaxYield = 3;

	/** Experience gained */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 ExperienceGain = 5;

	/** Base selling price */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 BasePrice = 5;

	/** Resource icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Tool durability consumption rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float ToolDurabilityConsume = 1.0f;
};
