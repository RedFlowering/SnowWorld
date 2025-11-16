// Copyright RedFlowering. All Rights Reserved.

#include "AbilitySystem/HarmoniaGameplayTagsBFL.h"
#include "Engine/DataTable.h"

// Initialize static members
UDataTable* UHarmoniaGameplayTagsBFL::CachedGameplayTagsDataTable = nullptr;
UDataTable* UHarmoniaGameplayTagsBFL::CachedComboAttackDataTable = nullptr;

UDataTable* UHarmoniaGameplayTagsBFL::LoadDataTable(const FString& AssetPath, UDataTable*& CachedDataTable)
{
	if (CachedDataTable)
	{
		return CachedDataTable;
	}

	CachedDataTable = LoadObject<UDataTable>(nullptr, *AssetPath);

	if (!CachedDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load DataTable at path: %s"), *AssetPath);
	}

	return CachedDataTable;
}

UDataTable* UHarmoniaGameplayTagsBFL::GetGameplayTagsDataTable()
{
	// TODO: Update this path to point to your actual DataTable asset
	const FString DataTablePath = TEXT("/HarmoniaKit/DataTables/DT_HarmoniaGameplayTags.DT_HarmoniaGameplayTags");
	return LoadDataTable(DataTablePath, CachedGameplayTagsDataTable);
}

UDataTable* UHarmoniaGameplayTagsBFL::GetComboAttackDataTable()
{
	// TODO: Update this path to point to your actual DataTable asset
	const FString DataTablePath = TEXT("/HarmoniaKit/DataTables/DT_ComboAttackData.DT_ComboAttackData");
	return LoadDataTable(DataTablePath, CachedComboAttackDataTable);
}

bool UHarmoniaGameplayTagsBFL::GetGameplayTagData(FName TagId, FHarmoniaGameplayTagData& OutTagData)
{
	UDataTable* DataTable = GetGameplayTagsDataTable();
	if (!DataTable)
	{
		return false;
	}

	FHarmoniaGameplayTagData* FoundData = DataTable->FindRow<FHarmoniaGameplayTagData>(TagId, TEXT("GetGameplayTagData"));
	if (FoundData)
	{
		OutTagData = *FoundData;
		return true;
	}

	return false;
}

bool UHarmoniaGameplayTagsBFL::GetComboAttackData(int32 ComboIndex, FHarmoniaComboAttackData& OutComboData)
{
	UDataTable* DataTable = GetComboAttackDataTable();
	if (!DataTable)
	{
		return false;
	}

	// Find row by combo index
	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FHarmoniaComboAttackData* RowData = DataTable->FindRow<FHarmoniaComboAttackData>(RowName, TEXT("GetComboAttackData"));
		if (RowData && RowData->ComboIndex == ComboIndex)
		{
			OutComboData = *RowData;
			return true;
		}
	}

	return false;
}

void UHarmoniaGameplayTagsBFL::GetAllComboAttackData(TArray<FHarmoniaComboAttackData>& OutComboDataArray)
{
	OutComboDataArray.Empty();

	UDataTable* DataTable = GetComboAttackDataTable();
	if (!DataTable)
	{
		return;
	}

	TArray<FHarmoniaComboAttackData*> AllRows;
	DataTable->GetAllRows<FHarmoniaComboAttackData>(TEXT("GetAllComboAttackData"), AllRows);

	// Convert pointers to values and sort by combo index
	for (FHarmoniaComboAttackData* RowData : AllRows)
	{
		if (RowData)
		{
			OutComboDataArray.Add(*RowData);
		}
	}

	// Sort by combo index
	OutComboDataArray.Sort([](const FHarmoniaComboAttackData& A, const FHarmoniaComboAttackData& B)
	{
		return A.ComboIndex < B.ComboIndex;
	});
}

int32 UHarmoniaGameplayTagsBFL::GetMaxComboIndex()
{
	TArray<FHarmoniaComboAttackData> AllComboData;
	GetAllComboAttackData(AllComboData);

	if (AllComboData.Num() == 0)
	{
		return -1;
	}

	int32 MaxIndex = -1;
	for (const FHarmoniaComboAttackData& ComboData : AllComboData)
	{
		if (ComboData.ComboIndex > MaxIndex)
		{
			MaxIndex = ComboData.ComboIndex;
		}
	}

	return MaxIndex;
}
