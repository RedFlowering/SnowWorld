// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "HarmoniaCombatSystemDefinitions.generated.h"

class UGameplayEffect;
class UGameplayCueSet;

// ============================================================================
// Enums
// ============================================================================

/**
 * Damage Type Enum
 * Defines how damage is applied
 */
UENUM(BlueprintType)
enum class EHarmoniaDamageType : uint8
{
	// Single instant damage (melee hit, bullet)
	Instant UMETA(DisplayName = "Instant Damage"),

	// Damage over time (poison, fire)
	Duration UMETA(DisplayName = "Duration Damage"),

	// Area explosion damage with falloff
	Explosion UMETA(DisplayName = "Explosion Damage"),

	// Percentage-based damage (ignores armor)
	Percentage UMETA(DisplayName = "Percentage Damage"),

	// Custom damage processing
	Custom UMETA(DisplayName = "Custom Damage")
};

/**
 * Attack Trace Shape
 * Defines the shape used for attack detection via Sense System
 */
UENUM(BlueprintType)
enum class EHarmoniaAttackTraceShape : uint8
{
	// Box shape
	Box UMETA(DisplayName = "Box"),

	// Sphere shape
	Sphere UMETA(DisplayName = "Sphere"),

	// Capsule shape
	Capsule UMETA(DisplayName = "Capsule"),

	// Line trace
	Line UMETA(DisplayName = "Line"),

	// Custom shape (defined in component)
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Hit Reaction Type
 * Defines how the target reacts to being hit
 */
UENUM(BlueprintType)
enum class EHarmoniaHitReactionType : uint8
{
	None UMETA(DisplayName = "None"),
	Light UMETA(DisplayName = "Light Hit"),
	Medium UMETA(DisplayName = "Medium Hit"),
	Heavy UMETA(DisplayName = "Heavy Hit"),
	Knockback UMETA(DisplayName = "Knockback"),
	Stun UMETA(DisplayName = "Stun"),
	Ragdoll UMETA(DisplayName = "Ragdoll")
};

/**
 * Attack Type Enum
 * Defines the type of attack for combo lookup
 */
UENUM(BlueprintType)
enum class EHarmoniaAttackType : uint8
{
	Light UMETA(DisplayName = "Light Attack"),
	Heavy UMETA(DisplayName = "Heavy Attack"),
	Special UMETA(DisplayName = "Special Attack"),
	Charged UMETA(DisplayName = "Charged Attack")
};

/**
 * Melee Weapon Type
 * @deprecated Use GameplayTag (Weapon.Type.Sword etc.) instead for better extensibility
 * Soul-like melee weapon categories
 */
UENUM(BlueprintType)
enum class EHarmoniaMeleeWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	Sword UMETA(DisplayName = "Sword"),					// Balanced weapon
	GreatSword UMETA(DisplayName = "Great Sword"),			// Slow, powerful
	Dagger UMETA(DisplayName = "Dagger"),					// Fast, low damage
	Axe UMETA(DisplayName = "Axe"),							// High damage, slow
	Spear UMETA(DisplayName = "Spear"),						// Long reach
	Hammer UMETA(DisplayName = "Hammer"),					// High poise damage
	Katana UMETA(DisplayName = "Katana"),					// Fast combos
	Shield UMETA(DisplayName = "Shield"),					// Defensive
	Fist UMETA(DisplayName = "Fist"),						// Unarmed
	Whip UMETA(DisplayName = "Whip"),						// Unique range
	Scythe UMETA(DisplayName = "Scythe"),					// Wide sweeping
	// Extended weapon types
	Halberd UMETA(DisplayName = "Halberd"),					// Spear + Axe hybrid
	Rapier UMETA(DisplayName = "Rapier"),					// Thrust-focused
	Mace UMETA(DisplayName = "Mace"),						// Blunt weapon
	DualBlades UMETA(DisplayName = "Dual Blades"),			// Twin weapons
	GreatAxe UMETA(DisplayName = "Great Axe"),				// Two-handed axe
	Flail UMETA(DisplayName = "Flail"),						// Chained weapon
	Crossbow UMETA(DisplayName = "Crossbow"),				// Ranged/Melee hybrid
	Staff UMETA(DisplayName = "Staff"),						// Magic weapon
	Custom UMETA(DisplayName = "Custom")					// User-defined
};

/**
 * Defense State
 * Defines the current defensive state
 */
UENUM(BlueprintType)
enum class EHarmoniaDefenseState : uint8
{
	None UMETA(DisplayName = "None"),
	Blocking UMETA(DisplayName = "Blocking"),				// Active block
	Parrying UMETA(DisplayName = "Parrying"),				// Parry window
	Dodging UMETA(DisplayName = "Dodging"),					// I-frames active
	Stunned UMETA(DisplayName = "Stunned"),					// Cannot defend
	RiposteWindow UMETA(DisplayName = "Riposte Window")		// Can perform riposte after successful parry
};

/**
 * Critical Attack Type
 * Defines special critical attack types
 */
UENUM(BlueprintType)
enum class EHarmoniaCriticalAttackType : uint8
{
	Normal UMETA(DisplayName = "Normal"),					// Normal attack
	Backstab UMETA(DisplayName = "Backstab"),				// Attack from behind
	Riposte UMETA(DisplayName = "Riposte"),					// Counter after parry
	PlungingAttack UMETA(DisplayName = "Plunging Attack")	// Falling attack
};

// ============================================================================
// Structs
// ============================================================================

/**
 * Attack Trace Configuration
 * Defines how to trace for attack hits using Sense System
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAttackTraceConfig
{
	GENERATED_BODY()

	// Shape of the trace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	EHarmoniaAttackTraceShape TraceShape = EHarmoniaAttackTraceShape::Sphere;

	// Extent/size of the trace shape (meaning depends on shape)
	// Box: X=Width, Y=Height, Z=Depth
	// Sphere: X=Radius
	// Capsule: X=Radius, Z=HalfHeight
	// Line: X=Length
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	FVector TraceExtent = FVector(50.0f, 50.0f, 50.0f);

	// Offset from component location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	FVector TraceOffset = FVector::ZeroVector;

	// Socket name to attach trace to (if empty, uses component location)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	FName SocketName = NAME_None;

	// Whether to use continuous detection or single check
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	bool bContinuousDetection = false;

	// Duration of continuous detection (0 = until manually stopped)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (EditCondition = "bContinuousDetection"))
	float DetectionDuration = 0.3f;

	// Sensor tag to use for detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	FName SensorTag = FName("Attack");

	// Channel for sense detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	int32 SenseChannel = 0;

	// Minimum score required to register hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	float MinimumSenseScore = 0.5f;

	// Maximum number of targets to hit per attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	int32 MaxTargets = 10;

	// Whether to hit each target only once per attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	bool bHitOncePerTarget = true;

	// Debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugTrace = false;
};

/**
 * Damage Effect Configuration
 * Defines damage application and effects
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDamageEffectConfig
{
	GENERATED_BODY()

	// Type of damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EHarmoniaDamageType DamageType = EHarmoniaDamageType::Instant;

	// Damage multiplier (applied to AttackPower from attributes)
	// Use this for combo step scaling (1.0, 1.1, 1.5 for finisher, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageMultiplier = 1.0f;

	// Duration for DoT effects (Duration damage type only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (EditCondition = "DamageType == EHarmoniaDamageType::Duration"))
	float DurationSeconds = 5.0f;

	// Tick interval for DoT effects (Duration damage type only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (EditCondition = "DamageType == EHarmoniaDamageType::Duration"))
	float TickInterval = 1.0f;

	// Explosion radius (Explosion damage type only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (EditCondition = "DamageType == EHarmoniaDamageType::Explosion"))
	float ExplosionRadius = 500.0f;

	// Explosion falloff exponent (Explosion damage type only, higher = sharper falloff)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (EditCondition = "DamageType == EHarmoniaDamageType::Explosion"))
	float ExplosionFalloff = 1.0f;

	// Gameplay Effect class to apply
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Additional gameplay effects to apply (buffs, debuffs, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	TArray<TSubclassOf<UGameplayEffect>> AdditionalEffects;

	// Gameplay tags to add to the damage context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags")
	FGameplayTagContainer DamageTags;

	// Whether this damage can be blocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCanBeBlocked = true;

	// Whether this damage can be dodged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCanBeDodged = true;

	// Whether this damage can critical hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCanCritical = true;

	// Critical hit chance (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (EditCondition = "bCanCritical", ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalChance = 0.1f;

	// Critical hit damage multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (EditCondition = "bCanCritical"))
	float CriticalMultiplier = 2.0f;

	// SetByCaller tag for damage magnitude (used by GE)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Effect")
	FGameplayTag SetByCallerDamageTag = FGameplayTag::RequestGameplayTag(FName("SetByCaller.Damage"), false);
};

/**
 * Hit Reaction Configuration
 * Defines visual and gameplay feedback for hits
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaHitReactionConfig
{
	GENERATED_BODY()

	// Type of hit reaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	EHarmoniaHitReactionType ReactionType = EHarmoniaHitReactionType::Light;

	// Gameplay Cue tag to trigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Cue")
	FGameplayTag GameplayCueTag;

	// Impact force for physics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	float ImpactForce = 1000.0f;

	// Direction of impact force (if zero, uses hit direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	FVector ImpactDirection = FVector::ZeroVector;

	// Whether to apply hit pause/freeze frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	bool bApplyHitPause = false;

	// Duration of hit pause in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction", meta = (EditCondition = "bApplyHitPause"))
	float HitPauseDuration = 0.1f;

	// Camera shake class to apply
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	// Camera shake scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraShakeScale = 1.0f;
};

/**
 * Attack Hit Result
 * Contains information about a single attack hit
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAttackHitResult
{
	GENERATED_BODY()

	// Actor that was hit
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	TObjectPtr<AActor> HitActor = nullptr;

	// Component that was hit
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	TObjectPtr<UActorComponent> HitComponent = nullptr;

	// Location of the hit
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	FVector HitLocation = FVector::ZeroVector;

	// Normal of the hit surface
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	FVector HitNormal = FVector::ZeroVector;

	// Distance from attacker
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	float Distance = 0.0f;

	// Whether the hit was a critical hit
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	bool bWasCriticalHit = false;

	// Type of critical attack (if critical)
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	EHarmoniaCriticalAttackType CriticalType = EHarmoniaCriticalAttackType::Normal;

	// Whether the hit was blocked
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	bool bWasBlocked = false;

	// Final damage dealt
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	float DamageDealt = 0.0f;

	// Sense score at time of hit
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	float SenseScore = 1.0f;

	// Timestamp of hit
	UPROPERTY(BlueprintReadOnly, Category = "Hit Result")
	float HitTime = 0.0f;

	bool IsValid() const
	{
		return HitActor != nullptr;
	}
};

/**
 * Attack Data
 * Extended combo attack data with damage configuration
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaAttackData
{
	GENERATED_BODY()

	// Trace configuration for this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FHarmoniaAttackTraceConfig TraceConfig;

	// Damage configuration for this attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FHarmoniaDamageEffectConfig DamageConfig;

	// Hit reaction configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	FHarmoniaHitReactionConfig HitReactionConfig;

	// Whether this attack is enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bEnabled = true;
};

/**
 * Hit Direction
 * Direction from which the hit came
 */
UENUM(BlueprintType)
enum class EHarmoniaHitDirection : uint8
{
	Front UMETA(DisplayName = "Front"),
	Back UMETA(DisplayName = "Back"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

/**
 * Hit Reaction Data
 * Defines animation and behavior for hit reactions
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHitReactionData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name of this hit reaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	FText DisplayName;

	// Hit reaction type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	EHarmoniaHitReactionType ReactionType = EHarmoniaHitReactionType::Light;

	// Animation montages for different hit directions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Front = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Back = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Left = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Right = nullptr;

	// Duration of hit stun (player cannot act)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	float StunDuration = 0.3f;

	// Whether this hit reaction can be interrupted
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	bool bCanBeInterrupted = true;

	// Movement speed multiplier during hit reaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	float MovementSpeedMultiplier = 0.5f;

	// Whether to disable input during hit reaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit Reaction")
	bool bDisableInput = true;

	// Gameplay tags to apply during hit reaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
	FGameplayTagContainer AppliedTags;

	// Gameplay tags to block during hit reaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Tags")
	FGameplayTagContainer BlockedTags;

	/**
	 * Get animation montage for specific direction
	 */
	UAnimMontage* GetMontageForDirection(EHarmoniaHitDirection Direction) const
	{
		switch (Direction)
		{
		case EHarmoniaHitDirection::Front:
			return HitMontage_Front;
		case EHarmoniaHitDirection::Back:
			return HitMontage_Back;
		case EHarmoniaHitDirection::Left:
			return HitMontage_Left;
		case EHarmoniaHitDirection::Right:
			return HitMontage_Right;
		default:
			return HitMontage_Front;
		}
	}
};

// ============================================================================
// Melee Combat Structs
// ============================================================================

/**
 * Combo Attack Step
 * Individual attack in a combo sequence
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaComboAttackStep
{
	GENERATED_BODY()

	// Animation montage for this step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	// Montage section name to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName MontageSectionName = NAME_None;

	// Section to play if combo is not continued (End/Recovery)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName EndSectionName = NAME_None;

	// Attack data override (if empty, uses weapon default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FHarmoniaAttackData AttackDataOverride;

	// Use custom attack data?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bUseAttackDataOverride = false;

	// Damage multiplier for this combo step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageMultiplier = 1.0f;

	// Stamina cost multiplier for this step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaCostMultiplier = 1.0f;

	// Can be canceled into dodge/block?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bCanBeCanceled = true;

	// Minimum time before can be canceled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo", meta = (EditCondition = "bCanBeCanceled"))
	float MinimumCancelTime = 0.2f;
};

/**
 * Combo Attack Sequence
 * Full combo chain for a weapon
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaComboAttackSequence : public FTableRowBase
{
	GENERATED_BODY()

	// Combo name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FText ComboName;

	// Weapon type tag this combo belongs to (e.g., Weapon.Type.Sword)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (Categories = "Weapon.Type"))
	FGameplayTag WeaponTypeTag;

	// Attack type (Light, Heavy, Special, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	EHarmoniaAttackType AttackType = EHarmoniaAttackType::Light;

	// Owner type tag for combo lookup (e.g., Character.Race.Human, Character.Gender.Male)
	// Used to select different animations for different character types
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTag OwnerTypeTag;

	// Combo steps
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TArray<FHarmoniaComboAttackStep> ComboSteps;

	// Gameplay tags required to perform this combo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTagContainer RequiredTags;

	// Gameplay tags blocked by this combo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	FGameplayTagContainer BlockedTags;
};

/**
 * Defense Configuration
 * Defines blocking/parrying properties
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDefenseConfig
{
	GENERATED_BODY()

	// Can block attacks?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	bool bCanBlock = true;

	// Block damage reduction (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (EditCondition = "bCanBlock"))
	float BlockDamageReduction = 0.7f;

	// Stamina cost per blocked hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (EditCondition = "bCanBlock"))
	float BlockStaminaCost = 15.0f;

	// Can parry attacks?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	bool bCanParry = true;

	// Parry window duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (EditCondition = "bCanParry"))
	float ParryWindowDuration = 0.2f;

	// Stamina cost for parry attempt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (EditCondition = "bCanParry"))
	float ParryStaminaCost = 10.0f;

	// Parry success animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> ParrySuccessMontage = nullptr;

	// Parry fail animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> ParryFailMontage = nullptr;

	// Block start animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockStartMontage = nullptr;

	// Block loop animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockLoopMontage = nullptr;

	// Block end animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockEndMontage = nullptr;

	// Block hit reaction animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockHitMontage = nullptr;
};

/**
 * Dodge Configuration
 * Defines dodge/roll properties
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaDodgeConfig
{
	GENERATED_BODY()

	// Dodge type (roll, sidestep, backstep)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	FName DodgeType = FName("Roll");

	// Dodge distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDistance = 400.0f;

	// Dodge duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float DodgeDuration = 0.6f;

	// I-frame start time (seconds into dodge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float IFrameStartTime = 0.1f;

	// I-frame duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	float IFrameDuration = 0.3f;

	// Stamina cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaCost = 20.0f;

	// Dodge animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> DodgeMontage = nullptr;

	// Can attack immediately after dodge?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	bool bCanAttackAfterDodge = true;

	// Minimum time before can attack after dodge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (EditCondition = "bCanAttackAfterDodge"))
	float MinimumAttackDelay = 0.3f;
};

/**
 * Riposte Configuration
 * Defines riposte (counter attack after parry) properties
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaRiposteConfig
{
	GENERATED_BODY()

	// Riposte window duration after successful parry (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte")
	float RiposteWindowDuration = 2.0f;

	// Damage multiplier for riposte attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte")
	float RiposteDamageMultiplier = 3.0f;

	// Stamina cost for riposte
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte")
	float RiposteStaminaCost = 15.0f;

	// Riposte animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte|Animation")
	TObjectPtr<UAnimMontage> RiposteMontage = nullptr;

	// Whether riposte is guaranteed critical
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte")
	bool bGuaranteedCritical = true;

	// Whether to play special animation on target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte|Animation")
	bool bPlayTargetStunAnimation = true;

	// Target stun animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Riposte|Animation", meta = (EditCondition = "bPlayTargetStunAnimation"))
	TObjectPtr<UAnimMontage> TargetStunMontage = nullptr;
};

/**
 * Backstab Configuration
 * Defines backstab (critical attack from behind) properties
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaBackstabConfig
{
	GENERATED_BODY()

	// Enable backstab system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab")
	bool bEnableBackstab = true;

	// Maximum angle from behind to trigger backstab (degrees)
	// 0 = must be exactly behind, 180 = from any direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab", meta = (EditCondition = "bEnableBackstab", ClampMin = "0.0", ClampMax = "180.0"))
	float BackstabAngleTolerance = 45.0f;

	// Maximum distance for backstab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab", meta = (EditCondition = "bEnableBackstab"))
	float BackstabMaxDistance = 150.0f;

	// Damage multiplier for backstab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab", meta = (EditCondition = "bEnableBackstab"))
	float BackstabDamageMultiplier = 4.0f;

	// Whether backstab is guaranteed critical
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab", meta = (EditCondition = "bEnableBackstab"))
	bool bGuaranteedCritical = true;

	// Backstab animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab|Animation", meta = (EditCondition = "bEnableBackstab"))
	TObjectPtr<UAnimMontage> BackstabMontage = nullptr;

	// Whether to play special animation on target
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab|Animation", meta = (EditCondition = "bEnableBackstab"))
	bool bPlayTargetAnimation = true;

	// Target backstab animation montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstab|Animation", meta = (EditCondition = "bEnableBackstab && bPlayTargetAnimation"))
	TObjectPtr<UAnimMontage> TargetBackstabMontage = nullptr;
};

// ============================================================================
// Ranged Combat Structs & Enums
// ============================================================================

/**
 * Ranged Weapon Type
 * Defines different categories of ranged weapons
 */
UENUM(BlueprintType)
enum class EHarmoniaRangedWeaponType : uint8
{
	None UMETA(DisplayName = "None"),

	// Bow & Crossbow
	Bow UMETA(DisplayName = "Bow"),								// Traditional bow
	Longbow UMETA(DisplayName = "Longbow"),						// High damage, slow draw
	Shortbow UMETA(DisplayName = "Shortbow"),					// Fast draw, low damage
	CompositeBow UMETA(DisplayName = "Composite Bow"),			// Balanced
	Crossbow UMETA(DisplayName = "Crossbow"),					// High accuracy, slow reload
	HeavyCrossbow UMETA(DisplayName = "Heavy Crossbow"),		// Armor piercing
	RepeatingCrossbow UMETA(DisplayName = "Repeating Crossbow"),// Multi-shot

	// Firearms (Dark Fantasy)
	Pistol UMETA(DisplayName = "Pistol"),						// Single shot pistol
	DualPistols UMETA(DisplayName = "Dual Pistols"),			// Twin pistols
	Rifle UMETA(DisplayName = "Rifle"),							// Long range rifle
	Shotgun UMETA(DisplayName = "Shotgun"),						// Spread shot
	Musket UMETA(DisplayName = "Musket"),						// Heavy single shot

	// Throwing Weapons
	ThrowingKnife UMETA(DisplayName = "Throwing Knife"),		// Fast, low damage
	Shuriken UMETA(DisplayName = "Shuriken"),					// Multi-projectile
	ThrowingAxe UMETA(DisplayName = "Throwing Axe"),			// High damage
	Javelin UMETA(DisplayName = "Javelin"),						// Long range spear
	Bomb UMETA(DisplayName = "Bomb"),							// Explosive
	PoisonVial UMETA(DisplayName = "Poison Vial"),				// Status effect

	// Magic Weapons
	Staff UMETA(DisplayName = "Staff"),							// Basic magic weapon
	Wand UMETA(DisplayName = "Wand"),							// Fast casting
	Tome UMETA(DisplayName = "Tome"),							// Multiple spells
	Catalyst UMETA(DisplayName = "Catalyst"),					// Faith-based
	Orb UMETA(DisplayName = "Orb"),								// Intelligence-based

	Custom UMETA(DisplayName = "Custom")						// User-defined
};

/**
 * Projectile Type
 * Defines different types of projectiles and their behavior
 */
UENUM(BlueprintType)
enum class EHarmoniaProjectileType : uint8
{
	// Physical Projectiles
	Arrow UMETA(DisplayName = "Arrow"),							// Standard arrow
	Bolt UMETA(DisplayName = "Bolt"),							// Crossbow bolt
	Bullet UMETA(DisplayName = "Bullet"),						// Firearm bullet
	Pellet UMETA(DisplayName = "Pellet"),						// Shotgun pellet

	// Thrown Projectiles
	Knife UMETA(DisplayName = "Knife"),
	Shuriken UMETA(DisplayName = "Shuriken"),
	Axe UMETA(DisplayName = "Axe"),
	Spear UMETA(DisplayName = "Spear"),

	// Explosive Projectiles
	Bomb UMETA(DisplayName = "Bomb"),
	Grenade UMETA(DisplayName = "Grenade"),
	FireBomb UMETA(DisplayName = "Fire Bomb"),

	// Magic Projectiles
	MagicBolt UMETA(DisplayName = "Magic Bolt"),
	Fireball UMETA(DisplayName = "Fireball"),
	IceShard UMETA(DisplayName = "Ice Shard"),
	LightningBolt UMETA(DisplayName = "Lightning Bolt"),
	DarkOrb UMETA(DisplayName = "Dark Orb"),
	HolyBeam UMETA(DisplayName = "Holy Beam"),

	// Hitscan (Instant)
	Hitscan UMETA(DisplayName = "Hitscan"),						// Instant hit

	Custom UMETA(DisplayName = "Custom")
};

/**
 * Spell Element Type
 * Defines elemental types for magic
 */
UENUM(BlueprintType)
enum class EHarmoniaSpellElement : uint8
{
	None UMETA(DisplayName = "None"),
	Fire UMETA(DisplayName = "Fire"),							// Damage over time
	Ice UMETA(DisplayName = "Ice"),								// Slow/freeze
	Lightning UMETA(DisplayName = "Lightning"),					// High damage, chain
	Dark UMETA(DisplayName = "Dark"),							// Debuff/curse
	Holy UMETA(DisplayName = "Holy"),							// Heal/buff
	Arcane UMETA(DisplayName = "Arcane"),						// Pure magic
	Poison UMETA(DisplayName = "Poison"),						// DoT
	Blood UMETA(DisplayName = "Blood"),							// Life steal
	Earth UMETA(DisplayName = "Earth"),							// Defense/stun
	Wind UMETA(DisplayName = "Wind"),							// Knockback/mobility
};

/**
 * Aiming Mode
 * Defines how aiming is performed
 */
UENUM(BlueprintType)
enum class EHarmoniaAimingMode : uint8
{
	None UMETA(DisplayName = "No Aiming"),						// Fire straight
	Soft UMETA(DisplayName = "Soft Aim"),						// Slight aim assist
	Precision UMETA(DisplayName = "Precision Aim"),				// Full aim mode (zoom)
	LockOn UMETA(DisplayName = "Lock-On Target"),				// Auto-aim to locked target
	OverTheShoulder UMETA(DisplayName = "Over-The-Shoulder"),	// TPS aiming
};

/**
 * Projectile Movement Type
 * Defines how projectiles move through space
 */
UENUM(BlueprintType)
enum class EHarmoniaProjectileMovement : uint8
{
	Ballistic UMETA(DisplayName = "Ballistic"),					// Affected by gravity
	Straight UMETA(DisplayName = "Straight"),					// Linear movement
	Homing UMETA(DisplayName = "Homing"),						// Seeks target
	Parabolic UMETA(DisplayName = "Parabolic"),					// Arc trajectory
	Boomerang UMETA(DisplayName = "Boomerang"),					// Returns to owner
	Hitscan UMETA(DisplayName = "Hitscan"),						// Instant
};

/**
 * Projectile Data
 * Defines properties of a projectile
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaProjectileData
{
	GENERATED_BODY()

	// Projectile type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EHarmoniaProjectileType ProjectileType = EHarmoniaProjectileType::Arrow;

	// Movement type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EHarmoniaProjectileMovement MovementType = EHarmoniaProjectileMovement::Ballistic;

	// Initial speed (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float InitialSpeed = 3000.0f;

	// Maximum speed (cm/s, 0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxSpeed = 0.0f;

	// Gravity scale (1.0 = normal gravity, 0 = no gravity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityScale = 1.0f;

	// Homing acceleration (for homing projectiles)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (EditCondition = "MovementType == EHarmoniaProjectileMovement::Homing"))
	float HomingAcceleration = 1000.0f;

	// Lifetime (seconds, 0 = infinite)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Lifetime = 10.0f;

	// Damage configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	FHarmoniaDamageEffectConfig DamageConfig;

	// Hit reaction configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction")
	FHarmoniaHitReactionConfig HitReactionConfig;

	// Penetration count (how many targets can it pierce)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 PenetrationCount = 0;

	// Damage falloff per penetration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float PenetrationDamageFalloff = 0.5f;

	// Bounce count (how many times can it bounce)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 BounceCount = 0;

	// Bounce velocity retention (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float BounceVelocityRetention = 0.6f;

	// Stick to surfaces on hit?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bStickToSurfaces = false;

	// Explode on impact?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bExplodeOnImpact = false;

	// Explosion radius (if explosive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion", meta = (EditCondition = "bExplodeOnImpact"))
	float ExplosionRadius = 500.0f;

	// Mesh to use for projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UStaticMesh> ProjectileMesh;

	// Particle trail effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UParticleSystem> TrailEffect;

	// Impact effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UParticleSystem> ImpactEffect;

	// Impact sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<USoundBase> ImpactSound;

	// Gameplay Cue for impact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Cue")
	FGameplayTag ImpactGameplayCueTag;
};

/**
 * Spell Data
 * Defines properties of a magic spell
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaSpellData : public FTableRowBase
{
	GENERATED_BODY()

	// Spell name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	FText SpellName;

	// Spell description
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	FText Description;

	// Element type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	EHarmoniaSpellElement Element = EHarmoniaSpellElement::Arcane;

	// Mana cost
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	float ManaCost = 20.0f;

	// Cast time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	float CastTime = 1.0f;

	// Cooldown (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	float Cooldown = 5.0f;

	// Projectile data (if projectile-based spell)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	FHarmoniaProjectileData ProjectileData;

	// Use projectile?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	bool bUsesProjectile = true;

	// Intelligence requirement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 IntelligenceRequirement = 0;

	// Faith requirement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Requirements")
	int32 FaithRequirement = 0;

	// Cast animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> CastMontage;

	// Gameplay Effect to apply (for buffs/healing)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	TSubclassOf<UGameplayEffect> SpellEffectClass;

	// Can be cast while moving?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	bool bCanCastWhileMoving = false;

	// Can be interrupted?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	bool bCanBeInterrupted = true;

	// Charge levels (0 = no charging)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell")
	int32 MaxChargeLevel = 0;

	// Damage multiplier per charge level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spell", meta = (EditCondition = "MaxChargeLevel > 0"))
	float ChargeMultiplier = 1.5f;
};
