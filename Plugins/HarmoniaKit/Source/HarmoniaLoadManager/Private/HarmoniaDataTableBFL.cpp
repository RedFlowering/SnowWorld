// Copyright 2025 Snow Game Studio.

#include "HarmoniaDataTableBFL.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

UDataTable* UHarmoniaDataTableBFL::GetItemDataTable()
{
    return HARMONIALOADMANAGER()->GetDataTableByKey(TEXT("Item"));
}

//FItemData UHarmoniaDataTableBFL::FindItemRow(FName RowName)
//{
//    if (UDataTable* Table = GetItemDataTable())
//    {
//        return *Table->FindRow<FItemData>(RowName, TEXT("FindItemRow"));
//    }
//    return FItemData();
//}
//
//void UHarmoniaDataTableBFL::GetAllItemRows(TArray<FItemData>& OutRows)
//{
//    OutRows.Empty();
//    if (UDataTable* Table = GetItemDataTable())
//    {
//        for (const auto& Elem : Table->GetRowMap())
//        {
//            if (FItemData* Row = reinterpret_cast<FItemData*>(Elem.Value))
//            {
//                OutRows.Add(*Row);
//            }
//        }
//    }
//}

