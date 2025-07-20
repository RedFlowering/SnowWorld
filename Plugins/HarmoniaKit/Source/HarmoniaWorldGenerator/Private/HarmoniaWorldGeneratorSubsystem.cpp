// Copyright 2025 Snow Game Studio.

#include "HarmoniaWorldGeneratorSubsystem.h"
#include "WorldGeneratorTypes.h"
#include "PerlinNoiseHelper.h"
#include "Landscape.h"
#include "LandscapeProxy.h"

/**
 * 코어 월드 생성 (지형 + 오브젝트)
 */
void UHarmoniaWorldGeneratorSubsystem::GenerateWorld(const FWorldGeneratorConfig& Config, TArray<int32>& OutHeightData, TArray<FWorldObjectData>& OutObjects, TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap)
{
    int32 SizeX = Config.SizeX;
    int32 SizeY = Config.SizeY;
    OutHeightData.SetNumUninitialized(SizeX * SizeY);

    // Heightmap 생성
    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            float NormX = (float)X / (float)(SizeX - 1);
            float NormY = (float)Y / (float)(SizeY - 1);

            float HeightNorm = PerlinNoiseHelper::GetEarthLikeHeight(NormX * 10, NormY * 10, Config.Seed);
            float Height01 = (HeightNorm + 1.f) * 0.5f;
            float FinalHeight = FMath::Max(Height01, Config.SeaLevel);

            int32 HeightVal = (int32)FMath::Clamp(FinalHeight * 65535.f, 0.f, 65535.f);
            OutHeightData[Y * SizeX + X] = HeightVal;

            if (!FMath::IsFinite(HeightNorm) || !FMath::IsFinite(FinalHeight))
            {
                UE_LOG(LogTemp, Error, TEXT("NaN/INF 발생! X:%d Y:%d HeightNorm:%f FinalHeight:%f"), X, Y, HeightNorm, FinalHeight);
                checkNoEntry();
            }
        }
    }

    // 오브젝트 자동 배치
    FRandomStream Random(Config.Seed);
    OutObjects.Empty();
    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            float HeightNorm = (float)OutHeightData[Y * SizeX + X] / 65535.f;
            if (HeightNorm > Config.SeaLevel + 0.02f)
            {
                if (Random.FRand() < Config.ObjectDensity)
                {
                    EWorldObjectType ObjType = PickObjectType(Config.ObjectTypeProbabilities, Random);
                    FVector Location(X * 100.f, Y * 100.f, HeightNorm * Config.MaxHeight);

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
    }
}

EWorldObjectType UHarmoniaWorldGeneratorSubsystem::PickObjectType(const TMap<EWorldObjectType, float>& ProbMap, FRandomStream& Random)
{
    float RandValue = Random.FRand();
    float Cumulative = 0.0f;

    for (const auto& Elem : ProbMap)
    {
        Cumulative += Elem.Value;
        if (RandValue < Cumulative)
        {
            return Elem.Key;
        }
    }

    // 혹시 합이 1이 안 되면 마지막 값 반환(실전에서는 예외 처리 추천)
    return ProbMap.Num() > 0 ? ProbMap.CreateConstIterator()->Key : EWorldObjectType::None;
}
