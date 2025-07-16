// Copyright 2025 Snow Game Studio.

#pragma once

#include "WorldGeneratorTypes.h"

/**
 * 오브젝트 배치 관련 유틸리티 함수 모음
 */
class ObjectPlacementHelper
{
public:
    // 지정 위치가 배치 가능한지 판정 (예시)
    static bool IsPlacementValid(const FVector& Location, const TArray<FWorldObjectData>& ExistingObjects, float MinDistance = 200.0f);

    // 가장 가까운 기존 오브젝트까지의 거리 반환 (예시)
    static float GetMinDistanceToObjects(const FVector& Location, const TArray<FWorldObjectData>& ExistingObjects);
};