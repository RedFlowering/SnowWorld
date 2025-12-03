// Copyright 2025 Snow Game Studio.

#pragma once

#include "WorldGeneratorTypes.h"

class UHarmoniaObjectManager;
class UHarmoniaFarmingManager;
class UHarmoniaBuildingManager;
// �ʿ信 ���� �߰�

/**
 * ���� ������ �� �ϸ�Ͼ� �Ŵ��� ������Ʈ ��� ���� �Լ� ����
 */
class HarmoniaWorldIntegrationHelper
{
public:
    /**
     * ���� ���� ����� �ϸ�Ͼ� �� �Ŵ����� ���
     * @param WorldObjects   - ������ ������Ʈ ����Ʈ
     * @param ObjectManager  - ������Ʈ ��ü ���� �Ŵ���
     * @param FarmingManager - �Ĺ� ���� �Ŵ���(����)
     * @param BuildingManager- ���� ���� �Ŵ���(����)
     */
	static void RegisterWorldObjectsToManagers(const TArray<FWorldObjectData>& WorldObjects/*, UHarmoniaObjectManager* ObjectManager,
												UHarmoniaFarmingManager* FarmingManager = nullptr, UHarmoniaBuildingManager* BuildingManager = nullptr*/);
};