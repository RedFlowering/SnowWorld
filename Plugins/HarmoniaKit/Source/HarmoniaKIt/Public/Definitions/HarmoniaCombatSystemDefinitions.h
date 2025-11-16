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
