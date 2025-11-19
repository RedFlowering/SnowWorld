// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HarmoniaMovementSystemDefinitions.generated.h"

// ============================================================================
// ENUMS
// ============================================================================

/**
 * Types of mounts available in the game
 */
UENUM(BlueprintType)
enum class EHarmoniaMountType : uint8
{
	None UMETA(DisplayName = "None"),

	// Ground Mounts
	Horse UMETA(DisplayName = "Horse"),
	Warhorse UMETA(DisplayName = "Warhorse"),
	Elk UMETA(DisplayName = "Elk"),
	Wolf UMETA(DisplayName = "Wolf"),
	Bear UMETA(DisplayName = "Bear"),
	Raptor UMETA(DisplayName = "Raptor"),
	Boar UMETA(DisplayName = "Boar"),

	// Flying Mounts
	Griffin UMETA(DisplayName = "Griffin"),
	Dragon UMETA(DisplayName = "Dragon"),
	Wyvern UMETA(DisplayName = "Wyvern"),
	Phoenix UMETA(DisplayName = "Phoenix"),
	Eagle UMETA(DisplayName = "Giant Eagle"),
	Bat UMETA(DisplayName = "Giant Bat"),

	// Exotic Mounts
	Unicorn UMETA(DisplayName = "Unicorn"),
	Nightmare UMETA(DisplayName = "Nightmare"),
	Manticore UMETA(DisplayName = "Manticore"),
	Pegasus UMETA(DisplayName = "Pegasus")
};

/**
 * Mount movement behavior type
 */
UENUM(BlueprintType)
enum class EHarmoniaMountMovementType : uint8
{
	Ground UMETA(DisplayName = "Ground Only"),
	Flying UMETA(DisplayName = "Flying"),
	Amphibious UMETA(DisplayName = "Amphibious (Ground + Water)"),
	Versatile UMETA(DisplayName = "Versatile (Ground + Flying)")
};

/**
 * Climbing surface types
 */
UENUM(BlueprintType)
enum class EHarmoniaClimbingSurfaceType : uint8
{
	None UMETA(DisplayName = "Not Climbable"),
	Rough UMETA(DisplayName = "Rough Surface"),
	Smooth UMETA(DisplayName = "Smooth Surface"),
	Ledge UMETA(DisplayName = "Ledge"),
	Ladder UMETA(DisplayName = "Ladder"),
	Rope UMETA(DisplayName = "Rope"),
	Vine UMETA(DisplayName = "Vine"),
	IceWall UMETA(DisplayName = "Ice Wall")
};

/**
 * Parkour movement types
 */
UENUM(BlueprintType)
enum class EHarmoniaParkourMovementType : uint8
{
	None UMETA(DisplayName = "None"),
	Vault UMETA(DisplayName = "Vault"),
	Slide UMETA(DisplayName = "Slide"),
	WallRun UMETA(DisplayName = "Wall Run"),
	Climb UMETA(DisplayName = "Climb"),
	Hang UMETA(DisplayName = "Hang"),
	Swing UMETA(DisplayName = "Swing"),
	LedgeGrab UMETA(DisplayName = "Ledge Grab"),
	Mantle UMETA(DisplayName = "Mantle")
};

/**
 * Swimming depth levels
 */
UENUM(BlueprintType)
enum class EHarmoniaSwimmingDepth : uint8
{
	Surface UMETA(DisplayName = "Surface"),
	Shallow UMETA(DisplayName = "Shallow (0-5m)"),
	Medium UMETA(DisplayName = "Medium (5-20m)"),
	Deep UMETA(DisplayName = "Deep (20-50m)"),
	Abyss UMETA(DisplayName = "Abyss (50m+)")
};

/**
 * Water environment types
 */
UENUM(BlueprintType)
enum class EHarmoniaWaterEnvironmentType : uint8
{
	Freshwater UMETA(DisplayName = "Freshwater"),
	Saltwater UMETA(DisplayName = "Saltwater"),
	Toxic UMETA(DisplayName = "Toxic"),
	Lava UMETA(DisplayName = "Lava"),
	Ice UMETA(DisplayName = "Icy Water"),
	Magical UMETA(DisplayName = "Magical Water")
};

/**
 * Fast travel network types
 */
UENUM(BlueprintType)
enum class EHarmoniaFastTravelType : uint8
{
	Waypoint UMETA(DisplayName = "Waypoint"),
	Portal UMETA(DisplayName = "Portal"),
	Teleport UMETA(DisplayName = "Teleport Stone"),
	Mount UMETA(DisplayName = "Mount Special Ability"),
	Flight UMETA(DisplayName = "Flight Path"),
	MagicCircle UMETA(DisplayName = "Magic Circle")
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Mount base data configuration
 */
USTRUCT(BlueprintType)
struct FHarmoniaMountData : public FTableRowBase
{
	GENERATED_BODY()

	/** Type of mount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	EHarmoniaMountType MountType = EHarmoniaMountType::Horse;

	/** Movement type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	EHarmoniaMountMovementType MovementType = EHarmoniaMountMovementType::Ground;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount")
	FText Description;

	/** Mount mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Visual")
	TSoftObjectPtr<USkeletalMesh> MountMesh;

	/** Mount animation blueprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Visual")
	TSoftClassPtr<UAnimInstance> AnimationBlueprint;

	/** Base movement speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Stats", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float SpeedMultiplier = 1.5f;

	/** Flying speed (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Stats", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float FlyingSpeedMultiplier = 2.0f;

	/** Sprint speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Stats", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float SprintMultiplier = 1.8f;

	/** Stamina cost per second while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Stats")
	float StaminaCostPerSecond = 5.0f;

	/** Stamina cost for sprint per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Stats")
	float SprintStaminaCostPerSecond = 15.0f;

	/** Health of the mount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Stats")
	float MountHealth = 500.0f;

	/** Can attack while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Combat")
	bool bCanAttackWhileMounted = true;

	/** Can use abilities while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Combat")
	bool bCanUseAbilitiesWhileMounted = false;

	/** Allowed weapon types while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Combat")
	TArray<FGameplayTag> AllowedWeaponTags;

	/** Mount special ability (charge, aerial strike, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Combat")
	TSoftClassPtr<UGameplayAbility> MountSpecialAbility;

	/** Time to mount/dismount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Timing")
	float MountTime = 1.5f;

	/** Can mount in combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Restrictions")
	bool bCanMountInCombat = false;

	/** Minimum character level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|Restrictions")
	int32 RequiredLevel = 1;

	/** Gameplay tags to apply while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount|GameplayTags")
	FGameplayTagContainer MountedTags;
};

/**
 * Climbing surface configuration
 */
USTRUCT(BlueprintType)
struct FHarmoniaClimbingData : public FTableRowBase
{
	GENERATED_BODY()

	/** Surface type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing")
	EHarmoniaClimbingSurfaceType SurfaceType = EHarmoniaClimbingSurfaceType::Rough;

	/** Climbing speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stats", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ClimbingSpeedMultiplier = 0.5f;

	/** Stamina cost per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Stats")
	float StaminaCostPerSecond = 10.0f;

	/** Requires special equipment (climbing gear) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Requirements")
	bool bRequiresClimbingGear = false;

	/** Minimum strength attribute required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Requirements")
	float MinimumStrength = 0.0f;

	/** Can hang indefinitely */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Mechanics")
	bool bCanHangIndefinitely = false;

	/** Jump power multiplier when jumping from wall */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Mechanics")
	float WallJumpPowerMultiplier = 1.2f;

	/** Surface friction coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing|Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SurfaceFriction = 0.8f;
};

/**
 * Parkour movement configuration
 */
USTRUCT(BlueprintType)
struct FHarmoniaParkourData : public FTableRowBase
{
	GENERATED_BODY()

	/** Parkour movement type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	EHarmoniaParkourMovementType MovementType = EHarmoniaParkourMovementType::Vault;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FText DisplayName;

	/** Animation montage to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Animation")
	TSoftObjectPtr<UAnimMontage> AnimationMontage;

	/** Stamina cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Stats")
	float StaminaCost = 15.0f;

	/** Movement speed during action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Stats")
	float MovementSpeed = 600.0f;

	/** Required dexterity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Requirements")
	float RequiredDexterity = 0.0f;

	/** Minimum approach speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Requirements")
	float MinimumApproachSpeed = 300.0f;

	/** Detection range for auto-parkour */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Detection")
	float DetectionRange = 150.0f;

	/** Maximum height for this movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Constraints")
	float MaximumHeight = 200.0f;

	/** Can chain into other parkour moves */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Mechanics")
	bool bCanChainMovement = true;

	/** Tags for valid surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|GameplayTags")
	FGameplayTagContainer ValidSurfaceTags;
};

/**
 * Swimming and diving configuration
 */
USTRUCT(BlueprintType)
struct FHarmoniaSwimmingData : public FTableRowBase
{
	GENERATED_BODY()

	/** Water environment type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	EHarmoniaWaterEnvironmentType EnvironmentType = EHarmoniaWaterEnvironmentType::Freshwater;

	/** Swimming depth */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	EHarmoniaSwimmingDepth DepthLevel = EHarmoniaSwimmingDepth::Surface;

	/** Swimming speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Stats", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float SwimmingSpeedMultiplier = 0.7f;

	/** Diving speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Stats", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float DivingSpeedMultiplier = 0.5f;

	/** Base oxygen capacity (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Oxygen")
	float OxygenCapacity = 60.0f;

	/** Oxygen consumption rate per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Oxygen")
	float OxygenConsumptionRate = 1.0f;

	/** Stamina cost per second while swimming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Stats")
	float StaminaCostPerSecond = 5.0f;

	/** Stamina cost per second while diving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Stats")
	float DivingStaminaCostPerSecond = 8.0f;

	/** Environmental damage per second (for toxic/lava water) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Environment")
	float EnvironmentalDamagePerSecond = 0.0f;

	/** Requires special equipment (diving gear) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Requirements")
	bool bRequiresDivingGear = false;

	/** Pressure damage begins at depth */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Environment")
	float PressureDamageDepth = 50.0f;

	/** Pressure damage per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Environment")
	float PressureDamagePerSecond = 5.0f;

	/** Water density (affects movement) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Physics", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float WaterDensity = 1.0f;

	/** Current strength (drift force) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Environment")
	float CurrentStrength = 0.0f;

	/** Can use weapons underwater */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Combat")
	bool bCanUseWeaponsUnderwater = false;

	/** Allowed underwater abilities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming|Combat")
	FGameplayTagContainer AllowedUnderwaterAbilities;
};

/**
 * Fast travel waypoint data
 */
USTRUCT(BlueprintType)
struct FHarmoniaWaypointData : public FTableRowBase
{
	GENERATED_BODY()

	/** Waypoint unique ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FName WaypointID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FText Description;

	/** Travel type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	EHarmoniaFastTravelType TravelType = EHarmoniaFastTravelType::Waypoint;

	/** World location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FVector Location;

	/** World rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FRotator Rotation;

	/** Icon for map/UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|UI")
	TSoftObjectPtr<UTexture2D> WaypointIcon;

	/** Is discovered by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Discovery")
	bool bDiscoveredByDefault = false;

	/** Discovery radius (auto-discover when player enters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Discovery")
	float DiscoveryRadius = 500.0f;

	/** Travel cost (gold, mana, items, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Cost")
	float TravelCost = 0.0f;

	/** Cooldown after using this waypoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Restrictions")
	float TravelCooldown = 0.0f;

	/** Can travel from anywhere */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Restrictions")
	bool bCanTravelFromAnywhere = false;

	/** Requires item to travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Restrictions")
	bool bRequiresTravelItem = false;

	/** Connected waypoints (one-way connections) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Network")
	TArray<FName> ConnectedWaypoints;

	/** Required quest completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Requirements")
	FGameplayTag RequiredQuestTag;

	/** Minimum level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|Requirements")
	int32 MinimumLevel = 1;

	/** Tags for this waypoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint|GameplayTags")
	FGameplayTagContainer WaypointTags;
};

/**
 * Mount combat data for specialized mount abilities
 */
USTRUCT(BlueprintType)
struct FHarmoniaMountCombatData
{
	GENERATED_BODY()

	/** Damage multiplier while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageMultiplier = 1.0f;

	/** Defense multiplier while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DefenseMultiplier = 0.9f;

	/** Charge attack damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Charge")
	float ChargeAttackDamage = 100.0f;

	/** Charge attack speed threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Charge")
	float ChargeSpeedThreshold = 800.0f;

	/** Charge attack stamina cost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Charge")
	float ChargeStaminaCost = 30.0f;

	/** Trample damage (for enemies below mount) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Trample")
	float TrampleDamage = 50.0f;

	/** Aerial strike damage (flying mounts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Aerial")
	float AerialStrikeDamage = 150.0f;

	/** Can block while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanBlockWhileMounted = true;

	/** Can dodge while mounted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanDodgeWhileMounted = false;
};
