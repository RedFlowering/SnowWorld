// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaEnhancementSystemDefinitions.generated.h"

// ============================================================================
// Enums
// ============================================================================

/**
 * Enhancement result type
 */
UENUM(BlueprintType)
enum class EEnhancementResult : uint8
{
	Success				UMETA(DisplayName = "Success"),				// Enhancement succeeded
	Failure				UMETA(DisplayName = "Failure"),				// Enhancement failed, item not destroyed
	GreatSuccess		UMETA(DisplayName = "Great Success"),		// Enhancement succeeded with bonus
	Destruction			UMETA(DisplayName = "Destruction"),			// Enhancement failed, item destroyed
	LevelDown			UMETA(DisplayName = "Level Down"),			// Enhancement failed, level decreased
	Downgrade			UMETA(DisplayName = "Downgrade"),			// Alias for LevelDown
	Protected			UMETA(DisplayName = "Protected"),			// Failed but protected
	InvalidItem			UMETA(DisplayName = "Invalid Item"),		// Item invalid or not found
	InsufficientMaterials	UMETA(DisplayName = "Insufficient Materials"),	// Not enough materials
	InsufficientCurrency	UMETA(DisplayName = "Insufficient Currency"),	// Not enough currency
	Cancelled			UMETA(DisplayName = "Cancelled")			// Enhancement cancelled by user
};

/**
 * Enchant slot type
 */
UENUM(BlueprintType)
enum class EEnchantSlot : uint8
{
	None				UMETA(DisplayName = "None"),
	Weapon				UMETA(DisplayName = "Weapon"),
	Armor				UMETA(DisplayName = "Armor"),
	Accessory			UMETA(DisplayName = "Accessory"),
	Universal			UMETA(DisplayName = "Universal"),
	Socket1				UMETA(DisplayName = "Socket 1"),
	Socket2				UMETA(DisplayName = "Socket 2"),
	Socket3				UMETA(DisplayName = "Socket 3")
};

/**
 * Protection type (for enhancement failure protection)
 */
UENUM(BlueprintType)
enum class EProtectionType : uint8
{
	None				UMETA(DisplayName = "None"),
	PreventDestruction	UMETA(DisplayName = "Prevent Destruction"),
	PreventLevelDown	UMETA(DisplayName = "Prevent Level Down"),
	IncreaseSuccess		UMETA(DisplayName = "Increase Success Rate"),
	GuaranteeSuccess	UMETA(DisplayName = "Guarantee Success")
};

/**
 * Gem socket type
 */
UENUM(BlueprintType)
enum class EGemSocketType : uint8
{
	None				UMETA(DisplayName = "None"),
	Red					UMETA(DisplayName = "Red Socket"),			// Strength/Attack gems
	Blue				UMETA(DisplayName = "Blue Socket"),			// Intelligence/Mana gems
	Yellow				UMETA(DisplayName = "Yellow Socket"),		// Dexterity/Speed gems
	Green				UMETA(DisplayName = "Green Socket"),		// Vitality/Defense gems
	Universal			UMETA(DisplayName = "Universal Socket"),	// Any gem type
	MAX					UMETA(Hidden)
};

/**
 * Gem type/category
 */
UENUM(BlueprintType)
enum class EGemType : uint8
{
	None				UMETA(DisplayName = "None"),
	Attack				UMETA(DisplayName = "Attack"),				// Increases damage
	Defense				UMETA(DisplayName = "Defense"),				// Increases defense/armor
	Health				UMETA(DisplayName = "Health"),				// Increases HP
	Mana				UMETA(DisplayName = "Mana"),				// Increases mana
	Speed				UMETA(DisplayName = "Speed"),				// Increases movement/attack speed
	Critical			UMETA(DisplayName = "Critical"),			// Increases critical chance/damage
	Elemental			UMETA(DisplayName = "Elemental"),			// Adds elemental damage
	Special				UMETA(DisplayName = "Special"),				// Special effects
	MAX					UMETA(Hidden)
};

/**
 * Reforge stat type
 */
UENUM(BlueprintType)
enum class EReforgeStatType : uint8
{
	Attack				UMETA(DisplayName = "Attack Power"),
	Defense				UMETA(DisplayName = "Defense"),
	MaxHealth			UMETA(DisplayName = "Max Health"),
	MaxMana				UMETA(DisplayName = "Max Mana"),
	CriticalChance		UMETA(DisplayName = "Critical Chance"),
	CriticalDamage		UMETA(DisplayName = "Critical Damage"),
	AttackSpeed			UMETA(DisplayName = "Attack Speed"),
	MovementSpeed		UMETA(DisplayName = "Movement Speed"),
	HealthRegen			UMETA(DisplayName = "Health Regeneration"),
	ManaRegen			UMETA(DisplayName = "Mana Regeneration"),
	Evasion				UMETA(DisplayName = "Evasion"),
	Accuracy			UMETA(DisplayName = "Accuracy"),
	ElementalResistance	UMETA(DisplayName = "Elemental Resistance"),
	MAX					UMETA(Hidden)
};

/**
 * Transcendence tier
 */
UENUM(BlueprintType)
enum class ETranscendenceTier : uint8
{
	None				UMETA(DisplayName = "None"),
	Tier1				UMETA(DisplayName = "Tier 1 - Ascended"),
	Tier2				UMETA(DisplayName = "Tier 2 - Exalted"),
	Tier3				UMETA(DisplayName = "Tier 3 - Divine"),
	Tier4				UMETA(DisplayName = "Tier 4 - Celestial"),
	Tier5				UMETA(DisplayName = "Tier 5 - Eternal"),
	MAX					UMETA(Hidden)
};

/**
 * Repair kit type
 */
UENUM(BlueprintType)
enum class ERepairKitType : uint8
{
	None				UMETA(DisplayName = "None"),
	Basic				UMETA(DisplayName = "Basic Repair Kit"),		// Repairs 25% durability
	Advanced			UMETA(DisplayName = "Advanced Repair Kit"),		// Repairs 50% durability
	Professional		UMETA(DisplayName = "Professional Repair Kit"),	// Repairs 75% durability
	Master				UMETA(DisplayName = "Master Repair Kit"),		// Repairs 100% durability
	Magical				UMETA(DisplayName = "Magical Repair Kit"),		// Repairs 100% + bonus
	MAX					UMETA(Hidden)
};

// ============================================================================
// Structs
// ============================================================================

/**
 * Enhancement level configuration
 */
USTRUCT(BlueprintType)
struct FEnhancementLevelConfig : public FTableRowBase
{
	GENERATED_BODY()

	/** Enhancement level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement")
	int32 Level = 0;

	/** Required materials */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement")
	TMap<FHarmoniaID, int32> RequiredMaterials;

	/** Required currency amount */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement")
	int32 RequiredCurrency = 0;

	/** Success chance (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuccessChance = 1.0f;

	/** Great success chance (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GreatSuccessChance = 0.0f;

	/** Destruction chance on failure (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DestructionChance = 0.0f;

	/** Level down chance on failure (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LevelDownChance = 0.0f;

	/** Stat bonus multiplier at this level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement")
	float StatMultiplier = 1.0f;

	/** Additional stat modifiers granted at this level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhancement")
	TArray<FEquipmentStatModifier> BonusStats;
};

/**
 * Socket data
 */
USTRUCT(BlueprintType)
struct FSocketData
{
	GENERATED_BODY()

	/** Socket type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	EGemSocketType SocketType = EGemSocketType::None;

	/** Inserted gem ID (invalid if empty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FHarmoniaID InsertedGemId = FHarmoniaID();

	/** Is socket unlocked? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bIsUnlocked = false;

	FSocketData()
		: SocketType(EGemSocketType::None)
		, InsertedGemId()
		, bIsUnlocked(false)
	{}

	FSocketData(EGemSocketType InType, bool bUnlocked = true)
		: SocketType(InType)
		, InsertedGemId()
		, bIsUnlocked(bUnlocked)
	{}

	/** Check if socket has a gem */
	bool HasGem() const { return InsertedGemId.IsValid(); }

	/** Check if socket is empty and unlocked */
	bool IsAvailable() const { return bIsUnlocked && !HasGem(); }
};

/**
 * Gem data (DataTable row)
 */
USTRUCT(BlueprintType)
struct FGemData : public FTableRowBase
{
	GENERATED_BODY()

	/** Gem ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FHarmoniaID GemId = FHarmoniaID();

	/** Gem name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FText DisplayName = FText();

	/** Gem description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FText Description = FText();

	/** Gem icon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	TSoftObjectPtr<UTexture2D> Icon = nullptr;

	/** Gem type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	EGemType GemType = EGemType::None;

	/** Gem tier/level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	int32 GemTier = 1;

	/** Gem grade */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	EItemGrade Grade = EItemGrade::Common;

	/** Compatible socket types */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	TArray<EGemSocketType> CompatibleSockets;

	/** Stat modifiers granted by this gem */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Stats")
	TArray<FEquipmentStatModifier> StatModifiers;

	/** Gameplay tags granted by this gem */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Effects")
	FGameplayTagContainer GrantedTags;

	/** Gameplay effects granted by this gem */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Effects")
	TArray<TSubclassOf<class UGameplayEffect>> GrantedEffects;

	/** Check if gem can be inserted into socket type */
	bool CanInsertIntoSocket(EGemSocketType SocketType) const
	{
		if (SocketType == EGemSocketType::Universal) return true;
		return CompatibleSockets.Contains(SocketType) || CompatibleSockets.Contains(EGemSocketType::Universal);
	}
};

/**
 * Reforge stat entry
 */
USTRUCT(BlueprintType)
struct FReforgeStatEntry
{
	GENERATED_BODY()

	/** Stat type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reforge")
	EReforgeStatType StatType = EReforgeStatType::Attack;

	/** Stat value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reforge")
	float Value = 0.0f;

	/** Is percentage modifier? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reforge")
	bool bIsPercentage = false;

	/** Is this stat locked from reforge changes? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reforge")
	bool bIsLocked = false;

	FReforgeStatEntry()
		: StatType(EReforgeStatType::Attack)
		, Value(0.0f)
		, bIsPercentage(false)
		, bIsLocked(false)
	{}

	FReforgeStatEntry(EReforgeStatType InType, float InValue, bool bPercentage = false)
		: StatType(InType)
		, Value(InValue)
		, bIsPercentage(bPercentage)
		, bIsLocked(false)
	{}
};

/**
 * Reforge configuration (DataTable row)
 */
USTRUCT(BlueprintType)
struct FReforgeConfig : public FTableRowBase
{
	GENERATED_BODY()

	/** Item grade */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	EItemGrade ItemGrade = EItemGrade::Common;

	/** Required currency for reforge */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	int32 RequiredCurrency = 0;

	/** Required materials for reforge */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	TMap<FHarmoniaID, int32> RequiredMaterials;

	/** Minimum number of stats to roll */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	int32 MinStatCount = 1;

	/** Maximum number of stats to roll */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	int32 MaxStatCount = 3;

	/** Possible stats pool */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	TArray<EReforgeStatType> PossibleStats;

	/** Min value range for stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	float MinStatValue = 1.0f;

	/** Max value range for stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reforge")
	float MaxStatValue = 10.0f;
};

/**
 * Transcendence material requirement
 */
USTRUCT(BlueprintType)
struct FTranscendenceMaterial
{
	GENERATED_BODY()

	/** Material ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transcendence")
	FHarmoniaID MaterialId = FHarmoniaID();

	/** Required amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transcendence")
	int32 Amount = 1;

	FTranscendenceMaterial()
		: MaterialId()
		, Amount(1)
	{}

	FTranscendenceMaterial(const FHarmoniaID& InId, int32 InAmount)
		: MaterialId(InId)
		, Amount(InAmount)
	{}
};

/**
 * Transcendence configuration (DataTable row)
 */
USTRUCT(BlueprintType)
struct FTranscendenceConfig : public FTableRowBase
{
	GENERATED_BODY()

	/** Transcendence tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	ETranscendenceTier Tier = ETranscendenceTier::None;

	/** Required base item grade */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	EItemGrade RequiredGrade = EItemGrade::Legendary;

	/** Required enhancement level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	int32 RequiredEnhancementLevel = 10;

	/** Required materials */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	TArray<FTranscendenceMaterial> RequiredMaterials;

	/** Required currency */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	int32 RequiredCurrency = 0;

	/** Stat multiplier boost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	float StatMultiplierBonus = 0.5f; // +50% stats

	/** Additional bonus stats */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	TArray<FEquipmentStatModifier> BonusStats;

	/** Special effects granted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence")
	TArray<TSubclassOf<class UGameplayEffect>> GrantedEffects;

	/** Visual effect (particle system, etc) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transcendence|Visual")
	TSoftObjectPtr<class UNiagaraSystem> VisualEffect = nullptr;
};

/**
 * Transmog (appearance override) data
 */
USTRUCT(BlueprintType)
struct FTransmogData
{
	GENERATED_BODY()

	/** Original equipment ID (for visual override) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmog")
	FHarmoniaID OriginalEquipmentId = FHarmoniaID();

	/** Override mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmog")
	TSoftObjectPtr<USkeletalMesh> OverrideMesh = nullptr;

	/** Override materials */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmog")
	TArray<TSoftObjectPtr<UMaterialInstance>> OverrideMaterials;

	/** Is transmog active? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmog")
	bool bIsActive = false;

	FTransmogData()
		: OriginalEquipmentId()
		, OverrideMesh(nullptr)
		, bIsActive(false)
	{}

	bool IsValid() const { return OriginalEquipmentId.IsValid() && OverrideMesh != nullptr; }
};

/**
 * Repair configuration
 */
USTRUCT(BlueprintType)
struct FRepairConfig : public FTableRowBase
{
	GENERATED_BODY()

	/** Item grade */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair")
	EItemGrade ItemGrade = EItemGrade::Common;

	/** Cost per durability point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair")
	float CostPerDurability = 1.0f;

	/** Can use repair materials instead of currency? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair")
	bool bCanUseRepairMaterials = true;

	/** Repair materials (alternative to currency) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair", meta = (EditCondition = "bCanUseRepairMaterials"))
	TMap<FHarmoniaID, int32> RepairMaterials;

	/** Durability restored per material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair")
	float DurabilityPerMaterial = 10.0f;
};

/**
 * Repair kit data (DataTable row)
 */
USTRUCT(BlueprintType)
struct FRepairKitData : public FTableRowBase
{
	GENERATED_BODY()

	/** Repair kit ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	FHarmoniaID RepairKitId = FHarmoniaID();

	/** Repair kit type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	ERepairKitType KitType = ERepairKitType::Basic;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	FText DisplayName = FText();

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	FText Description = FText();

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	TSoftObjectPtr<UTexture2D> Icon = nullptr;

	/** Durability restored (percentage, 0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DurabilityRestored = 0.25f;

	/** Quality bonus (adds extra durability %) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	float QualityBonus = 0.0f;

	/** Can be used in combat? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	bool bCanUseInCombat = false;

	/** Repair time (seconds, 0 = instant) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	float RepairTime = 0.0f;

	/** Maximum item grade that can be repaired */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	EItemGrade MaxRepairableGrade = EItemGrade::Rare;

	/** Compatible equipment slots (empty = all slots) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	TArray<EEquipmentSlot> CompatibleSlots;

	/** Number of uses before consumed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	int32 MaxUses = 1;

	/** Item grade/rarity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Repair Kit")
	EItemGrade Grade = EItemGrade::Common;

	/** Check if can repair equipment slot */
	bool CanRepairSlot(EEquipmentSlot Slot) const
	{
		return CompatibleSlots.Num() == 0 || CompatibleSlots.Contains(Slot);
	}

	/** Check if can repair item grade */
	bool CanRepairGrade(EItemGrade ItemGrade) const
	{
		return ItemGrade <= MaxRepairableGrade;
	}
};

/**
 * Enhanced item instance data
 * This is saved per-item instance
 */
USTRUCT(BlueprintType)
struct FEnhancedItemData
{
	GENERATED_BODY()

	/** Item instance GUID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement")
	FGuid ItemGUID = FGuid();

	/** Base item ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement")
	FHarmoniaID ItemId = FHarmoniaID();

	/** Enhancement level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement")
	int32 EnhancementLevel = 0;

	/** Sockets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Sockets")
	TArray<FSocketData> Sockets;

	/** Reforged stats */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Reforge")
	TArray<FReforgeStatEntry> ReforgedStats;

	/** Transcendence tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Transcendence")
	ETranscendenceTier TranscendenceTier = ETranscendenceTier::None;

	/** Transmog data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Transmog")
	FTransmogData Transmog;

	/** Current durability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Durability")
	float CurrentDurability = 100.0f;

	/** Max durability (can be increased through enhancement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Durability")
	float MaxDurability = 100.0f;

	/** Applied enchantments by slot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Enchantments")
	TMap<EEnchantSlot, FName> AppliedEnchantments;

	/** Protection count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Protection")
	int32 ProtectionCount = 0;

	/** Pity counter for enhancement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Pity")
	int32 PityCounter = 0;

	/** Failed enhancement attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhancement|Stats")
	int32 FailedAttempts = 0;

	FEnhancedItemData()
		: ItemGUID()
		, ItemId()
		, EnhancementLevel(0)
		, TranscendenceTier(ETranscendenceTier::None)
		, CurrentDurability(100.0f)
		, MaxDurability(100.0f)
		, ProtectionCount(0)
		, PityCounter(0)
		, FailedAttempts(0)
	{}

	/** Get socket count */
	int32 GetSocketCount() const { return Sockets.Num(); }

	/** Get unlocked socket count */
	int32 GetUnlockedSocketCount() const
	{
		return Sockets.FilterByPredicate([](const FSocketData& Socket) { return Socket.bIsUnlocked; }).Num();
	}

	/** Get filled socket count */
	int32 GetFilledSocketCount() const
	{
		return Sockets.FilterByPredicate([](const FSocketData& Socket) { return Socket.HasGem(); }).Num();
	}

	/** Check if item is damaged */
	bool IsDamaged() const { return CurrentDurability < MaxDurability; }

	/** Check if item is broken */
	bool IsBroken() const { return CurrentDurability <= 0.0f; }

	/** Get durability percentage */
	float GetDurabilityPercent() const
	{
		return MaxDurability > 0.0f ? (CurrentDurability / MaxDurability) : 0.0f;
	}
};

/**
 * Enhancement session data
 */
USTRUCT(BlueprintType)
struct FEnhancementSession
{
	GENERATED_BODY()

	/** Session ID */
	UPROPERTY(BlueprintReadOnly, Category = "Enhancement")
	FGuid SessionId = FGuid();

	/** Target item GUID */
	UPROPERTY(BlueprintReadWrite, Category = "Enhancement")
	FGuid TargetItemGUID = FGuid();

	/** Current enhancement level */
	UPROPERTY(BlueprintReadWrite, Category = "Enhancement")
	int32 CurrentLevel = 0;

	/** Target enhancement level */
	UPROPERTY(BlueprintReadWrite, Category = "Enhancement")
	int32 TargetLevel = 0;

	/** Start time */
	UPROPERTY(BlueprintReadOnly, Category = "Enhancement")
	FDateTime StartTime = FDateTime::Now();

	FEnhancementSession()
		: SessionId(FGuid::NewGuid())
		, TargetItemGUID()
		, CurrentLevel(0)
		, TargetLevel(0)
		, StartTime(FDateTime::Now())
	{}
};

/**
 * Enchantment definition (for adding magical properties)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FEnchantmentDefinition : public FTableRowBase
{
	GENERATED_BODY()

	/** Enchantment ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	FName EnchantmentID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	FText Description;

	/** Applicable slot */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	EEnchantSlot Slot = EEnchantSlot::Universal;

	/** Stat modifiers granted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	TArray<FEquipmentStatModifier> StatModifiers;

	/** Exclusive with (cannot be combined with these enchantments) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	TArray<FName> ExclusiveWith;

	/** Required item level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	int32 RequiredItemLevel = 1;

	/** Application cost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	int64 ApplicationCost = 100;

	/** Is this a permanent enchantment? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enchantment")
	bool bPermanent = false;

	FEnchantmentDefinition()
		: EnchantmentID(NAME_None)
		, Slot(EEnchantSlot::Universal)
		, RequiredItemLevel(1)
		, ApplicationCost(100)
		, bPermanent(false)
	{}

	/** Get attribute bonuses (alias for StatModifiers) */
	const TArray<FEquipmentStatModifier>& GetAttributeBonuses() const { return StatModifiers; }
};

/**
 * Enhancement material requirement
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FEnhancementMaterial
{
	GENERATED_BODY()

	/** Item ID required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FHarmoniaID ItemId;

	/** Quantity required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	int32 Quantity = 1;

	/** Is this optional (increases success rate)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	bool bOptional = false;

	/** Success rate bonus if used (for optional materials) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (EditCondition = "bOptional"))
	float SuccessRateBonus = 0.0f;

	FEnhancementMaterial()
		: ItemId()
		, Quantity(1)
		, bOptional(false)
		, SuccessRateBonus(0.0f)
	{}
};

/**
 * Enhancement session result (for subsystem callbacks)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaEnhancementSessionResult
{
	GENERATED_BODY()

	/** The item that was enhanced */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FGuid ItemGUID;

	/** Previous enhancement level */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 PreviousLevel = 0;

	/** Target enhancement level */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 TargetLevel = 0;

	/** Resulting enhancement level / New level after attempt */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int32 ResultingLevel = 0;

	/** Alias for ResultingLevel */
	int32 NewLevel = 0;

	/** Result of enhancement attempt */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EEnhancementResult Result = EEnhancementResult::Failure;

	/** Was protection item used? */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bUsedProtection = false;

	/** Alias for bUsedProtection */
	bool bProtectionUsed = false;

	/** Was item destroyed? */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bItemDestroyed = false;

	/** Cost paid for enhancement */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int64 CostPaid = 0;

	/** Currency spent */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	int64 CurrencySpent = 0;

	/** Error message if failed */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FText ErrorMessage;

	/** Consumed materials */
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	TMap<FHarmoniaID, int32> ConsumedMaterials;

	FHarmoniaEnhancementSessionResult() = default;
};

// ============================================================================
// Delegates
// ============================================================================

/** Enhancement started */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnhancementStarted, FGuid, ItemGUID, int32, TargetLevel);

/** Enhancement completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnhancementCompleted, FGuid, ItemGUID, EEnhancementResult, Result, int32, NewLevel);

/** Gem inserted */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGemInserted, FGuid, ItemGUID, int32, SocketIndex, FHarmoniaID, GemId);

/** Gem removed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGemRemoved, FGuid, ItemGUID, int32, SocketIndex, FHarmoniaID, GemId);

/** Item reforged */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemReforged, FGuid, ItemGUID, const TArray<FReforgeStatEntry>&, NewStats);

/** Item transcended */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemTranscended, FGuid, ItemGUID, ETranscendenceTier, NewTier);

/** Transmog applied */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransmogApplied, FGuid, ItemGUID, FHarmoniaID, AppearanceItemId);

/** Transmog removed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransmogRemoved, FGuid, ItemGUID);

/** Item repaired */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRepaired, FGuid, ItemGUID, float, NewDurability);

/** Item destroyed (broken durability or failed enhancement) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDestroyed, FGuid, ItemGUID, bool, bWasEnhancementFailure);
