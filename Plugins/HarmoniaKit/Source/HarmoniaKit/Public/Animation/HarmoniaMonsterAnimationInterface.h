// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "HarmoniaMonsterAnimationInterface.generated.h"

class UAnimMontage;

/**
 * Monster Animation Type
 * Used to identify different animation categories for various creature types
 */
UENUM(BlueprintType)
enum class EHarmoniaMonsterAnimationType : uint8
{
	// Locomotion
	Idle UMETA(DisplayName = "Idle"),
	Walk UMETA(DisplayName = "Walk"),
	Run UMETA(DisplayName = "Run"),
	Jump UMETA(DisplayName = "Jump"),
	Fall UMETA(DisplayName = "Fall"),
	Land UMETA(DisplayName = "Land"),

	// Combat
	Attack_Light UMETA(DisplayName = "Light Attack"),
	Attack_Heavy UMETA(DisplayName = "Heavy Attack"),
	Attack_Special UMETA(DisplayName = "Special Attack"),
	Block UMETA(DisplayName = "Block"),
	Dodge UMETA(DisplayName = "Dodge"),

	// Reactions
	HitReaction_Front UMETA(DisplayName = "Hit Front"),
	HitReaction_Back UMETA(DisplayName = "Hit Back"),
	HitReaction_Left UMETA(DisplayName = "Hit Left"),
	HitReaction_Right UMETA(DisplayName = "Hit Right"),
	Stunned UMETA(DisplayName = "Stunned"),

	// States
	Death UMETA(DisplayName = "Death"),
	Spawn UMETA(DisplayName = "Spawn"),
	Roar UMETA(DisplayName = "Roar/Taunt"),

	// Boss Specific
	PhaseTransition UMETA(DisplayName = "Phase Transition"),
	UltimateAttack UMETA(DisplayName = "Ultimate Attack"),

	// Custom
	Custom UMETA(DisplayName = "Custom")
};

UINTERFACE(MinimalAPI, BlueprintType)
class UHarmoniaMonsterAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * IHarmoniaMonsterAnimationInterface
 *
 * Interface for monster Animation Blueprints
 * Allows different creature types (humanoid, quadruped, flying, mechanical, etc.)
 * to have their own animation BP while maintaining a common interface
 *
 * This should be implemented in Animation Blueprint classes, NOT character classes
 */
class HARMONIAKIT_API IHarmoniaMonsterAnimationInterface
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Animation State
	// ============================================================================

	/**
	 * Play an animation by type
	 * @param AnimationType Type of animation to play
	 * @param PlayRate Speed multiplier for the animation
	 * @return true if animation was successfully triggered
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	bool PlayMonsterAnimation(EHarmoniaMonsterAnimationType AnimationType, float PlayRate = 1.0f);

	/**
	 * Play a custom animation montage
	 * @param Montage Montage to play
	 * @param PlayRate Speed multiplier
	 * @return true if montage was successfully played
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	bool PlayCustomMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

	/**
	 * Stop current animation montage
	 * @param BlendOutTime Time to blend out
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	void StopMonsterAnimation(float BlendOutTime = 0.25f);

	/**
	 * Get the current animation type being played
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	EHarmoniaMonsterAnimationType GetCurrentAnimationType() const;

	// ============================================================================
	// Animation Parameters (for blendspaces, state machines)
	// ============================================================================

	/**
	 * Update movement speed parameter
	 * @param Speed Current movement speed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	void UpdateMovementSpeed(float Speed);

	/**
	 * Update movement direction parameter
	 * @param Direction Movement direction relative to rotation
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	void UpdateMovementDirection(float Direction);

	/**
	 * Update is in air parameter
	 * @param bInAir Is character in air
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	void UpdateIsInAir(bool bInAir);

	/**
	 * Update is in combat parameter
	 * @param bInCombat Is character in combat
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	void UpdateIsInCombat(bool bInCombat);

	/**
	 * Update monster state
	 * @param State Current monster state
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation")
	void UpdateMonsterState(EHarmoniaMonsterState State);

	// ============================================================================
	// Special Creature-Type Specific Animations
	// ============================================================================

	/**
	 * For flying creatures - update hover/flight state
	 * @param bIsFlying Is creature currently flying
	 * @param FlightSpeed Current flight speed
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation|Flight")
	void UpdateFlightState(bool bIsFlying, float FlightSpeed);

	/**
	 * For multi-limbed creatures - trigger specific limb attack
	 * @param LimbIndex Which limb is attacking (0-N)
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation|Combat")
	void TriggerLimbAttack(int32 LimbIndex);

	/**
	 * For creatures with special abilities - trigger ability animation
	 * @param AbilityName Name/ID of the ability
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Monster Animation|Abilities")
	void TriggerAbilityAnimation(FName AbilityName);
};
