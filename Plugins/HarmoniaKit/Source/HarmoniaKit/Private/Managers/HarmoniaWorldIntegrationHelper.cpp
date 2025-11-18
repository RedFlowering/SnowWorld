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
        //// 1. 전체 오브젝트 매니저에 등록
        //ObjectManager->RegisterWorldObject(ObjData);

        //// 2. 타입별 매니저에도 등록
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

        //    // 필요 시 추가 케이스
        //default:
        //    break;
        //}
    }
}
