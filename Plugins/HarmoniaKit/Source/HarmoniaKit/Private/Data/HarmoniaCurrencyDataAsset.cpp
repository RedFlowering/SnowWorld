// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/HarmoniaCurrencyDataAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UHarmoniaCurrencyDataAsset::UHarmoniaCurrencyDataAsset()
	: CurrencyType(EHarmoniaCurrencyType::None)
	, DisplayName(FText::FromString(TEXT("Unknown Currency")))
	, Description(FText::GetEmpty())
	, CurrencyColor(FLinearColor::White)
	, MaxCarryAmount(0)
	, bCanDropOnDeath(true)
	, DropPercentage(1.0f)
	, bIsTradeable(false)
{
}

FText UHarmoniaCurrencyDataAsset::GetDisplayTextWithAmount(int32 Amount) const
{
	return FText::Format(
		FText::FromString(TEXT("{0}: {1}")),
		DisplayName,
		FText::AsNumber(Amount)
	);
}

bool UHarmoniaCurrencyDataAsset::IsWithinCarryLimit(int32 Amount) const
{
	if (MaxCarryAmount <= 0)
	{
		return true; // Unlimited
	}
	return Amount <= MaxCarryAmount;
}

int32 UHarmoniaCurrencyDataAsset::ConvertTo(EHarmoniaCurrencyType TargetCurrency, int32 Amount) const
{
	if (const float* ConversionRate = ConversionRates.Find(TargetCurrency))
	{
		return FMath::FloorToInt32(Amount * (*ConversionRate));
	}
	return 0;
}

#if WITH_EDITOR
EDataValidationResult UHarmoniaCurrencyDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;

	if (CurrencyType == EHarmoniaCurrencyType::None)
	{
		Context.AddError(FText::FromString(TEXT("Currency type cannot be None")));
		Result = EDataValidationResult::Invalid;
	}

	if (DisplayName.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("Display name cannot be empty")));
		Result = EDataValidationResult::Invalid;
	}

	if (bCanDropOnDeath && (DropPercentage < 0.0f || DropPercentage > 1.0f))
	{
		Context.AddError(FText::FromString(TEXT("Drop percentage must be between 0.0 and 1.0")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
