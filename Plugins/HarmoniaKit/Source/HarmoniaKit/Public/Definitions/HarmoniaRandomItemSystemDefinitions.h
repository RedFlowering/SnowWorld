// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "HarmoniaRandomItemSystemDefinitions.generated.h"

/**
 * Item rarity (for random generation)
 */
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common			UMETA(DisplayName = "Common"),			// 일반
	Uncommon		UMETA(DisplayName = "Uncommon"),		// 고급
	Rare			UMETA(DisplayName = "Rare"),			// 희귀
	Epic			UMETA(DisplayName = "Epic"),			// 영웅
	Legendary		UMETA(DisplayName = "Legendary"),		// 전설
	MAX				UMETA(Hidden)
};

// Alias for naming consistency
using EHarmoniaItemRarity = EItemRarity;

/**
 * Affix type
 */
UENUM(BlueprintType)
enum class EAffixType : uint8
{
	Prefix			UMETA(DisplayName = "Prefix"),			// 접두사
	Suffix			UMETA(DisplayName = "Suffix"),			// 접미사
	Implicit		UMETA(DisplayName = "Implicit"),		// 암시적 (기본 부여)
	Enchant			UMETA(DisplayName = "Enchant"),			// 마법부여
	MAX				UMETA(Hidden)
};

// Alias for naming consistency
using EHarmoniaAffixType = EAffixType;

/**
 * Affix stat modifier type
 */
UENUM(BlueprintType)
enum class EAffixModifierType : uint8
{
	Flat			UMETA(DisplayName = "Flat"),			// 고정값 (예: +50 공격력)
	Percent			UMETA(DisplayName = "Percent"),			// 퍼센트 (예: +10% 공격력)
	Multiplier		UMETA(DisplayName = "Multiplier"),		// 배율 (예: 1.5x 피해)
	MAX				UMETA(Hidden)
};

/**
 * Rarity weight configuration
 */
USTRUCT(BlueprintType)
struct FRarityWeight : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	EItemRarity Rarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	int32 BaseWeight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	FLinearColor DisplayColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	int32 MinAffixCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	int32 MaxAffixCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity")
	float StatMultiplier = 1.0f;
};

/**
 * Affix stat modifier
 */
USTRUCT(BlueprintType)
struct FAffixStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
	FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
	EAffixModifierType ModifierType = EAffixModifierType::Flat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affix")
	float MaxValue = 0.0f;
};

/**
 * Affix definition
 */
USTRUCT(BlueprintType)
struct FAffixDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FHarmoniaID AffixId = FHarmoniaID();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FText DisplayName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	EAffixType Type = EAffixType::Prefix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	EItemRarity MinRarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	int32 Weight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	int32 MinItemLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	int32 MaxItemLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	TArray<FAffixStatModifier> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FGameplayTagContainer RequiredItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FGameplayTagContainer ExcludedItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Affix")
	FGameplayTagContainer ExclusiveWithAffixes;
};

/**
 * Applied affix instance
 */
USTRUCT(BlueprintType)
struct FAppliedAffix
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Affix")
	FHarmoniaID AffixId = FHarmoniaID();

	UPROPERTY(BlueprintReadOnly, Category = "Affix")
	EAffixType Type = EAffixType::Prefix;

	UPROPERTY(BlueprintReadOnly, Category = "Affix")
	TMap<FGameplayTag, float> RolledValues;
};

/**
 * Item generation parameters
 */
USTRUCT(BlueprintType)
struct FItemGenerationParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 ItemLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	EItemRarity MinRarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	EItemRarity MaxRarity = EItemRarity::Legendary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float MagicFindBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FGameplayTagContainer RequiredItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	FGameplayTagContainer ExcludedItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 Seed = 0;  // 0 = random
};

/**
 * Generated item data
 */
USTRUCT(BlueprintType)
struct FGeneratedItemData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FGuid ItemGUID;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FHarmoniaID BaseItemId = FHarmoniaID();

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	FText GeneratedName = FText();

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	EItemRarity Rarity = EItemRarity::Common;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 ItemLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TArray<FAppliedAffix> Affixes;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	TMap<FGameplayTag, float> TotalStats;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 GenerationSeed = 0;

	FGeneratedItemData()
	{
		ItemGUID = FGuid::NewGuid();
	}
};

/**
 * Item base template for generation
 */
USTRUCT(BlueprintType)
struct FItemBaseTemplate : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	FHarmoniaID BaseItemId = FHarmoniaID();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	FText BaseName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	FGameplayTagContainer ItemTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	int32 MinItemLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	int32 MaxItemLevel = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	int32 DropWeight = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	TArray<FAffixStatModifier> BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	int32 MaxPrefixCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	int32 MaxSuffixCount = 3;
};

// ============================================================================
// Type aliases for consistent naming with subsystems
// ============================================================================

using FHarmoniaAffixDefinition = FAffixDefinition;
using FHarmoniaRarityWeight = FRarityWeight;
using FHarmoniaGeneratedItem = FGeneratedItemData;
using FHarmoniaItemGenerationSettings = FItemGenerationParams;
using FHarmoniaItemBaseTemplate = FItemBaseTemplate;
using FHarmoniaAppliedAffix = FAppliedAffix;
