// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaRowIncludes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaLoadManager.h"

#include "HarmoniaDataTableBFL.generated.h"

// This file is auto-generated. Do not modify manually.
// Any manual changes will be overwritten by the code generator.

UCLASS()
class HARMONIALOADMANAGER_API UHarmoniaDataTableBFL : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetItemDataTable();

    UFUNCTION(BlueprintPure, Category = "Harmonia|LoadManager")
    static UDataTable* GetGamePlayTagsDataTable();

};
