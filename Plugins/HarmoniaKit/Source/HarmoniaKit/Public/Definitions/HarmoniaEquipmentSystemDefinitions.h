// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "HarmoniaEquipmentSystemDefinitions.generated.h"

class UGameplayEffect;

/**
 * Weapon type to Ultimate Gauge configuration mapping
 * Used for per-weapon-type Ultimate Gauge regen rate and max gauge adjustments
 */
USTRUCT(BlueprintType)
struct FWeaponUltimateGaugeConfig
{
	GENERATED_BODY()

	/** Weapon type tag to match (e.g., Weapon.Type.Sword, Weapon.Type.Dagger) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Gauge")
	FGameplayTag WeaponTypeTag;

	/** GameplayEffect to apply when this weapon type is equipped
	 * This GE should modify UltimateGaugeRegenRate and/or MaxUltimateGauge
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ultimate Gauge")
	TSubclassOf<UGameplayEffect> UltimateGaugeConfigEffect;

	FWeaponUltimateGaugeConfig()
		: WeaponTypeTag()
		, UltimateGaugeConfigEffect(nullptr)
	{}
};

/**
 * Combat sound types for weapons
 */
UENUM(BlueprintType)
enum class ECombatSoundType : uint8
{
	Parry		UMETA(DisplayName = "Parry"),
	Block		UMETA(DisplayName = "Block"),
	Swing		UMETA(DisplayName = "Swing"),
	Hit			UMETA(DisplayName = "Hit")
};

/**
 * Sound configuration for a combat sound type
 * Supports multiple sounds for random selection
 */
USTRUCT(BlueprintType)
struct FCombatSoundConfig
{
	GENERATED_BODY()

	/** Sound tags for this combat action (random selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Categories = "Sound"))
	TArray<FGameplayTag> SoundTags;

	/** Get a random sound tag from the array */
	FGameplayTag GetRandomSoundTag() const
	{
		if (SoundTags.Num() == 0)
		{
			return FGameplayTag();
		}
		return SoundTags[FMath::RandRange(0, SoundTags.Num() - 1)];
	}
};

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
	{
	}

	FEquipmentStatModifier(const FString& InAttributeName, EStatModifierType InModifierType, float InValue)
		: AttributeName(InAttributeName)
		, ModifierType(InModifierType)
		, Value(InValue)
	{
	}
};

/**
 * Configuration for MissNoHit tracer on weapons
 * Enables per-weapon hit detection configuration in DataTable
 */
USTRUCT(BlueprintType)
struct FWeaponTraceConfig
{
	GENERATED_BODY()

	/** Enable tracer for this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	bool bEnableTracer = false;

	/** Tracer tag for identification (e.g., "Tracer.MainHand.Sword") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bEnableTracer", EditConditionHides))
	FGameplayTag TracerTag;

	/** First socket for trace (weapon base/handle) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bEnableTracer", EditConditionHides))
	FName Socket1Name = FName("weapon_base");

	/** Second socket for trace (weapon tip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bEnableTracer", EditConditionHides))
	FName Socket2Name = FName("weapon_tip");

	/** Tracer capsule radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bEnableTracer", EditConditionHides, ClampMin = "1.0"))
	float TracerRadius = 10.0f;

	/** Length offset to extend tracer beyond sockets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bEnableTracer", EditConditionHides))
	float LengthOffset = 0.0f;

	/** Collision channel for trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace", meta = (EditCondition = "bEnableTracer", EditConditionHides))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	/** Enable debug draw visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace|Debug", meta = (EditCondition = "bEnableTracer", EditConditionHides))
	bool bDebugDraw = false;

	FWeaponTraceConfig()
		: bEnableTracer(false)
		, TracerTag()
		, Socket1Name(FName("weapon_base"))
		, Socket2Name(FName("weapon_tip"))
		, TracerRadius(10.0f)
		, LengthOffset(0.0f)
		, TraceChannel(ECC_Pawn)
		, bDebugDraw(false)
	{}
};

/**
 * Equipment data (DataTable row)
 */
USTRUCT(BlueprintType)
struct FHarmoniaEquipmentData : public FTableRowBase
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

	// Offset from socket (position, rotation, scale)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	FTransform AttachOffset = FTransform::Identity;

	// Material instances to apply
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TArray<TSoftObjectPtr<UMaterialInstance>> MaterialInstances;

	// Static mesh for world pickup and UI preview (loot, inventory, shop)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TSoftObjectPtr<UStaticMesh> PickupMesh = nullptr;

	// Actor class to spawn when dropped in world (optional, for physics/interaction)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Visual")
	TSubclassOf<AActor> PickupActorClass = nullptr;

	// Durability (0 = indestructible)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Durability")
	float MaxDurability = 100.f;

	// Weight
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Properties")
	float Weight = 1.f;

	// Item grade/tier
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Properties")
	EItemGrade Grade = EItemGrade::Common;

	// NOTE: MissNoHit tracer configuration moved to separate DataTable
	// Use DT_WeaponTraceConfig for per-weapon tracer settings
	
	// ============================================================================
	// Combat Sounds (무기별 사운드 - 랜덤 선택 지원)
	// ============================================================================
	
	/** 패링 사운드 (여러 개 등록 시 랜덤 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Sound")
	FCombatSoundConfig ParrySounds;
	
	/** 블락 사운드 (여러 개 등록 시 랜덤 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Sound")
	FCombatSoundConfig BlockSounds;
	
	/** 휘두르기 사운드 (여러 개 등록 시 랜덤 선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Sound")
	FCombatSoundConfig SwingSounds;
	
	/** 타격 사운드 (여러 개 등록 시 랜덤 선택, PhysMat은 SoundCacheSubsystem에서 처리) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|Sound")
	FCombatSoundConfig HitSounds;
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
