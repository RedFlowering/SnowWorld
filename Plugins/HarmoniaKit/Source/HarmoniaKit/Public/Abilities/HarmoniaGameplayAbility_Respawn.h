// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Respawn.generated.h"

class APlayerCameraManager;
class UGameplayEffect;

/**
 * Respawn Gameplay Ability for Souls-like games
 * 
 * Handles the respawn sequence:
 * - Teleport to last checkpoint
 * - Restore health/stamina
 * - Apply optional ethereal penalties
 * - Re-enable player input
 * - Fade in screen
 *
 * Features:
 * - All penalties are optional (configurable)
 * - Works with HarmoniaCheckpointSubsystem
 * - Works with HarmoniaDeathPenaltyComponent
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md for respawn system config
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Respawn : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Respawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	// ============================================================================
	// Respawn Sequence
	// ============================================================================

	/** Teleport player to checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void TeleportToCheckpoint();

	/** Restore player stats (health, stamina) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void RestorePlayerStats();

	/** Apply death penalties (if enabled) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void ApplyDeathPenalties();

	/** Reset character physics/collision state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void ResetCharacterState();

	/** Start fade in sequence */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void StartFadeIn();

	/** Re-enable player input */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void EnablePlayerInput();

	/** Called when respawn sequence is complete */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Respawn")
	void OnRespawnComplete();

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/** Duration of screen fade in (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Fade")
	float FadeInDuration = 1.0f;

	/** Delay before enabling input after fade in (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Fade")
	float InputEnableDelay = 0.5f;

	/** Whether to restore health to max on respawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Stats")
	bool bRestoreHealthToMax = true;

	/** Whether to restore stamina to max on respawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Stats")
	bool bRestoreStaminaToMax = true;

	/** Whether to restore mana to max on respawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Stats")
	bool bRestoreManaToMax = true;

	/** Whether to restore poise to max on respawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Stats")
	bool bRestorePoiseToMax = true;

	/** GameplayEffect to apply for restoring stats (optional, if not set will use code-based restoration) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Stats")
	TSubclassOf<UGameplayEffect> RestoreStatsEffect;

	/** Whether to apply death penalties (uses HarmoniaDeathPenaltyComponent) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Penalties")
	bool bApplyDeathPenalties = true;

	/** Whether to use checkpoint system for respawn location */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Location")
	bool bUseCheckpointSystem = true;

	/** Fallback respawn location if no checkpoint is available */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn|Location")
	FVector FallbackRespawnLocation = FVector::ZeroVector;

	// ============================================================================
	// Runtime State
	// ============================================================================

	/** Timer for fade in completion */
	FTimerHandle FadeInTimerHandle;

	/** Timer for input enable */
	FTimerHandle InputEnableTimerHandle;

private:
	/** Get checkpoint location from subsystem */
	bool GetCheckpointLocation(FVector& OutLocation, FRotator& OutRotation) const;
};
