// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaCraftingSystemDefinitions.generated.h"

/**
 * Item/Equipment Grade/Tier System
 */
UENUM(BlueprintType)
enum class EItemGrade : uint8
{
	Common			UMETA(DisplayName = "Common"),			// 일반
	Uncommon		UMETA(DisplayName = "Uncommon"),		// 고급
	Rare			UMETA(DisplayName = "Rare"),			// 희귀
	Epic			UMETA(DisplayName = "Epic"),			// 영웅
	Legendary		UMETA(DisplayName = "Legendary"),		// 전설
	Mythic			UMETA(DisplayName = "Mythic"),			// 신화
	MAX				UMETA(Hidden)
};

/**
 * Crafting result type
 */
UENUM(BlueprintType)
enum class ECraftingResult : uint8
{
	Success			UMETA(DisplayName = "Success"),			// 제작 성공
	Failure			UMETA(DisplayName = "Failure"),			// 제작 실패
	CriticalSuccess	UMETA(DisplayName = "Critical Success"),// 대성공 (추가 보상 등)
	Cancelled		UMETA(DisplayName = "Cancelled")		// 취소됨
};

/**
 * Crafting station type
 * Defines where a recipe can be crafted
 */
UENUM(BlueprintType)
enum class ECraftingStationType : uint8
{
	None			UMETA(DisplayName = "None"),			// 어디서나 제작 가능 (손으로)
	Anvil			UMETA(DisplayName = "Anvil"),			// 대장간 모루
	Forge			UMETA(DisplayName = "Forge"),			// 용광로
	WorkBench		UMETA(DisplayName = "Work Bench"),		// 작업대
	CookingPot		UMETA(DisplayName = "Cooking Pot"),		// 요리 냄비
	CampFire		UMETA(DisplayName = "Camp Fire"),		// 캠프파이어
	AlchemyTable	UMETA(DisplayName = "Alchemy Table"),	// 연금술 테이블
	SewingTable		UMETA(DisplayName = "Sewing Table"),	// 재봉틀
	TanningRack		UMETA(DisplayName = "Tanning Rack"),	// 무두질 선반
	Loom			UMETA(DisplayName = "Loom"),			// 베틀
	GrindStone		UMETA(DisplayName = "Grind Stone"),		// 숫돌
	Enchanting		UMETA(DisplayName = "Enchanting"),		// 마법 부여대
	Custom			UMETA(DisplayName = "Custom"),			// 커스텀 (태그로 지정)
	MAX				UMETA(Hidden)
};

/**
 * Item grade configuration (DataTable row)
 * Configures grade properties like color, stat multipliers, etc.
 */
USTRUCT(BlueprintType)
struct FItemGradeConfig : public FTableRowBase
{
	GENERATED_BODY()

	// Grade type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grade")
	EItemGrade Grade = EItemGrade::Common;

	// Display name for this grade
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grade")
	FText DisplayName = FText();

	// Grade color (for UI)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grade")
	FLinearColor GradeColor = FLinearColor::White;

	// Stat multiplier for this grade
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grade")
	float StatMultiplier = 1.0f;

	// Durability multiplier for this grade
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grade")
	float DurabilityMultiplier = 1.0f;

	// Sell price multiplier for this grade
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grade")
	float PriceMultiplier = 1.0f;
};

/**
 * Crafting station data (DataTable row)
 * Defines properties of a crafting station
 */
USTRUCT(BlueprintType)
struct FCraftingStationData : public FTableRowBase
{
	GENERATED_BODY()

	// Station type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	ECraftingStationType StationType = ECraftingStationType::None;

	// Station name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	FText StationName = FText();

	// Station description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	FText StationDescription = FText();

	// Station icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	TSoftObjectPtr<UTexture2D> StationIcon = nullptr;

	// Gameplay tags for custom stations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	FGameplayTagContainer StationTags;

	// Interaction range (how close player needs to be)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	float InteractionRange = 300.0f;
};

/**
 * Material requirement for crafting
 */
USTRUCT(BlueprintType)
struct FCraftingMaterial
{
	GENERATED_BODY()

	// Required item ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FHarmoniaID ItemId = FHarmoniaID();

	// Required amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 Amount = 1;

	// Whether this material is consumed on crafting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	bool bConsumeOnCraft = true;

	FCraftingMaterial()
		: ItemId()
		, Amount(1)
		, bConsumeOnCraft(true)
	{}

	FCraftingMaterial(const FHarmoniaID& InItemId, int32 InAmount, bool bInConsumeOnCraft = true)
		: ItemId(InItemId)
		, Amount(InAmount)
		, bConsumeOnCraft(bInConsumeOnCraft)
	{}
};

/**
 * Crafting result item with probability
 */
USTRUCT(BlueprintType)
struct FCraftingResultItem
{
	GENERATED_BODY()

	// Result item ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FHarmoniaID ItemId = FHarmoniaID();

	// Amount to give
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 Amount = 1;

	// Item grade (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	EItemGrade Grade = EItemGrade::Common;

	// Probability to receive this item (0.0 - 1.0)
	// For example, 0.1 = 10% chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Probability = 1.0f;

	FCraftingResultItem()
		: ItemId()
		, Amount(1)
		, Grade(EItemGrade::Common)
		, Probability(1.0f)
	{}
};

/**
 * Crafting recipe data (DataTable row)
 */
USTRUCT(BlueprintType)
struct FCraftingRecipeData : public FTableRowBase
{
	GENERATED_BODY()

	// Recipe ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FHarmoniaID RecipeId = FHarmoniaID();

	// Recipe name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FText RecipeName = FText();

	// Recipe description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FText RecipeDescription = FText();

	// Recipe icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TSoftObjectPtr<UTexture2D> RecipeIcon = nullptr;

	// Recipe category tags (e.g., "Crafting.Category.Weapon", "Crafting.Category.Armor")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FGameplayTagContainer CategoryTags;

	// Required materials
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Materials")
	TArray<FCraftingMaterial> RequiredMaterials;

	// Casting time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Crafting", meta = (ClampMin = "0.0"))
	float CastingTime = 3.0f;

	// Animation montage to play during crafting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Animation")
	TSoftObjectPtr<UAnimMontage> CraftingMontage = nullptr;

	// Base success probability (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Success", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseSuccessChance = 1.0f;

	// Critical success probability (for bonus rewards) (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Success", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalSuccessChance = 0.0f;

	// Result items on success
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Results")
	TArray<FCraftingResultItem> SuccessResults;

	// Result items on failure (optional, e.g., return some materials)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Results")
	TArray<FCraftingResultItem> FailureResults;

	// Result items on critical success (bonus items)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Results")
	TArray<FCraftingResultItem> CriticalSuccessResults;

	// Required player level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	int32 RequiredLevel = 1;

	// Required crafting skill level (if you have skill system)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	int32 RequiredSkillLevel = 0;

	// Required crafting station type (None = can craft anywhere)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	ECraftingStationType RequiredStation = ECraftingStationType::None;

	// Required station tags (for Custom station type or additional filtering)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	FGameplayTagContainer RequiredStationTags;

	// Experience gained on successful crafting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Rewards")
	int32 ExperienceReward = 0;

	// Whether player needs to learn this recipe first
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	bool bRequiresLearning = false;
};

/**
 * Crafting category configuration (DataTable row)
 * Used to organize recipes into categories
 */
USTRUCT(BlueprintType)
struct FCraftingCategoryData : public FTableRowBase
{
	GENERATED_BODY()

	// Category tag
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	FGameplayTag CategoryTag;

	// Category display name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	FText CategoryName = FText();

	// Category icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	TSoftObjectPtr<UTexture2D> CategoryIcon = nullptr;

	// Sort order
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	int32 SortOrder = 0;
};

/**
 * Active crafting session data
 */
USTRUCT(BlueprintType)
struct FActiveCraftingSession
{
	GENERATED_BODY()

	// Recipe being crafted
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	FHarmoniaID RecipeId = FHarmoniaID();

	// Elapsed time
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float ElapsedTime = 0.0f;

	// Total casting time
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float TotalCastingTime = 0.0f;

	// Whether crafting is in progress
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	bool bIsActive = false;

	// Crafting start timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	float StartTime = 0.0f;

	FActiveCraftingSession()
		: RecipeId()
		, ElapsedTime(0.0f)
		, TotalCastingTime(0.0f)
		, bIsActive(false)
		, StartTime(0.0f)
	{}

	bool IsValid() const
	{
		return RecipeId.IsValid() && bIsActive;
	}

	float GetProgress() const
	{
		if (TotalCastingTime <= 0.0f) return 0.0f;
		return FMath::Clamp(ElapsedTime / TotalCastingTime, 0.0f, 1.0f);
	}
};

/**
 * Learned recipe data for save system
 */
USTRUCT(BlueprintType)
struct FLearnedRecipeSaveData
{
	GENERATED_BODY()

	// Learned recipe IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	TArray<FHarmoniaID> LearnedRecipes;

	FLearnedRecipeSaveData()
		: LearnedRecipes()
	{}
};
