// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaDataTableUtils.generated.h"

/**
 * Utility functions for data table access
 * Provides consistent error handling and logging for data table operations
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDataTableUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get row from data table with proper error handling
	 * @param DataTable - The data table to query
	 * @param RowName - The row name to find
	 * @param OutRow - The row data (if found)
	 * @param bLogError - Whether to log an error if not found
	 * @return True if row was found
	 */
	template<typename TRowType>
	static bool GetDataTableRow(UDataTable* DataTable, FName RowName, TRowType& OutRow, bool bLogError = true)
	{
		if (!DataTable)
		{
			if (bLogError)
			{
				UE_LOG(LogHarmoniaKit, Error, TEXT("GetDataTableRow: Null DataTable provided for row %s"), *RowName.ToString());
			}
			return false;
		}

		if (RowName.IsNone())
		{
			if (bLogError)
			{
				UE_LOG(LogHarmoniaKit, Error, TEXT("GetDataTableRow: Invalid RowName provided"));
			}
			return false;
		}

		TRowType* Row = DataTable->FindRow<TRowType>(RowName, TEXT("GetDataTableRow"));
		if (!Row)
		{
			if (bLogError)
			{
				UE_LOG(LogHarmoniaKit, Warning, TEXT("GetDataTableRow: Row '%s' not found in table '%s'"),
					*RowName.ToString(), *DataTable->GetName());
			}
			return false;
		}

		OutRow = *Row;
		return true;
	}

	/**
	 * Get row from data table using HarmoniaID
	 * @param DataTable - The data table to query
	 * @param ID - The Harmonia ID
	 * @param OutRow - The row data (if found)
	 * @param bLogError - Whether to log an error if not found
	 * @return True if row was found
	 */
	template<typename TRowType>
	static bool GetDataTableRowByID(UDataTable* DataTable, const FHarmoniaID& ID, TRowType& OutRow, bool bLogError = true)
	{
		if (!ID.IsValid())
		{
			if (bLogError)
			{
				UE_LOG(LogHarmoniaKit, Error, TEXT("GetDataTableRowByID: Invalid HarmoniaID provided"));
			}
			return false;
		}

		return GetDataTableRow<TRowType>(DataTable, ID.ToName(), OutRow, bLogError);
	}

	/**
	 * Get all rows from a data table
	 * @param DataTable - The data table to query
	 * @param OutRows - Array to fill with rows
	 * @return Number of rows retrieved
	 */
	template<typename TRowType>
	static int32 GetAllDataTableRows(UDataTable* DataTable, TArray<TRowType>& OutRows)
	{
		OutRows.Empty();

		if (!DataTable)
		{
			UE_LOG(LogHarmoniaKit, Error, TEXT("GetAllDataTableRows: Null DataTable provided"));
			return 0;
		}

		TArray<FName> RowNames = DataTable->GetRowNames();
		OutRows.Reserve(RowNames.Num());

		for (const FName& RowName : RowNames)
		{
			TRowType* Row = DataTable->FindRow<TRowType>(RowName, TEXT("GetAllDataTableRows"));
			if (Row)
			{
				OutRows.Add(*Row);
			}
		}

		return OutRows.Num();
	}

	/**
	 * Get filtered rows from a data table
	 * @param DataTable - The data table to query
	 * @param FilterPredicate - Predicate function to filter rows
	 * @param OutRows - Array to fill with filtered rows
	 * @return Number of rows retrieved
	 */
	template<typename TRowType>
	static int32 GetFilteredDataTableRows(UDataTable* DataTable, TFunction<bool(const TRowType&)> FilterPredicate, TArray<TRowType>& OutRows)
	{
		OutRows.Empty();

		if (!DataTable)
		{
			return 0;
		}

		TArray<FName> RowNames = DataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			TRowType* Row = DataTable->FindRow<TRowType>(RowName, TEXT("GetFilteredDataTableRows"));
			if (Row && FilterPredicate(*Row))
			{
				OutRows.Add(*Row);
			}
		}

		return OutRows.Num();
	}

	/**
	 * Check if a row exists in a data table
	 * @param DataTable - The data table to query
	 * @param RowName - The row name to check
	 * @return True if the row exists
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Utils")
	static bool DoesRowExist(UDataTable* DataTable, FName RowName);

	/**
	 * Get all row names from a data table
	 * @param DataTable - The data table to query
	 * @return Array of row names
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Utils")
	static TArray<FName> GetAllRowNames(UDataTable* DataTable);

	/**
	 * Get row count from a data table
	 * @param DataTable - The data table to query
	 * @return Number of rows in the table
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Utils")
	static int32 GetRowCount(UDataTable* DataTable);

	/**
	 * Get random row name from a data table
	 * @param DataTable - The data table to query
	 * @return Random row name or NAME_None if table is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Utils")
	static FName GetRandomRowName(UDataTable* DataTable);
};
