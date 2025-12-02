// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "HarmoniaGatheringSystemDefinitions.generated.h"

/**
 * ì±„ì§‘ ?ì› ?€??
 */
UENUM(BlueprintType)
enum class EGatheringResourceType : uint8
{
	Mineral		UMETA(DisplayName = "Mineral"),		// ê´‘ë¬¼
	Herb		UMETA(DisplayName = "Herb"),		// ?½ì´ˆ
	Wood		UMETA(DisplayName = "Wood"),		// ëª©ì¬
	Fiber		UMETA(DisplayName = "Fiber"),		// ?¬ìœ 
	Stone		UMETA(DisplayName = "Stone"),		// ?ì¬
	Crystal		UMETA(DisplayName = "Crystal"),		// ?¬ë¦¬?¤íƒˆ
	Flower		UMETA(DisplayName = "Flower"),		// ê½?
	Mushroom	UMETA(DisplayName = "Mushroom")		// ë²„ì„¯
};

/**
 * ì±„ì§‘ ?„êµ¬ ?€??
 */
UENUM(BlueprintType)
enum class EGatheringToolType : uint8
{
	None		UMETA(DisplayName = "None"),
	Pickaxe		UMETA(DisplayName = "Pickaxe"),		// ê³¡ê´­??
	Axe			UMETA(DisplayName = "Axe"),			// ?„ë¼
	Sickle		UMETA(DisplayName = "Sickle"),		// ??
	Hammer		UMETA(DisplayName = "Hammer")		// ë§ì¹˜
};

/**
 * ì±„ì§‘ ?ì› ?¬ê???
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
 * ì±„ì§‘ ?ì› ?°ì´??
 */
USTRUCT(BlueprintType)
struct FGatheringResourceData
{
	GENERATED_BODY()

	/** ?ì› ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceName;

	/** ?ì› ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** ?ì› ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** ?¬ê???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** ?„ìš” ?„êµ¬ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** ìµœì†Œ ?„êµ¬ ?±ê¸‰ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinToolTier = 1;

	/** ìµœì†Œ ì±„ì§‘ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinGatheringLevel = 1;

	/** ì±„ì§‘ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float GatheringTime = 3.0f;

	/** ìµœì†Œ ?ë“??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MinYield = 1;

	/** ìµœë? ?ë“??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxYield = 3;

	/** ?ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 ExperienceReward = 10;

	/** ?ì› ë©”ì‹œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UStaticMesh> ResourceMesh;

	/** ?ì› ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	TSoftObjectPtr<UTexture2D> Icon;
};

/**
 * ì±„ì§‘ ?¸ë“œ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FGatheringNodeData
{
	GENERATED_BODY()

	/** ?¸ë“œ ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	FName NodeID;

	/** ?¸ë“œ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	EGatheringResourceType NodeType = EGatheringResourceType::Mineral;

	/** ?ë“ ê°€?¥í•œ ?ì› ëª©ë¡ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TArray<FName> PossibleResources;

	/** ì±„ì§‘ ê°€???Ÿìˆ˜ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	int32 MaxHarvestCount = 3;

	/** ë¦¬ìŠ¤???œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float RespawnTime = 300.0f;

	/** ?¸ë“œ ì²´ë ¥ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	float MaxHealth = 100.0f;

	/** ?¸ë“œ ë©”ì‹œ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node")
	TSoftObjectPtr<UStaticMesh> NodeMesh;
};

/**
 * ì±„ì§‘ ?„êµ¬ ?°ì´??
 */
USTRUCT(BlueprintType)
struct FGatheringToolData
{
	GENERATED_BODY()

	/** ?„êµ¬ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	FName ToolName;

	/** ?„êµ¬ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	EGatheringToolType ToolType = EGatheringToolType::None;

	/** ?„êµ¬ ?±ê¸‰ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 ToolTier = 1;

	/** ì±„ì§‘ ?ë„ ë°°ìœ¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float GatheringSpeedMultiplier = 1.0f;

	/** ?ë“??ë³´ë„ˆ??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	float YieldBonus = 0.0f;

	/** ?´êµ¬??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 Durability = 100;

	/** ìµœë? ?´êµ¬??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool")
	int32 MaxDurability = 100;
};

/**
 * ì±„ì§‘ ê²°ê³¼
 */
USTRUCT(BlueprintType)
struct FGatheringResult
{
	GENERATED_BODY()

	/** ?ë“???ì› ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FName ResourceID;

	/** ?ë“??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Quantity = 0;

	/** ?ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 Experience = 0;

	/** ?¬ë¦¬?°ì»¬ ì±„ì§‘ ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bCriticalGather = false;

	/** ë³´ë„ˆ???„ì´??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FName> BonusItems;
};

/**
 * ì±„ì§‘ ?¹ì„± (?¨ì‹œë¸??¤í‚¬)
 */
USTRUCT(BlueprintType)
struct FGatheringTrait
{
	GENERATED_BODY()

	/** ?¹ì„± ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FName TraitName;

	/** ?¹ì„± ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	FText Description;

	/** ì±„ì§‘ ?ë„ ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float GatheringSpeedBonus = 0.0f;

	/** ?ë“??ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float YieldBonus = 0.0f;

	/** ?¬ë¦¬?°ì»¬ ?•ë¥  ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float CriticalChanceBonus = 0.0f;

	/** ê²½í—˜ì¹?ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float ExperienceBonus = 0.0f;

	/** ?´êµ¬???Œëª¨ ê°ì†Œ (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float DurabilityReduction = 0.0f;

	/** ?¬ê? ?ì› ë°œê²¬ ?•ë¥  ë³´ë„ˆ??(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trait")
	float RareResourceBonus = 0.0f;
};

/**
 * ì±„ì§‘ ?ì› ?°ì´???Œì´ë¸?Row
 * ?ë””?°ì—??DataTableë¡?ì±„ì§‘ ?ì›??ê´€ë¦¬í•  ???¬ìš©
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FGatheringResourceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** ?ì› ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FName ResourceID;

	/** ?œì‹œ ?´ë¦„ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText DisplayName;

	/** ?¤ëª… */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	FText Description;

	/** ?ì› ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringResourceType ResourceType = EGatheringResourceType::Mineral;

	/** ?¬ê???*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	EGatheringRarity Rarity = EGatheringRarity::Common;

	/** ?„ìš” ?„êµ¬ ?€??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	EGatheringToolType RequiredTool = EGatheringToolType::None;

	/** ìµœì†Œ ?„êµ¬ ?±ê¸‰ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinToolTier = 1;

	/** ìµœì†Œ ì±„ì§‘ ?ˆë²¨ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Requirement")
	int32 MinGatheringLevel = 1;

	/** ì±„ì§‘ ?œê°„ (ì´? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float GatheringTime = 3.0f;

	/** ìµœì†Œ ?ë“??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MinYield = 1;

	/** ìµœë? ?ë“??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	int32 MaxYield = 3;

	/** ?ë“ ê²½í—˜ì¹?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 ExperienceGain = 5;

	/** ê¸°ë³¸ ?ë§¤ ê°€ê²?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Economy")
	int32 BasePrice = 5;

	/** ?„ì´ì½?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** ?„êµ¬ ?´êµ¬???Œëª¨??*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource|Gathering")
	float ToolDurabilityConsume = 1.0f;
};
