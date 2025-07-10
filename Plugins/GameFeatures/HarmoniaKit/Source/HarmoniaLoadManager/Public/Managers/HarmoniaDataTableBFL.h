// Copyright 2025 Snow Game Studio.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaLoadManager.h"
#include "HarmoniaDataTableBFL.generated.h"

UCLASS()
class HARMONIALOADMANAGER_API UHarmoniaDataTableBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetCosmeticDataTable();

};
