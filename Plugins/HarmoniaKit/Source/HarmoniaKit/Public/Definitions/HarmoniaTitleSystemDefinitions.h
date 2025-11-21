// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaTitleSystemDefinitions.generated.h"

/**
 * Title rarity
 */
UENUM(BlueprintType)
enum class ETitleRarity : uint8
{
	Common			UMETA(DisplayName = "Common"),
	Uncommon		UMETA(DisplayName = "Uncommon"),
	Rare			UMETA(DisplayName = "Rare"),
	Epic			UMETA(DisplayName = "Epic"),
	Legendary		UMETA(DisplayName = "Legendary"),
	MAX				UMETA(Hidden)
};

/**
 * Title definition
 */
USTRUCT(BlueprintType)
struct FHarmoniaTitleDefinition : public FTableRowBase
{
	GENERATED_BODY()

	// Title ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FHarmoniaID TitleId;

	// Display Text (e.g., "Dragon Slayer")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FText DisplayText;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FText Description;

	// Text Color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FLinearColor TextColor = FLinearColor::White;

	// Rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	ETitleRarity Rarity = ETitleRarity::Common;

	FHarmoniaTitleDefinition()
		: TitleId()
		, DisplayText()
		, Description()
		, TextColor(FLinearColor::White)
		, Rarity(ETitleRarity::Common)
	{}
};

/**
 * Title runtime state
 */
USTRUCT(BlueprintType)
struct FTitleState
{
	GENERATED_BODY()

	// Title ID
	UPROPERTY(BlueprintReadOnly, Category = "Title")
	FHarmoniaID TitleId;

	// Is Unlocked?
	UPROPERTY(BlueprintReadOnly, Category = "Title")
	bool bUnlocked = false;

	// Unlocked Date
	UPROPERTY(BlueprintReadOnly, Category = "Title")
	FDateTime UnlockedDate;

	FTitleState()
		: TitleId()
		, bUnlocked(false)
		, UnlockedDate(0)
	{}
};
