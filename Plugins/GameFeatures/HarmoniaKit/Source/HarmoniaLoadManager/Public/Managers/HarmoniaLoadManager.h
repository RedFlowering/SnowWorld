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

	// 플러그인 Config에서 테이블 정보를 로드
	void LoadFromProjectSettings();

	// Blueprint에서도 접근 가능
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DataTable")
	UDataTable* GetDataTableByKey(FName Key);

	// 모든 테이블을 미리 로드
	UFUNCTION(BlueprintCallable, Category = "Harmonia|DataTable")
	void LoadAllTables();

private:
	TMap<FName, TSoftObjectPtr<UDataTable>> SoftTableMap;
	TMap<FName, TObjectPtr<UDataTable>> LoadedTableMap;

	static UHarmoniaLoadManager* SingletonInstance;
};