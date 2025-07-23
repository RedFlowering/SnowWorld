// Copyright 2025 Snow Game Studio.

#include "WorldGeneratorTypes.h"
#include "PerlinNoiseHelper.h"

void GenerateEarthLikeWorld(
    const FWorldGeneratorConfig& Config,
    TArray<FWorldObjectData>& OutObjects,
    TArray<uint16>& OutHeightData, // Heightmap (Landscape 용)
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap
)
{
    int32 SizeX = Config.SizeX;
    int32 SizeY = Config.SizeY;

    // 1. 지형(높이맵) 생성
    OutHeightData.SetNumUninitialized(SizeX * SizeY);

    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            float NormX = (float)X / (float)(SizeX - 1);
            float NormY = (float)Y / (float)(SizeY - 1);

            float HeightNorm = PerlinNoiseHelper::GetEarthLikeHeight(NormX * 10, NormY * 10, Config.Seed, Config.NoiseSettings); // -1~1
            float Height01 = (HeightNorm + 1.f) * 0.5f;
            float FinalHeight = FMath::Max(Height01, Config.SeaLevel);

            uint16 HeightVal = (uint16)FMath::Clamp(FinalHeight * 65535.f, 0.f, 65535.f);
            OutHeightData[Y * SizeX + X] = HeightVal;
        }
    }

    // 2. 오브젝트 자동 배치 (육지에만)
    FRandomStream Random(Config.Seed);
    for (int32 i = 0; i < SizeX * SizeY * Config.ObjectDensity; ++i)
    {
        int32 X = Random.RandRange(0, SizeX - 1);
        int32 Y = Random.RandRange(0, SizeY - 1);
        float HeightNorm = (float)OutHeightData[Y * SizeX + X] / 65535.f;

        if (HeightNorm > Config.SeaLevel + 0.02f) // 바다 위만
        {
            FVector Location(X * 100.f, Y * 100.f, HeightNorm * Config.MaxHeight);
            EWorldObjectType ObjType = (Random.FRand() > 0.7f) ? EWorldObjectType::Tree : EWorldObjectType::Rock;

            FWorldObjectData ObjData;
            ObjData.ObjectType = ObjType;
            ObjData.ActorClass = ActorClassMap.Contains(ObjType) ? ActorClassMap[ObjType] : nullptr;
            ObjData.Location = Location;
            ObjData.Rotation = FRotator(0.f, Random.FRandRange(0, 360.f), 0.f);
            ObjData.Scale = FVector(1.f);

            OutObjects.Add(ObjData);
        }
    }
}