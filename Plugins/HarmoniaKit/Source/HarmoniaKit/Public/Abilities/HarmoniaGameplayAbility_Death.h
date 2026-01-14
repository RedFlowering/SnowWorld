// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Death.generated.h"

class UAnimMontage;
class APlayerCameraManager;

/**
 * Death Gameplay Ability for Souls-like games
 * 
 * Handles the death sequence:
 * - Disable player input
 * - Play death montage
 * - Transition to stable ragdoll (no physics impulse reactions)
 * - Fade out screen
 * - Trigger respawn ability
 *
 * Features:
 * - Stable ragdoll: bEnableGravity = true, collision responses disabled
 * - Configurable delays for each phase
 * - Broadcasts OnDeathSequenceComplete when ready for respawn
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md for death system config
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Death : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Death(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	// ============================================================================
	// Death Montage Callbacks
	// ============================================================================

	/** Called when death montage completes */
	UFUNCTION()
	void OnMontageCompleted();

	/** Called when death montage is interrupted */
	UFUNCTION()
	void OnMontageInterrupted();

	// ============================================================================
	// Death Sequence Phases
	// ============================================================================

	/** Start ragdoll physics (stable - no impulse reactions) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death")
	void ActivateStableRagdoll();

	/** Start fade out sequence */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death")
	void StartFadeOut();

	/** Called when death sequence is complete and ready for respawn */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death")
	void OnDeathSequenceComplete();

	/** Trigger respawn ability (GA_Respawn) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Death")
	void TriggerRespawnAbility();

protected:
	// ============================================================================
	// Configuration
	// ============================================================================

	/** Death animation montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** Whether to enable ragdoll after death montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Ragdoll")
	bool bEnableRagdoll = true;

	/** Delay before activating ragdoll after montage ends (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Ragdoll", meta = (EditCondition = "bEnableRagdoll"))
	float RagdollDelay = 0.1f;

	/** Duration to stay in ragdoll before fade out (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Ragdoll", meta = (EditCondition = "bEnableRagdoll"))
	float RagdollDuration = 2.0f;

	/** Duration of screen fade out (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Fade")
	float FadeOutDuration = 1.0f;

	/** Color to fade to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Fade")
	FLinearColor FadeColor = FLinearColor::Black;

	/** Delay after fade completes before triggering respawn (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Respawn")
	float RespawnDelay = 0.5f;

	/** Whether to automatically trigger respawn ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Respawn")
	bool bAutoTriggerRespawn = true;

	/** Tag to use for triggering respawn ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Death|Respawn", meta = (EditCondition = "bAutoTriggerRespawn"))
	FGameplayTag RespawnAbilityTag;

	// ============================================================================
	// Runtime State
	// ============================================================================

	/** Timer for ragdoll activation */
	FTimerHandle RagdollTimerHandle;

	/** Timer for fade out start */
	FTimerHandle FadeOutTimerHandle;

	/** Timer for respawn trigger */
	FTimerHandle RespawnTimerHandle;

	/** Cached skeletal mesh for ragdoll */
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> CachedMesh;

private:
	/** Disable player input */
	void DisablePlayerInput();

	/** Store original mesh collision settings for restoration */
	void StoreOriginalCollisionSettings();

	/** Apply stable ragdoll settings (no impulse reactions) */
	void ApplyStableRagdollSettings(USkeletalMeshComponent* Mesh);
};
