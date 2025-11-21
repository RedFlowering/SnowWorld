// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaCosmeticSystemDefinitions.generated.h"

/**
 * Cosmetic slot type
 */
UENUM(BlueprintType)
enum class ECosmeticSlotType : uint8
{
	Head			UMETA(DisplayName = "Head"),
	Body			UMETA(DisplayName = "Body"),
	Weapon			UMETA(DisplayName = "Weapon"),
	Accessory		UMETA(DisplayName = "Accessory"),
	Aura			UMETA(DisplayName = "Aura"),
	MAX				UMETA(Hidden)
};

/**
 * Cosmetic rarity
 */
UENUM(BlueprintType)
enum class ECosmeticRarity : uint8
{
	Common			UMETA(DisplayName = "Common"),
	Uncommon		UMETA(DisplayName = "Uncommon"),
	Rare			UMETA(DisplayName = "Rare"),
	Epic			UMETA(DisplayName = "Epic"),
	Legendary		UMETA(DisplayName = "Legendary"),
	MAX				UMETA(Hidden)
};

/**
 * Cosmetic item definition
 */
USTRUCT(BlueprintType)
struct FHarmoniaCosmeticItem : public FTableRowBase
{
	GENERATED_BODY()

	// Cosmetic ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	FHarmoniaID CosmeticId;

	// Display Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	FText Description;

	// Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSoftObjectPtr<UTexture2D> Icon;

	// Slot Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	ECosmeticSlotType SlotType = ECosmeticSlotType::Head;

	// Rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	ECosmeticRarity Rarity = ECosmeticRarity::Common;

	// Mesh (Optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSoftObjectPtr<USkeletalMesh> Mesh;

	// Material (Optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSoftObjectPtr<UMaterialInterface> Material;

	// Particle System (Optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSoftObjectPtr<UParticleSystem> Particle;

	FHarmoniaCosmeticItem()
		: CosmeticId()
		, DisplayName()
		, Description()
		, Icon(nullptr)
		, SlotType(ECosmeticSlotType::Head)
		, Rarity(ECosmeticRarity::Common)
		, Mesh(nullptr)
		, Material(nullptr)
		, Particle(nullptr)
	{}
};

/**
 * Cosmetic runtime state
 */
USTRUCT(BlueprintType)
struct FCosmeticState
{
	GENERATED_BODY()

	// Cosmetic ID
	UPROPERTY(BlueprintReadOnly, Category = "Cosmetic")
	FHarmoniaID CosmeticId;

	// Is Unlocked?
	UPROPERTY(BlueprintReadOnly, Category = "Cosmetic")
	bool bUnlocked = false;

	FCosmeticState()
		: CosmeticId()
		, bUnlocked(false)
	{}
};
