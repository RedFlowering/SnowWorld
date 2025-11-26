// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaGameplayAbility_HitReaction.generated.h"

class UAnimMontage;

/**
 * UHarmoniaGameplayAbility_HitReaction
 *
 * Gameplay Ability that handles hit reactions when character receives damage
 * - Plays directional hit animations
 * - Applies hit stun
 * - Manages movement speed reduction
 * - Can be interrupted based on configuration
 *
 * Activation:
 * - Automatically triggered by GameplayEvent.HitReaction
 * - Can be activated via Blueprint
 *
 * Usage:
 * 1. Create DataTable with FHitReactionData rows
 * 2. Configure animations for each direction (Front/Back/Left/Right)
 * 3. Set stun duration and other parameters
 * 4. Bind to damage received event to trigger automatically
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.6 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_HitReaction : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_HitReaction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	/**
	 * Perform hit reaction
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	virtual void PerformHitReaction(const FGameplayEventData& EventData);

	/**
	 * Calculate hit direction from instigator location
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Ability")
	virtual EHarmoniaHitDirection CalculateHitDirection(const FVector& InstigatorLocation) const;

	/**
	 * Get hit reaction data for specific type
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Ability")
	FHitReactionData GetHitReactionData(EHarmoniaHitReactionType ReactionType) const;

	/**
	 * Called when montage completes
	 */
	UFUNCTION()
	virtual void OnMontageCompleted();

	/**
	 * Called when montage is cancelled
	 */
	UFUNCTION()
	virtual void OnMontageCancelled();

	/**
	 * Called when montage is interrupted
	 */
	UFUNCTION()
	virtual void OnMontageInterrupted();

	/**
	 * Apply movement speed modification
	 */
	virtual void ApplyMovementSpeedModification(float Multiplier);

	/**
	 * Remove movement speed modification
	 */
	virtual void RemoveMovementSpeedModification();

protected:
	/**
	 * DataTable containing hit reaction data
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	TObjectPtr<UDataTable> HitReactionDataTable = nullptr;

	/**
	 * Default hit reaction type when no specific type is provided
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	EHarmoniaHitReactionType DefaultReactionType = EHarmoniaHitReactionType::Light;

	/**
	 * Whether to automatically determine hit direction
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	bool bAutoDetectHitDirection = true;

	/**
	 * Angle threshold for front/back detection (degrees)
	 * 0-45 = Front, 135-180 = Back, etc.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	float DirectionAngleThreshold = 45.0f;

	/**
	 * Whether to allow interrupting current hit reaction
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	bool bAllowInterruption = true;

	/**
	 * Minimum time before hit reaction can be interrupted (seconds)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (EditCondition = "bAllowInterruption"))
	float MinimumInterruptTime = 0.1f;

	/**
	 * Whether to apply invincibility frames during hit reaction
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction")
	bool bApplyInvincibilityFrames = false;

	/**
	 * Duration of invincibility frames (seconds)
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hit Reaction", meta = (EditCondition = "bApplyInvincibilityFrames"))
	float InvincibilityDuration = 0.5f;

	// ============================================================================
	// Runtime State
	// ============================================================================

	/**
	 * Current hit reaction data being used
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Hit Reaction")
	FHitReactionData CurrentReactionData;

	/**
	 * Current hit direction
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Hit Reaction")
	EHarmoniaHitDirection CurrentHitDirection = EHarmoniaHitDirection::Front;

	/**
	 * Time when hit reaction started
	 */
	float HitReactionStartTime = 0.0f;

	/**
	 * Timer handle for stun duration
	 */
	FTimerHandle StunTimerHandle;

	/**
	 * Timer handle for invincibility duration
	 */
	FTimerHandle InvincibilityTimerHandle;

	/**
	 * Active gameplay effect handle for movement speed modification
	 */
	FActiveGameplayEffectHandle MovementSpeedEffectHandle;
};
