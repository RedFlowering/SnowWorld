// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/Executions/HarmoniaDamageExecution.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/Attributes/LyraCombatSet.h"
#include "AbilitySystemComponent.h"
#include "HarmoniaGameplayTags.h"
#include "HarmoniaLogCategories.h"

struct FHarmoniaDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;
	FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
	FGameplayEffectAttributeCaptureDefinition DefenseDef;

	FHarmoniaDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(
			ULyraCombatSet::GetBaseDamageAttribute(), 
			EGameplayEffectAttributeCaptureSource::Source, 
			true
		);
		
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UHarmoniaAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true
		);
		
		DefenseDef = FGameplayEffectAttributeCaptureDefinition(
			UHarmoniaAttributeSet::GetDefenseAttribute(),
			EGameplayEffectAttributeCaptureSource::Target,
			true
		);
	}
};

static FHarmoniaDamageStatics& DamageStatics()
{
	static FHarmoniaDamageStatics Statics;
	return Statics;
}

UHarmoniaDamageExecution::UHarmoniaDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void UHarmoniaDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UE_LOG(LogHarmoniaCombat, Log, TEXT("[DamageExecution] Execute_Implementation called"));

#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, BaseDamage);

	if (BaseDamage <= 0.0f)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluateParameters, BaseDamage);
	}

	float Defense = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvaluateParameters, Defense);

	float DamageMultiplier = 1.0f;
	const FGameplayTag DamageMultiplierTag = HarmoniaGameplayTags::SetByCaller_DamageMultiplier.GetTag();
	if (DamageMultiplierTag.IsValid())
	{
		DamageMultiplier = Spec.GetSetByCallerMagnitude(DamageMultiplierTag, false, 1.0f);
	}

	const float DamageBeforeDefense = BaseDamage * DamageMultiplier;
	const float FinalDamage = FMath::Max(DamageBeforeDefense - Defense, 0.0f);

	if (FinalDamage > 0.0f)
	{
		// Output to HarmoniaAttributeSet's IncomingDamage (not LyraHealthSet::Damage)
		// This ensures PostGameplayEffectExecute is called on HarmoniaAttributeSet, not LyraHealthSet
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UHarmoniaAttributeSet::GetIncomingDamageAttribute(), 
				EGameplayModOp::Additive, 
				FinalDamage
			)
		);
	}
#else
	UE_LOG(LogHarmoniaCombat, Warning, TEXT("[DamageExecution] Skipped - not server code"));
#endif
}

