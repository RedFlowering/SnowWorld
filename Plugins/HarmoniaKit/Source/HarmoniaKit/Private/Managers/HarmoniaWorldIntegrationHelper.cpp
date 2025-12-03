// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaWorldIntegrationHelper.h"


void HarmoniaWorldIntegrationHelper::RegisterWorldObjectsToManagers(const TArray<FWorldObjectData>& WorldObjects)
{
    //if (!ObjectManager)
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("ObjectManager is null!"));
    //    return;
    //}

    for (const FWorldObjectData& ObjData : WorldObjects)
    {
        //// 1. ��ü ������Ʈ �Ŵ����� ���
        //ObjectManager->RegisterWorldObject(ObjData);

        //// 2. Ÿ�Ժ� �Ŵ������� ���
        //switch (ObjData.ObjectType)
        //{
        //case EWorldObjectType::Resource:
        //    if (FarmingManager)
        //    {
        //        FarmingManager->RegisterFarmingObject(ObjData);
        //    }
        //    break;

        //case EWorldObjectType::Structure:
        //    if (BuildingManager)
        //    {
        //        BuildingManager->RegisterBuildingObject(ObjData);
        //    }
        //    break;

        //    // �ʿ� �� �߰� ���̽�
        //default:
        //    break;
        //}
    }
}
