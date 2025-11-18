// Copyright 2025 Snow Game Studio.

#pragma once

#include "WorldGeneratorTypes.h"

class UHarmoniaObjectManager;
class UHarmoniaFarmingManager;
class UHarmoniaBuildingManager;
// 필요에 따라 추가

/**
 * 월드 생성기 → 하모니아 매니저 오브젝트 등록 보조 함수 집합
 */
class HarmoniaWorldIntegrationHelper
{
public:
    /**
     * 월드 생성 결과를 하모니아 각 매니저에 등록
     * @param WorldObjects   - 생성된 오브젝트 리스트
     * @param ObjectManager  - 오브젝트 전체 관리 매니저
     * @param FarmingManager - 파밍 전용 매니저(선택)
     * @param BuildingManager- 건축 전용 매니저(선택)
     */
	static void RegisterWorldObjectsToManagers(const TArray<FWorldObjectData>& WorldObjects/*, UHarmoniaObjectManager* ObjectManager,
												UHarmoniaFarmingManager* FarmingManager = nullptr, UHarmoniaBuildingManager* BuildingManager = nullptr*/);
};