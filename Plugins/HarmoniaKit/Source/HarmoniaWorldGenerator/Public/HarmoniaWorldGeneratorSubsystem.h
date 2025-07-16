// Copyright 2025 Snow Game Studio.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "WorldGeneratorTypes.h"
#include "HarmoniaWorldGeneratorSubsystem.generated.h"

/**
 * 하모니아 월드 생성 서브시스템
 * - HeightData(지형)와 OutObjects(오브젝트)를 한 번에 생성
 * - 지구 스타일, 시드 기반 월드 생성
 */
UCLASS()
class HARMONIAWORLDGENERATOR_API UHarmoniaWorldGeneratorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /**
     * 지형(HeightData)와 오브젝트 자동 생성
     * @param Config         - 월드 생성 파라미터
     * @param OutHeightData  - 생성된 Heightmap 데이터 (Landscape 등에서 사용)
     * @param OutObjects     - 생성된 오브젝트 목록
     * @param ActorClassMap  - 오브젝트 타입별 클래스 매핑
     */
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator")
    void GenerateWorld(const FWorldGeneratorConfig& Config, TArray<int32>& OutHeightData, TArray<FWorldObjectData>& OutObjects, TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap);
};