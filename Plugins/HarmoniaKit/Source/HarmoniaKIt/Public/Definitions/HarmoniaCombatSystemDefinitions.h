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
 * Melee Weapon Type
 * Soul-like melee weapon categories
 */
UENUM(BlueprintType)
enum class EHarmoniaMeleeWeaponType : uint8
{
	None UMETA(DisplayName = "None"),
	Sword UMETA(DisplayName = "Sword"),						// Balanced weapon
	GreatSword UMETA(DisplayName = "Great Sword"),			// Slow, powerful
	Dagger UMETA(DisplayName = "Dagger"),					// Fast, low damage
	Axe UMETA(DisplayName = "Axe"),							// High damage, slow
	Spear UMETA(DisplayName = "Spear"),						// Long reach
	Hammer UMETA(DisplayName = "Hammer"),					// High poise damage
	Katana UMETA(DisplayName = "Katana"),					// Fast combos
	Shield UMETA(DisplayName = "Shield"),					// Defensive
	Fist UMETA(DisplayName = "Fist"),						// Unarmed
	Whip UMETA(DisplayName = "Whip"),						// Unique range
	Scythe UMETA(DisplayName = "Scythe")					// Wide sweeping
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
	Stunned UMETA(DisplayName = "Stunned")					// Cannot defend
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

	// Base damage amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 10.0f;

	// Damage multiplier (applied to base damage)
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
	TObjectPtr<UAnimMontage> HitMontage_Front;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Back;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitMontage_Right;

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
 * Melee Weapon Data
 * Defines properties for a melee weapon type
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaMeleeWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	// Weapon type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	EHarmoniaMeleeWeaponType WeaponType = EHarmoniaMeleeWeaponType::Sword;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	// Base damage multiplier for this weapon type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float BaseDamageMultiplier = 1.0f;

	// Attack speed multiplier (higher = faster)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackSpeedMultiplier = 1.0f;

	// Stamina cost per light attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float LightAttackStaminaCost = 10.0f;

	// Stamina cost per heavy attack
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stamina")
	float HeavyAttackStaminaCost = 25.0f;

	// Poise damage (ability to stagger enemies)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float PoiseDamage = 10.0f;

	// Default attack trace configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FHarmoniaAttackTraceConfig DefaultTraceConfig;

	// Can this weapon parry?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	bool bCanParry = false;

	// Can this weapon block?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense")
	bool bCanBlock = false;

	// Block damage reduction percentage (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (EditCondition = "bCanBlock", ClampMin = "0.0", ClampMax = "1.0"))
	float BlockDamageReduction = 0.5f;

	// Stamina cost per blocked hit
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defense", meta = (EditCondition = "bCanBlock"))
	float BlockStaminaCost = 15.0f;

	// Maximum combo chain length
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	int32 MaxComboChain = 3;

	// Combo window duration (time to input next attack)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboWindowDuration = 0.5f;
};

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
	TObjectPtr<UAnimMontage> AttackMontage;

	// Montage section name to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName MontageSectionName = NAME_None;

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

	// Weapon type this combo belongs to
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	EHarmoniaMeleeWeaponType WeaponType = EHarmoniaMeleeWeaponType::Sword;

	// Is this a light or heavy combo?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	bool bIsHeavyCombo = false;

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
	TObjectPtr<UAnimMontage> ParrySuccessMontage;

	// Parry fail animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> ParryFailMontage;

	// Block start animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockStartMontage;

	// Block loop animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockLoopMontage;

	// Block end animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockEndMontage;

	// Block hit reaction animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> BlockHitMontage;
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
	TObjectPtr<UAnimMontage> DodgeMontage;

	// Can attack immediately after dodge?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	bool bCanAttackAfterDodge = true;

	// Minimum time before can attack after dodge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (EditCondition = "bCanAttackAfterDodge"))
	float MinimumAttackDelay = 0.3f;
};
