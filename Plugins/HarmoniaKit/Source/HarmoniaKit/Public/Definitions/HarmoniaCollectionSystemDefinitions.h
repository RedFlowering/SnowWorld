// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaCollectionSystemDefinitions.h
 * @brief Collection system type definitions
 * @author Harmonia Team
 */

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaCollectionSystemDefinitions.generated.h"

/**
 * @enum ECollectionItemType
 * @brief Collection item type
 */
UENUM(BlueprintType)
enum class ECollectionItemType : uint8
{
	Item			UMETA(DisplayName = "Item"),			// Item
	Monster			UMETA(DisplayName = "Monster"),			// Monster
	Location		UMETA(DisplayName = "Location"),		// Location/Place
	Character		UMETA(DisplayName = "Character"),		// NPC/Character
	Lore			UMETA(DisplayName = "Lore"),			// World lore
	Custom			UMETA(DisplayName = "Custom"),			// Custom
	MAX				UMETA(Hidden)
};

/**
 * @enum ECollectionState
 * @brief Collection state
 */
UENUM(BlueprintType)
enum class ECollectionState : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),			// Undiscovered
	Seen			UMETA(DisplayName = "Seen"),			// Discovered (not obtained)
	Collected		UMETA(DisplayName = "Collected"),		// Collected
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
