// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "HarmoniaCombatLibrary.generated.h"

class AActor;
class UAbilitySystemComponent;
struct FHitResult;

/**
 * Combat Library
 * 
 * Static utility functions for combat-related operations.
 * Provides helper functions that are commonly used across multiple combat components.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Direction Calculations
	// ============================================================================

	/**
	 * Calculate if an attack is from behind the target
	 * @param Target The target being attacked
	 * @param AttackOrigin The origin of the attack
	 * @param AngleTolerance Maximum angle from behind to consider (0-180 degrees)
	 * @return True if attack is from behind within tolerance
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static bool IsAttackFromBehind(AActor* Target, FVector AttackOrigin, float AngleTolerance = 45.0f);

	/**
	 * Get hit direction relative to target
	 * @param Target The target being hit
	 * @param HitLocation The location of the hit
	 * @return Direction enum (Front, Back, Left, Right)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static EHarmoniaHitDirection GetHitDirection(AActor* Target, FVector HitLocation);

	/**
	 * Calculate angle between two vectors in degrees (0-180)
	 * @param VectorA First vector
	 * @param VectorB Second vector
	 * @param bIgnoreZ If true, ignores Z component
	 * @return Angle in degrees
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static float GetAngleBetweenVectors(FVector VectorA, FVector VectorB, bool bIgnoreZ = true);

	// ============================================================================
	// Combat State Queries
	// ============================================================================

	/**
	 * Check if an actor can be attacked
	 * @param Attacker The attacking actor
	 * @param Target The potential target
	 * @return True if target can be attacked
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static bool CanAttackTarget(AActor* Attacker, AActor* Target);

	/**
	 * Check if actor is in invulnerable state (has invulnerable tag)
	 * @param Actor The actor to check
	 * @return True if invulnerable
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static bool IsInvulnerable(AActor* Actor);

	/**
	 * Check if actor is currently attacking (has attacking tag)
	 * @param Actor The actor to check
	 * @return True if attacking
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static bool IsAttacking(AActor* Actor);

	// ============================================================================
	// Damage Calculations
	// ============================================================================

	/**
	 * Calculate damage with multipliers
	 * @param BaseDamage The base damage value
	 * @param DamageMultiplier Damage multiplier
	 * @param bIsCritical Whether this is a critical hit
	 * @param CriticalMultiplier Critical hit multiplier
	 * @return Final damage amount
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Damage")
	static float CalculateDamage(float BaseDamage, float DamageMultiplier = 1.0f, bool bIsCritical = false, float CriticalMultiplier = 2.0f);

	/**
	 * Calculate damage with block reduction
	 * @param Damage The incoming damage
	 * @param bIsBlocked Whether the attack was blocked
	 * @param BlockReduction Block damage reduction (0-1)
	 * @return Final damage after block reduction
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Damage")
	static float ApplyBlockReduction(float Damage, bool bIsBlocked, float BlockReduction = 0.7f);

	/**
	 * Roll for critical hit
	 * @param CriticalChance Chance to crit (0-1)
	 * @param bGuaranteedCrit Force critical hit
	 * @return True if critical hit
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Damage")
	static bool RollForCritical(float CriticalChance = 0.1f, bool bGuaranteedCrit = false);

	// ============================================================================
	// Hit Detection Helpers
	// ============================================================================

	/**
	 * Perform a box sweep for melee hit detection
	 * @param World World context
	 * @param Origin Origin point of the sweep
	 * @param Direction Direction to sweep
	 * @param Distance Distance to sweep
	 * @param BoxExtent Half extents of the box
	 * @param Rotation Rotation of the box
	 * @param IgnoredActors Actors to ignore
	 * @param OutHits Array of hit results
	 * @param bDebug Draw debug visualization
	 * @return True if any hits were detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Hit Detection", meta = (WorldContext = "World"))
	static bool BoxSweepForHits(
		UObject* World,
		FVector Origin,
		FVector Direction,
		float Distance,
		FVector BoxExtent,
		FRotator Rotation,
		const TArray<AActor*>& IgnoredActors,
		TArray<FHitResult>& OutHits,
		bool bDebug = false
	);

	/**
	 * Perform a sphere sweep for melee hit detection
	 * @param World World context
	 * @param Origin Origin point of the sweep
	 * @param Direction Direction to sweep
	 * @param Distance Distance to sweep
	 * @param Radius Radius of the sphere
	 * @param IgnoredActors Actors to ignore
	 * @param OutHits Array of hit results
	 * @param bDebug Draw debug visualization
	 * @return True if any hits were detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Hit Detection", meta = (WorldContext = "World"))
	static bool SphereSweepForHits(
		UObject* World,
		FVector Origin,
		FVector Direction,
		float Distance,
		float Radius,
		const TArray<AActor*>& IgnoredActors,
		TArray<FHitResult>& OutHits,
		bool bDebug = false
	);

	// ============================================================================
	// Gameplay Tag Helpers
	// ============================================================================

	/**
	 * Add gameplay tag to actor's ASC
	 * @param Actor The actor
	 * @param Tag Tag to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Tags")
	static void AddGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * Remove gameplay tag from actor's ASC
	 * @param Actor The actor
	 * @param Tag Tag to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Tags")
	static void RemoveGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * Check if actor has gameplay tag
	 * @param Actor The actor
	 * @param Tag Tag to check
	 * @return True if actor has the tag
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat|Tags")
	static bool HasGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * Get ASC from actor
	 * @param Actor The actor
	 * @return Ability System Component, or nullptr if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Combat")
	static UAbilitySystemComponent* GetAbilitySystemFromActor(AActor* Actor);

	// ============================================================================
	// Animation Helpers
	// ============================================================================

	/**
	 * Play montage on actor's mesh
	 * @param Actor The actor
	 * @param Montage Montage to play
	 * @param PlayRate Play rate
	 * @param SectionName Optional section name to jump to
	 * @return Duration of the montage
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Animation")
	static float PlayMontageOnActor(AActor* Actor, UAnimMontage* Montage, float PlayRate = 1.0f, FName SectionName = NAME_None);

	/**
	 * Stop montage on actor's mesh
	 * @param Actor The actor
	 * @param Montage Montage to stop (if nullptr, stops current montage)
	 * @param BlendOutTime Blend out time
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Combat|Animation")
	static void StopMontageOnActor(AActor* Actor, UAnimMontage* Montage = nullptr, float BlendOutTime = 0.25f);
};
