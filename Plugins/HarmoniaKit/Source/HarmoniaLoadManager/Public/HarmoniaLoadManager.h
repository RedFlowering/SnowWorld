// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "HarmoniaLoadManager.generated.h"

class UHarmoniaRegistryAsset;

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

    // Registry 에셋으로부터 테이블 맵 초기화
    void InitializeFromRegistry(const UHarmoniaRegistryAsset* Registry);

    // 모든 테이블 비동기 로드 시작
    void LoadAllTables();

    // 초기화 여부 확인
    bool IsInitialized() const { return bIsInitialized; }

private:
    // 프로젝트 내 Registry 에셋을 자동으로 찾아 로드
    void AutoLoadRegistry();

    TMap<FName, TSoftObjectPtr<UDataTable>> SoftTableMap;
    TMap<FName, TObjectPtr<UDataTable>> LoadedTableMap;

    bool bIsInitialized = false;

    static UHarmoniaLoadManager* SingletonInstance;
};