// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "HarmoniaEquipmentSystemDefinitions.generated.h"

/**
 * Equipment slot types
 */
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None			UMETA(DisplayName = "None"),
	Head			UMETA(DisplayName = "Head"),			// Helmet, Hat
	Chest			UMETA(DisplayName = "Chest"),			// Armor, Shirt
	Legs			UMETA(DisplayName = "Legs"),			// Pants, Greaves
	Feet			UMETA(DisplayName = "Feet"),			// Boots, Shoes
	Hands			UMETA(DisplayName = "Hands"),			// Gloves
	MainHand		UMETA(DisplayName = "Main Hand"),		// Primary Weapon
	OffHand			UMETA(DisplayName = "Off Hand"),		// Shield, Secondary Weapon
	Accessory1		UMETA(DisplayName = "Accessory 1"),		// Ring, Necklace
	Accessory2		UMETA(DisplayName = "Accessory 2"),		// Ring, Necklace
	Back			UMETA(DisplayName = "Back"),			// Cape, Backpack
	MAX				UMETA(Hidden)
};

/**
 * Stat modifier type
 */
UENUM(BlueprintType)
enum class EStatModifierType : uint8
{
	Flat			UMETA(DisplayName = "Flat Value"),		// Add flat value (e.g., +10 HP)
	Percentage		UMETA(DisplayName = "Percentage"),		// Add percentage (e.g., +10% HP)
	Override		UMETA(DisplayName = "Override")			// Override value (rare)
};

/**
 * Individual stat modifier
 */
USTRUCT(BlueprintType)
struct FEquipmentStatModifier
{
	GENERATED_BODY()

	// Attribute name (e.g., "MaxHealth", "AttackPower", "Defense")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FString AttributeName;

	// Modifier type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EStatModifierType ModifierType = EStatModifierType::Flat;

	// Modifier value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	float Value = 0.f;

	FEquipmentStatModifier()
		: AttributeName("")
		, ModifierType(EStatModifierType::Flat)
		, Value(0.f)
	{}

	FEquipmentStatModifier(const FString& InAttributeName, EStatModifierType InModifierType, float InValue)
		: AttributeName(InAttributeName)
		, ModifierType(InModifierType)
		, Value(InValue)
	{}
};

/**
 * Equipment data (DataTable row)
 */
USTRUCT(BlueprintType)
struct FEquipmentData : public FTableRowBase
{
	GENERATED_BODY()

	// Equipment ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FHarmoniaID EquipmentId = FHarmoniaID();

	// Equipment name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FText DisplayName = FText();

	// Equipment description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	FText Description = FText();

	// Equipment icon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TSoftObjectPtr<UTexture2D> Icon = nullptr;

	// Equipment slot
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	EEquipmentSlot EquipmentSlot = EEquipmentSlot::None;

	// Required level to equip
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	int32 RequiredLevel = 1;

	// Required stats to equip
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Requirements")
	int32 RequiredStrength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Requirements")
	int32 RequiredDexterity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Requirements")
	int32 RequiredIntelligence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Requirements")
	int32 RequiredFaith = 0;

	// Stat modifiers
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Stats")
	TArray<FEquipmentStatModifier> StatModifiers;

	// Gameplay tags for buffs/effects
	// Example: "Equipment.Buff.FireResistance", "Equipment.Set.Knight"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Effects")
	FGameplayTagContainer GrantedTags;

	// Gameplay Effect classes to apply when equipped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Effects")
	TArray<TSubclassOf<class UGameplayEffect>> GrantedEffects;

	// Mesh or actor to spawn when equipped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TSoftObjectPtr<USkeletalMesh> EquipmentMesh = nullptr;

	// Skeletal mesh socket to attach to
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	FName AttachSocketName = NAME_None;

	// Material instances to apply
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TArray<TSoftObjectPtr<UMaterialInstance>> MaterialInstances;

	// Durability (0 = indestructible)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability")
	float MaxDurability = 100.f;

	// Weight
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Properties")
	float Weight = 1.f;

	// Item rarity/quality (legacy - use Grade instead)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Properties")
	int32 Rarity = 0;

	// Item grade/tier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Properties")
	EItemGrade Grade = EItemGrade::Common;
};

/**
 * Equipped item instance
 */
USTRUCT(BlueprintType)
struct FEquippedItem
{
	GENERATED_BODY()

	// Equipment ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FHarmoniaID EquipmentId = FHarmoniaID();

	// Equipment slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EEquipmentSlot Slot = EEquipmentSlot::None;

	// Current durability
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float CurrentDurability = 100.f;

	// Instance GUID (for unique instances)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FGuid InstanceGUID = FGuid();

	// Active gameplay effect handles (for cleanup)
	UPROPERTY()
	TArray<struct FActiveGameplayEffectHandle> ActiveEffectHandles;

	FEquippedItem()
		: EquipmentId()
		, Slot(EEquipmentSlot::None)
		, CurrentDurability(100.f)
		, InstanceGUID()
	{}

	bool IsValid() const
	{
		return EquipmentId.IsValid() && Slot != EEquipmentSlot::None;
	}
};

/**
 * Equipment save data
 */
USTRUCT(BlueprintType)
struct FEquipmentSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<EEquipmentSlot, FEquippedItem> EquippedItems;

	FEquipmentSaveData()
		: EquippedItems()
	{}
};
