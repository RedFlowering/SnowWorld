// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaWorldGenerator.cpp
 * @brief 기본 월드 생성 함수 구현
 * 
 * Earth-like 지형 생성을 위한 헬퍼 함수를 제공합니다.
 */

#include "WorldGeneratorTypes.h"
#include "PerlinNoiseHelper.h"

//=============================================================================
// World Generation Functions
//=============================================================================

/**
 * @brief Earth-like 월드 생성 함수
 * @param Config 월드 생성 설정
 * @param OutObjects 생성된 월드 오브젝트 배열 (출력)
 * @param OutHeightData Landscape용 하이트맵 데이터 (출력)
 * @param ActorClassMap 오브젝트 타입별 액터 클래스 매핑
 */
void GenerateEarthLikeWorld(
    const FWorldGeneratorConfig& Config,
    TArray<FWorldObjectData>& OutObjects,
    TArray<uint16>& OutHeightData,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap
)
{
    int32 SizeX = Config.SizeX;
    int32 SizeY = Config.SizeY;

    // Step 1: Generate terrain heightmap
    OutHeightData.SetNumUninitialized(SizeX * SizeY);

    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            float NormX = (float)X / (float)(SizeX - 1);
            float NormY = (float)Y / (float)(SizeY - 1);

            // Get height using multi-octave Perlin noise
            float HeightNorm = PerlinNoiseHelper::GetEarthLikeHeight(NormX * 10, NormY * 10, Config.Seed, Config.NoiseSettings); // -1~1
            float Height01 = (HeightNorm + 1.f) * 0.5f;
            float FinalHeight = FMath::Max(Height01, Config.SeaLevel);

            uint16 HeightVal = (uint16)FMath::Clamp(FinalHeight * 65535.f, 0.f, 65535.f);
            OutHeightData[Y * SizeX + X] = HeightVal;
        }
    }

    // Step 2: Place objects randomly (simple placement)
    FRandomStream Random(Config.Seed);
    for (int32 i = 0; i < SizeX * SizeY * Config.ObjectDensity; ++i)
    {
        int32 X = Random.RandRange(0, SizeX - 1);
        int32 Y = Random.RandRange(0, SizeY - 1);
        float HeightNorm = (float)OutHeightData[Y * SizeX + X] / 65535.f;

        // Only place above sea level
        if (HeightNorm > Config.SeaLevel + 0.02f)
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