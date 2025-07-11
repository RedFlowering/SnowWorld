// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "HarmoniaLoadManager.generated.h"

UCLASS(BlueprintType)
class HARMONIALOADMANAGER_API UHarmoniaLoadManager : public UObject
{
	GENERATED_BODY()

public:
    // 싱글톤 접근자
    static UHarmoniaLoadManager* Get();

    // Blueprint에서 호출 가능하게
    UFUNCTION(BlueprintCallable, Category="Harmonia|DataTable")
    UDataTable* GetDataTableByKey(FName Key);

private:
    void LoadAllTables();

    TMap<FName, TSoftObjectPtr<UDataTable>> SoftTableMap;
    TMap<FName, TObjectPtr<UDataTable>> LoadedTableMap;

    static UHarmoniaLoadManager* SingletonInstance;
};