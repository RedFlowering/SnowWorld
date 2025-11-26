// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_SwarmBehavior.generated.h"

/**
 * Swarm Monster Passive Ability
 * - Weak individually but strong in groups
 * - Attack/Defense scales with nearby swarm members
 * - Fear state when allies die
 *
 * ============================================================================
 * Required Tag Configuration (set in Blueprint or derived class):
 * ============================================================================
 *
 * AbilityTags:
 *   - Ability.Monster.Swarm.Passive (identifies this ability)
 *
 * ActivationPolicy: OnSpawn (auto-activates when granted)
 * ActivationGroup: Independent (can run alongside other abilities)
 *
 * ActivationOwnedTags (tags applied while in swarm):
 *   - State.Swarm.Active (permanent while ability is active)
 *   - State.Swarm.Empowered (when swarm bonus is active)
 *   - State.Swarm.Fearful (when ally dies nearby)
 *
 * ActivationBlockedTags:
 *   - (none - passive always active)
 *
 * Note: This is a passive ability that continuously monitors
 * nearby swarm members and applies dynamic stat modifiers.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGameplayAbility_SwarmBehavior : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_SwarmBehavior();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Range to detect swarm members */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	float SwarmDetectionRadius = 500.0f;

	/** Bonus per nearby swarm member */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	float AttackBonusPerMember = 0.15f;

	/** Defense bonus per nearby swarm member */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	float DefenseBonusPerMember = 0.1f;

	/** Speed bonus when in swarm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	float SpeedBonusPerMember = 0.05f;

	/** Fear duration when ally dies nearby (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	float FearDuration = 3.0f;

	/** Minimum swarm size before bonuses apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	int32 MinimumSwarmSize = 2;

	/** Maximum swarm members to count for bonuses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	int32 MaximumSwarmSize = 10;

	/** Update interval for swarm detection (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	float UpdateInterval = 1.0f;

	/** Gameplay effect for swarm bonuses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	TSubclassOf<UGameplayEffect> SwarmBonusEffect;

	/** Gameplay effect for fear state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swarm")
	TSubclassOf<UGameplayEffect> FearEffect;

	/** Current swarm bonus effect handle */
	FActiveGameplayEffectHandle SwarmBonusHandle;

	/** Current fear effect handle */
	FActiveGameplayEffectHandle FearEffectHandle;

	/** Timer for swarm updates */
	FTimerHandle UpdateTimerHandle;

	/** Currently detected swarm members */
	UPROPERTY()
	TArray<AActor*> SwarmMembers;

	/** Is currently in fear state */
	bool bInFear = false;

	/** Base stat values (stored per instance to avoid static TMap bug) */
	float BaseAttackPower = 0.0f;
	float BaseDefense = 0.0f;
	float BaseMovementSpeed = 0.0f;
	bool bBaseStatsStored = false;

	/** Update swarm bonuses */
	UFUNCTION()
	void UpdateSwarmBonuses();

	/** Apply swarm bonuses based on member count */
	void ApplySwarmBonuses(int32 MemberCount);

	/** Remove swarm bonuses */
	void RemoveSwarmBonuses();

	/** Trigger fear when nearby ally dies */
	UFUNCTION()
	void OnNearbyAllyDied(AActor* DeadAlly);

	/** Apply fear state */
	void ApplyFear();

	/** Remove fear state */
	void RemoveFear();

	/** Get nearby swarm members */
	TArray<AActor*> GetNearbySwarmMembers() const;

	/** Bind to ally death events */
	void BindToAllyDeathEvents();

	/** Unbind from ally death events */
	void UnbindFromAllyDeathEvents();
};
