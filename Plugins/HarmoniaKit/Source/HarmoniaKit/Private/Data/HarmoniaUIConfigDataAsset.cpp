// Copyright 2025 Snow Game Studio.

#include "Data/HarmoniaUIConfigDataAsset.h"

UHarmoniaUIConfigDataAsset::UHarmoniaUIConfigDataAsset()
{
	// 기본값�? 구조체에??초기?�됨
}

FLinearColor UHarmoniaUIConfigDataAsset::GetBorderColorForType(EHarmoniaStatusEffectType Type) const
{
	switch (Type)
	{
	case EHarmoniaStatusEffectType::Buff:
		return StatusEffectColors.BuffBorderColor;

	case EHarmoniaStatusEffectType::Debuff:
		return StatusEffectColors.DebuffBorderColor;

	case EHarmoniaStatusEffectType::Neutral:
	default:
		return StatusEffectColors.NeutralBorderColor;
	}
}

bool UHarmoniaUIConfigDataAsset::GetPredefinedEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const
{
	for (const FHarmoniaStatusEffectConfig& Config : PredefinedEffectConfigs)
	{
		if (Config.EffectTag.MatchesTagExact(EffectTag))
		{
			OutConfig = Config;
			return true;
		}
	}
	return false;
}
