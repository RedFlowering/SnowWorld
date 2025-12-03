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
	Common			UMETA(DisplayName = "Common"),			// ?�반
	Uncommon		UMETA(DisplayName = "Uncommon"),		// 고급
	Rare			UMETA(DisplayName = "Rare"),			// ?��?
	Epic			UMETA(DisplayName = "Epic"),			// ?�웅
	Legendary		UMETA(DisplayName = "Legendary"),		// ?�설
	Mythic			UMETA(DisplayName = "Mythic"),			// ?�화
	MAX				UMETA(Hidden)
};

/**
 * Crafting result type
 */
UENUM(BlueprintType)
enum class ECraftingResult : uint8
{
	Success			UMETA(DisplayName = "Success"),			// ?�작 ?�공
	Failure			UMETA(DisplayName = "Failure"),			// ?�작 ?�패
	CriticalSuccess	UMETA(DisplayName = "Critical Success"),// ?�?�공 (추�? 보상 ??
	Cancelled		UMETA(DisplayName = "Cancelled")		// 취소??
};

/**
 * Crafting station type
 * Defines where a recipe can be crafted
 */
UENUM(BlueprintType)
enum class ECraftingStationType : uint8
{
	None			UMETA(DisplayName = "None"),			// ?�디?�나 ?�작 가??(?�으�?
	Anvil			UMETA(DisplayName = "Anvil"),			// ?�?�간 모루
	Forge			UMETA(DisplayName = "Forge"),			// ?�광�?
	WorkBench		UMETA(DisplayName = "Work Bench"),		// ?�업?�
	CookingPot		UMETA(DisplayName = "Cooking Pot"),		// ?�리 ?�비
	CampFire		UMETA(DisplayName = "Camp Fire"),		// 캠프?�이??
	AlchemyTable	UMETA(DisplayName = "Alchemy Table"),	// ?�금???�이�?
	SewingTable		UMETA(DisplayName = "Sewing Table"),	// ?�봉?�
	TanningRack		UMETA(DisplayName = "Tanning Rack"),	// 무두�??�반
	Loom			UMETA(DisplayName = "Loom"),			// 베�?
	GrindStone		UMETA(DisplayName = "Grind Stone"),		// ?�돌
	Enchanting		UMETA(DisplayName = "Enchanting"),		// 마법 부?��?
	Custom			UMETA(DisplayName = "Custom"),			// 커스?� (?�그�?지??
	MAX				UMETA(Hidden)
};

/**
 * Recipe difficulty level
 */
UENUM(BlueprintType)
enum class EHarmoniaRecipeDifficulty : uint8
{
	Trivial			UMETA(DisplayName = "Trivial"),			// 매우 ?��?
	Easy			UMETA(DisplayName = "Easy"),			// ?��?
	Normal			UMETA(DisplayName = "Normal"),			// 보통
	Hard			UMETA(DisplayName = "Hard"),			// ?�려?�
	Expert			UMETA(DisplayName = "Expert"),			// ?�문가
	Master			UMETA(DisplayName = "Master"),			// 마스??
	Legendary		UMETA(DisplayName = "Legendary"),		// ?�설
	MAX				UMETA(Hidden)
};

/**
 * Crafting result type (renamed for consistency with subsystem)
 */
UENUM(BlueprintType)
enum class EHarmoniaCraftingResult : uint8
{
	Success				UMETA(DisplayName = "Success"),				// ?�작 ?�공
	Failure				UMETA(DisplayName = "Failure"),				// ?�작 ?�패
	CriticalSuccess		UMETA(DisplayName = "Critical Success"),	// ?�?�공 (추�? 보상 ??
	Cancelled			UMETA(DisplayName = "Cancelled"),			// 취소??
	InvalidRecipe		UMETA(DisplayName = "Invalid Recipe"),		// ?�못???�시??
	InsufficientMaterials	UMETA(DisplayName = "Insufficient Materials"),	// ?�료 부�?
	MAX					UMETA(Hidden)
};

/**
 * Item grade configuration (DataTable row)
 * Configures grade properties like color, stat multipliers, etc.
 */
USTRUCT(BlueprintType)
struct FHarmoniaItemGradeConfig : public FTableRowBase
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
struct FHarmoniaCraftingStationData : public FTableRowBase
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
 * Crafting output item (for subsystem use)
 */
USTRUCT(BlueprintType)
struct FHarmoniaCraftingOutput
{
	GENERATED_BODY()

	// Output item ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FHarmoniaID ItemId = FHarmoniaID();

	// Base quantity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 BaseQuantity = 1;

	// Chance to produce this output (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Chance = 1.0f;

	// Item grade (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	EItemGrade Grade = EItemGrade::Common;

	FHarmoniaCraftingOutput()
		: ItemId()
		, BaseQuantity(1)
		, Chance(1.0f)
		, Grade(EItemGrade::Common)
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
struct FHarmoniaCraftingRecipeData : public FTableRowBase
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

	// Base success rate alias
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Success", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseSuccessRate = 1.0f;

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

	// Alias for ExperienceReward for consistency
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Rewards")
	float ExperienceGain = 0.0f;

	// Whether player needs to learn this recipe first
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	bool bRequiresLearning = false;

	// Whether this recipe is unlocked by default
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	bool bUnlockedByDefault = true;

	// Prerequisite recipes that must be unlocked first
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Requirements")
	TArray<FName> PrerequisiteRecipes;

	// Single category tag (for simpler categorization)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FGameplayTag CategoryTag;

	// Crafting time alias for CastingTime
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Crafting", meta = (ClampMin = "0.0"))
	float CraftingTime = 3.0f;

	// Recipe difficulty
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Crafting")
	EHarmoniaRecipeDifficulty Difficulty = EHarmoniaRecipeDifficulty::Normal;

	// Output items (alias for SuccessResults with different structure)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Results")
	TArray<FHarmoniaCraftingOutput> Outputs;
};

/**
 * Crafting category configuration (DataTable row)
 * Used to organize recipes into categories
 */
USTRUCT(BlueprintType)
struct FHarmoniaCraftingCategoryData : public FTableRowBase
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

	// Skill level (for player skill tracking)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	int32 Level = 1;

	// Current experience points
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float Experience = 0.0f;

	// Current experience for level up tracking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float CurrentExperience = 0.0f;

	// Experience needed for next level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float ExperienceToNextLevel = 100.0f;

	// Crafting station type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	ECraftingStationType Station = ECraftingStationType::None;

	// Crafting speed multiplier based on skill
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float SpeedMultiplier = 1.0f;

	// Success rate bonus based on skill
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float SuccessRateBonus = 0.0f;

	// Bonus success rate alias
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float BonusSuccessRate = 0.0f;

	// Quality bonus based on skill
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float QualityBonus = 0.0f;

	// Bonus quality alias
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category")
	float BonusQuality = 0.0f;
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

/**
 * Crafting session result (for subsystem callbacks)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaCraftingSessionResult
{
	GENERATED_BODY()

	/** Recipe that was crafted */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FHarmoniaID RecipeId;

	/** Result of crafting attempt */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EHarmoniaCraftingResult Result = EHarmoniaCraftingResult::Failure;

	/** Output item IDs (if successful) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FHarmoniaID> OutputItems;

	/** Produced items (alias) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TArray<FCraftingResultItem> ProducedItems;

	/** Crafting quality (0.0 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float Quality = 1.0f;

	/** Experience gained */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 ExperienceGained = 0;

	/** Error message if failed */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FText ErrorMessage;

	/** Crafting time taken */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	float CraftingTime = 0.0f;

	// Helper function to get RecipeID as FName
	FName GetRecipeID() const { return RecipeId.GetID(); }

	FHarmoniaCraftingSessionResult() = default;
};
