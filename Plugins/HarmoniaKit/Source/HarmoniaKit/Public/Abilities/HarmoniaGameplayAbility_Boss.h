// Copyright 2024 Snow Game Studio.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Boss.generated.h"

class AHarmoniaBossMonster;

/**
 * UHarmoniaGameplayAbility_Boss
 *
 * Base class for boss-specific gameplay abilities.
 * Provides helper functions for accessing boss character and components.
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.6.1 for tag configuration
 */
UCLASS(Abstract)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Boss : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Boss(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Get the boss character that owns this ability */
	UFUNCTION(BlueprintPure, Category = "Boss|Ability")
	AHarmoniaBossMonster* GetBossCharacter() const;

	/** Get current boss phase */
	UFUNCTION(BlueprintPure, Category = "Boss|Ability")
	int32 GetBossPhase() const;

	/** Check if ability should only work in specific phases */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** Phases in which this ability can be activated (empty = all phases) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Ability")
	TArray<int32> ValidPhases;

	/** Minimum boss health percentage required to use this ability (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Ability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinHealthPercent = 0.0f;

	/** Maximum boss health percentage to use this ability (0.0 - 1.0, 0 = no limit) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Ability", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxHealthPercent = 0.0f;
};
