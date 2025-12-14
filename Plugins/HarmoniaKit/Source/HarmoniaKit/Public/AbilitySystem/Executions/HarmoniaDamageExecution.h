// Copyright 2025 Snow Game Studio.

#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "HarmoniaDamageExecution.generated.h"

/**
 * UHarmoniaDamageExecution
 *
 * Execution calculation for damage processing.
 * Captures BaseDamage from source's CombatSet, applies Defense reduction,
 * and outputs to ULyraHealthSet::Damage attribute.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UHarmoniaDamageExecution();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
