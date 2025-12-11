// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Roll.generated.h"

class AAlsCharacter;

/**
 * UHarmoniaGameplayAbility_Roll
 *
 * Gameplay Ability for ALS Rolling integration (Auto-Landing Roll).
 * Triggered via GameplayEvent when ALS starts rolling (after all ALS checks pass).
 *
 * Behavior:
 * - Activated by GameplayEvent.Roll.Landing (sent from HarmoniaCharacter::NotifyLocomotionActionChanged)
 * - ALS has already started rolling when this GA activates
 * - GA applies stamina cost and I-frames, monitors for roll completion
 * - Uses ActivationOwnedTags for state management (State.Rolling)
 *
 * Flow:
 * 1. ALS validates landing conditions and calls StartRolling()
 * 2. HarmoniaCharacter::NotifyLocomotionActionChanged detects Rolling action → sends GameplayEvent
 * 3. GA activates → applies stamina cost + I-frames
 * 4. Poll LocomotionAction: When no longer Rolling → EndAbility()
 *
 * Blueprint Setup:
 * - Add to AbilitySet (NO Input Tag needed)
 * - Set Ability Triggers: GameplayEvent.Roll.Landing (Gameplay Event)
 * - Set ActivationOwnedTags: State.Rolling
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Roll : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Roll(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Start a WaitDelay task to check roll completion on next interval */
	void StartRollCheckTask();

	/** Callback from WaitDelay task */
	UFUNCTION()
	void OnRollCheckTimer();

	/** Get the ALS character from the owning actor */
	UFUNCTION(BlueprintCallable, Category = "Roll")
	AAlsCharacter* GetAlsCharacter() const;

	/** Gameplay Effect for invincibility frames (optional) */
	UPROPERTY(EditDefaultsOnly, Category = "Roll|IFrames")
	TSubclassOf<UGameplayEffect> IFrameEffectClass;

	/** How often to check for roll completion (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float RollCheckInterval = 0.05f;

	/** Handle for active I-frame effect */
	FActiveGameplayEffectHandle IFrameEffectHandle;
};
