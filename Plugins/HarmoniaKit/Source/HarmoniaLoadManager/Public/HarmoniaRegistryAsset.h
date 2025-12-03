// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "HarmoniaRegistryAsset.generated.h"

USTRUCT()
struct FHarmoniaDataTableEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category = "DataTable")
	FString FunctionName;

	UPROPERTY(EditAnywhere, config, Category = "DataTable")
	TSoftObjectPtr<UDataTable> Table;
};

UCLASS(BlueprintType)
class HARMONIALOADMANAGER_API UHarmoniaRegistryAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Harmonia")
	TArray<FHarmoniaDataTableEntry> Entries;
};
