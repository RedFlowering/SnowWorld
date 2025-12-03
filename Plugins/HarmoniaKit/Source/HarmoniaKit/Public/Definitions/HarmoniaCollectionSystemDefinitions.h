// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaCollectionSystemDefinitions.generated.h"

/**
 * Collection item type
 */
UENUM(BlueprintType)
enum class ECollectionItemType : uint8
{
	Item			UMETA(DisplayName = "Item"),			// ?�이??
	Monster			UMETA(DisplayName = "Monster"),			// 몬스??
	Location		UMETA(DisplayName = "Location"),		// 지???�소
	Character		UMETA(DisplayName = "Character"),		// NPC/캐릭??
	Lore			UMETA(DisplayName = "Lore"),			// ?�야�??�정
	Custom			UMETA(DisplayName = "Custom"),			// 커스?�
	MAX				UMETA(Hidden)
};

/**
 * Collection state
 */
UENUM(BlueprintType)
enum class ECollectionState : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),			// 미발�?
	Seen			UMETA(DisplayName = "Seen"),			// 발견??(?�득?� ?�함)
	Collected		UMETA(DisplayName = "Collected"),		// ?�집??
	MAX				UMETA(Hidden)
};

/**
 * Collection item definition
 */
USTRUCT(BlueprintType)
struct FHarmoniaCollectionItemData : public FTableRowBase
{
	GENERATED_BODY()

	// Collection ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FHarmoniaID CollectionId;

	// Display Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FText Description;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	TSoftObjectPtr<UTexture2D> Icon;

	// Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	ECollectionItemType Type = ECollectionItemType::Item;

	// Rarity (Optional, can reuse existing rarity enums if available, using int for generic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 Rarity = 0;

	// Related ID (e.g., ItemID, MonsterID)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	FHarmoniaID RelatedId;

	FHarmoniaCollectionItemData()
		: CollectionId()
		, DisplayName()
		, Description()
		, Icon(nullptr)
		, Type(ECollectionItemType::Item)
		, Rarity(0)
		, RelatedId()
	{}
};

/**
 * Collection runtime state
 */
USTRUCT(BlueprintType)
struct FCollectionState
{
	GENERATED_BODY()

	// Collection ID
	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	FHarmoniaID CollectionId;

	// Current State
	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	ECollectionState State = ECollectionState::Unknown;

	// Collected Count
	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	int32 Count = 0;

	// First Collected Date
	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	FDateTime FirstCollectedDate;

	FCollectionState()
		: CollectionId()
		, State(ECollectionState::Unknown)
		, Count(0)
		, FirstCollectedDate(0)
	{}
};
