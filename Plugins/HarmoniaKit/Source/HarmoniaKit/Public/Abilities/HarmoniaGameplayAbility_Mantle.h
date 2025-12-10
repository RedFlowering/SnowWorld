// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Mantle.generated.h"

class AHarmoniaCharacter;

/**
 * UHarmoniaGameplayAbility_Mantle (MantleMonitor)
 *
 * Monitors for mantle opportunities during InAir state.
 * When mantling is detected, triggers GA_MantleExecute via GameplayEvent.
 * Automatically ends MantleExecute when mantling animation completes.
 *
 * Blueprint Setup (GA_Harmonia_Mantle):
 * - Ability Triggers: State.InAir (Owned Tag Present)
 * - MantleExecuteEventTag: Event.Mantle.Execute
 * - MantleExecuteAbilityTag: Ability.Mantle
 *
 * Requires separate GA_MantleExecute blueprint with:
 * - Ability Tags: Ability.Mantle
 * - Ability Triggers: Event.Mantle.Execute (On Gameplay Event)
 * - Cancel Abilities With Tag: Ability.Sprint, Ability.Block, Ability.Jump
 * - Activation Owned Tags: State.Mantling
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Mantle : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Mantle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanBeCanceled() const override;

protected:
	/** Start a WaitDelay task to check mantle on next interval */
	void StartMantleCheckTask();

	/** Callback from WaitDelay task */
	UFUNCTION()
	void OnMantleCheckTimer();

	/** Force cancel MantleExecute ability to remove State.Mantling tag */
	void CancelMantleExecuteAbility();

	AHarmoniaCharacter* GetHarmoniaCharacter() const;

protected:
	/** How often to check for mantle opportunities (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	float MantleCheckInterval = 0.05f;

	/** Event tag to trigger MantleExecute ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	FGameplayTag MantleExecuteEventTag;

	/** Ability tag used to identify and cancel MantleExecute */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	FGameplayTag MantleExecuteAbilityTag;

private:
	bool bIsMantling = false;
};
