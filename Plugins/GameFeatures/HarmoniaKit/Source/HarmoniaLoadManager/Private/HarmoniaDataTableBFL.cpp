// Copyright 2025 Snow Game Studio.

#include "HarmoniaDataTableBFL.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

UDataTable* UHarmoniaDataTableBFL::GetCosmeticDataTable()
{
    return HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("Cosmetic"));
}

FGameplayTag UHarmoniaDataTableBFL::FindCosmeticRow(FName RowName)
{
    if (UDataTable* Table = GetCosmeticDataTable())
    {
        return *Table->FindRow<FGameplayTag>(RowName, TEXT("FindCosmeticRow"));
    }
    return FGameplayTag();
}

void UHarmoniaDataTableBFL::GetAllCosmeticRows(TArray<FGameplayTag>& OutRows)
{
    OutRows.Empty();
    if (UDataTable* Table = GetCosmeticDataTable())
    {
        for (const auto& Elem : Table->GetRowMap())
        {
            if (FGameplayTag* Row = reinterpret_cast<FGameplayTag*>(Elem.Value))
            {
                OutRows.Add(*Row);
            }
        }
    }
}

