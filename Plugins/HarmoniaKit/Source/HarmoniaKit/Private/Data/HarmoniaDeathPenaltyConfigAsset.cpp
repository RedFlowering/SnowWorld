// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/HarmoniaDeathPenaltyConfigAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UHarmoniaDeathPenaltyConfigAsset::UHarmoniaDeathPenaltyConfigAsset()
	: bEnableDeathPenalty(true)
	, bShowMemoryEchoMarker(true)
	, bShowDistanceToMemoryEcho(true)
	, MemoryEchoMarkerColor(FLinearColor(0.5f, 0.8f, 1.0f, 1.0f))
	, DifficultyMultiplier(1.0f)
{
	// Setup default currency drop configs
	Config.CurrencyDropConfigs.SetNum(4);

	// Memory Essence - full drop, full loss
	Config.CurrencyDropConfigs[0].CurrencyType = EHarmoniaCurrencyType::MemoryEssence;
	Config.CurrencyDropConfigs[0].DropPercentage = 1.0f;
	Config.CurrencyDropConfigs[0].PermanentLossPercentage = 1.0f;

	// Soul Crystals - 75% drop, 50% permanent loss
	Config.CurrencyDropConfigs[1].CurrencyType = EHarmoniaCurrencyType::SoulCrystals;
	Config.CurrencyDropConfigs[1].DropPercentage = 0.75f;
	Config.CurrencyDropConfigs[1].PermanentLossPercentage = 0.5f;

	// Forgotten Knowledge - 50% drop, 25% permanent loss (more forgiving)
	Config.CurrencyDropConfigs[2].CurrencyType = EHarmoniaCurrencyType::ForgottenKnowledge;
	Config.CurrencyDropConfigs[2].DropPercentage = 0.5f;
	Config.CurrencyDropConfigs[2].PermanentLossPercentage = 0.25f;

	// Time Fragments - 100% drop, but 0% permanent loss (always recoverable on second try)
	Config.CurrencyDropConfigs[3].CurrencyType = EHarmoniaCurrencyType::TimeFragments;
	Config.CurrencyDropConfigs[3].DropPercentage = 1.0f;
	Config.CurrencyDropConfigs[3].PermanentLossPercentage = 0.0f;
}

float UHarmoniaDeathPenaltyConfigAsset::GetCurrencyDropPercentage(EHarmoniaCurrencyType CurrencyType) const
{
	for (const FHarmoniaCurrencyDropConfig& DropConfig : Config.CurrencyDropConfigs)
	{
		if (DropConfig.CurrencyType == CurrencyType)
		{
			return FMath::Clamp(DropConfig.DropPercentage * DifficultyMultiplier, 0.0f, 1.0f);
		}
	}
	return 0.0f; // Not configured = don't drop
}

float UHarmoniaDeathPenaltyConfigAsset::GetCurrencyPermanentLossPercentage(EHarmoniaCurrencyType CurrencyType) const
{
	for (const FHarmoniaCurrencyDropConfig& DropConfig : Config.CurrencyDropConfigs)
	{
		if (DropConfig.CurrencyType == CurrencyType)
		{
			return FMath::Clamp(DropConfig.PermanentLossPercentage * DifficultyMultiplier, 0.0f, 1.0f);
		}
	}
	return 1.0f; // Not configured = lose all
}

bool UHarmoniaDeathPenaltyConfigAsset::HasCurrencyDropConfig(EHarmoniaCurrencyType CurrencyType) const
{
	for (const FHarmoniaCurrencyDropConfig& DropConfig : Config.CurrencyDropConfigs)
	{
		if (DropConfig.CurrencyType == CurrencyType)
		{
			return true;
		}
	}
	return false;
}

#if WITH_EDITOR
EDataValidationResult UHarmoniaDeathPenaltyConfigAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	// Check for duplicate currency types
	TSet<EHarmoniaCurrencyType> SeenTypes;
	for (const FHarmoniaCurrencyDropConfig& DropConfig : Config.CurrencyDropConfigs)
	{
		if (DropConfig.CurrencyType != EHarmoniaCurrencyType::None)
		{
			if (SeenTypes.Contains(DropConfig.CurrencyType))
			{
				Context.AddError(FText::Format(
					FText::FromString(TEXT("Duplicate currency drop config for type: {0}")),
					FText::AsNumber(static_cast<int32>(DropConfig.CurrencyType))
				));
				Result = EDataValidationResult::Invalid;
			}
			SeenTypes.Add(DropConfig.CurrencyType);
		}
	}

	// Validate percentage ranges
	if (DifficultyMultiplier < 0.0f || DifficultyMultiplier > 5.0f)
	{
		Context.AddError(FText::FromString(TEXT("Difficulty multiplier must be between 0.0 and 5.0")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

void UHarmoniaDeathPenaltyConfigAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Clamp difficulty multiplier
	DifficultyMultiplier = FMath::Clamp(DifficultyMultiplier, 0.0f, 5.0f);
}
#endif
