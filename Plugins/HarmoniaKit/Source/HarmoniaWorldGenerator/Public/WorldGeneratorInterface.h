// Copyright 2025 Snow Game Studio.

#pragma once

#include "WorldGeneratorTypes.h"

// 순수 가상 인터페이스. 다양한 월드 생성기 구현에 사용
class IWorldGenerator
{
public:
    virtual ~IWorldGenerator() = default;

    // 주요 생성 함수
    virtual void GenerateWorldObjects(const FWorldGeneratorConfig& Config, TArray<FWorldObjectData>& OutObjects, TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap) = 0;
};