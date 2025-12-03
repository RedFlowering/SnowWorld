// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLoadManager.h
 * @brief 데이터 테이블 관리자 클래스 정의
 * 
 * 레지스트리 에셋을 통해 데이터 테이블을 중앙 관리하고
 * 비동기 로드를 지원합니다.
 */

#pragma once

#include "Engine/DataTable.h"
#include "HarmoniaLoadManager.generated.h"

class UHarmoniaRegistryAsset;

/**
 * @class UHarmoniaLoadManager
 * @brief 데이터 테이블 중앙 관리자 (싱글톤)
 * 
 * 레지스트리 에셋에 등록된 모든 데이터 테이블을
 * 중앙에서 관리하고 비동기 로드를 지원합니다.
 * 
 * @code
 * UDataTable* ItemTable = UHarmoniaLoadManager::Get()->GetDataTableByKey("ItemData");
 * @endcode
 * 
 * @see UHarmoniaRegistryAsset
 */
UCLASS(BlueprintType)
class HARMONIALOADMANAGER_API UHarmoniaLoadManager : public UObject
{
	GENERATED_BODY()

public:
    /**
     * @brief 싱글톤 인스턴스 접근자
     * @return UHarmoniaLoadManager 싱글톤 인스턴스
     */
    static UHarmoniaLoadManager* Get();

    /**
     * @brief 키로 데이터 테이블 조회
     * @param Key 레지스트리에 등록된 테이블 키
     * @return 로드된 데이터 테이블 (없으면 nullptr)
     */
    UFUNCTION(BlueprintCallable, Category="Harmonia|DataTable")
    UDataTable* GetDataTableByKey(FName Key);

    /**
     * @brief 레지스트리 에셋으로부터 초기화
     * @param Registry 데이터 테이블 레지스트리 에셋
     */
    void InitializeFromRegistry(const UHarmoniaRegistryAsset* Registry);

    /** @brief 모든 등록된 테이블 비동기 로드 시작 */
    void LoadAllTables();

    /** @brief 초기화 완료 여부 확인 */
    bool IsInitialized() const { return bIsInitialized; }

private:
    /** @brief 프로젝트 내 레지스트리 에셋 자동 탐색 및 로드 */
    void AutoLoadRegistry();

    /** @brief Soft 참조 테이블 맵 (로드 전) */
    TMap<FName, TSoftObjectPtr<UDataTable>> SoftTableMap;
    
    /** @brief 로드된 테이블 맵 */
    TMap<FName, TObjectPtr<UDataTable>> LoadedTableMap;

    bool bIsInitialized = false;

    static UHarmoniaLoadManager* SingletonInstance;
};