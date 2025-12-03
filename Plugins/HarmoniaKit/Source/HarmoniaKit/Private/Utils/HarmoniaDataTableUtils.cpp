// Copyright Epic Games, Inc. All Rights Reserved.

#include "Utils/HarmoniaDataTableUtils.h"
#include "HarmoniaLogCategories.h"

bool UHarmoniaDataTableUtils::DoesRowExist(UDataTable* DataTable, FName RowName)
{
	if (!DataTable || RowName.IsNone())
	{
		return false;
	}

	return DataTable->GetRowNames().Contains(RowName);
}

TArray<FName> UHarmoniaDataTableUtils::GetAllRowNames(UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("GetAllRowNames: Null DataTable provided"));
		return TArray<FName>();
	}

	return DataTable->GetRowNames();
}

int32 UHarmoniaDataTableUtils::GetRowCount(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return 0;
	}

	return DataTable->GetRowNames().Num();
}

FName UHarmoniaDataTableUtils::GetRandomRowName(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return NAME_None;
	}

	TArray<FName> RowNames = DataTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		return NAME_None;
	}

	int32 RandomIndex = FMath::RandRange(0, RowNames.Num() - 1);
	return RowNames[RandomIndex];
}
