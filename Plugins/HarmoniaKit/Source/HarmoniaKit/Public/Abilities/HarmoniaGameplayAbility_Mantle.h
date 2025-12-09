// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Mantle.generated.h"

class AHarmoniaCharacter;

/**
 * UHarmoniaGameplayAbility_Mantle
 *
 * Mantle ability that activates during InAir state and checks for mantle opportunities.
 * Uses AbilityTask_WaitDelay for periodic checking.
 *
 * Blueprint Setup:
 * - ActivationRequiredTags: State.InAir (auto-activate when jumping)
 * - ActivationOwnedTags: State.Mantling (applied during mantling)
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_Mantle : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Mantle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Start a WaitDelay task to check mantle on next interval */
	void StartMantleCheckTask();

	/** Callback from WaitDelay task */
	UFUNCTION()
	void OnMantleCheckTimer();

	/** Called when ALS mantling ends */
	UFUNCTION()
	void OnMantlingEnded();

	AHarmoniaCharacter* GetHarmoniaCharacter() const;

protected:
	/** How often to check for mantle opportunities (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	float MantleCheckInterval = 0.05f;

	/** Jump ability class to cancel when mantling starts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	TSubclassOf<UGameplayAbility> JumpAbilityClass;

	/** Sprint ability class to cancel when mantling starts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	TSubclassOf<UGameplayAbility> SprintAbilityClass;

	/** Block ability class to cancel when mantling starts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mantle")
	TSubclassOf<UGameplayAbility> BlockAbilityClass;

private:
	bool bIsMantling = false;
};
