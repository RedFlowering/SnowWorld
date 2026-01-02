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
 * NOTE: 페이즈/HP 조건은 PatternComponent에서 관리합니다.
 * - ValidPhases는 FBossAttackPattern.ValidPhases 사용
 * - HP 조건은 GameplayEffect의 Cost/Condition 사용
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
};
