// Copyright 2025 Snow Game Studio.

#pragma once

#include "WorldGeneratorTypes.h"

// ���� ���� �������̽�. �پ��� ���� ������ ������ ���
class IWorldGenerator
{
public:
    virtual ~IWorldGenerator() = default;

    // �ֿ� ���� �Լ�
    virtual void GenerateWorldObjects(const FWorldGeneratorConfig& Config, TArray<FWorldObjectData>& OutObjects, TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap) = 0;
};