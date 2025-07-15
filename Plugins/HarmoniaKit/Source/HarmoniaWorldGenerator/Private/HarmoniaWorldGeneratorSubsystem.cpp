// Copyright 2025 Snow Game Studio.

#include "HarmoniaWorldGeneratorSubsystem.h"

// 기본 Perlin 노이즈 기반 생성기 (간단한 내부 클래스 예시)
class FPerlinWorldGenerator : public IWorldGenerator
{
public:
    virtual void GenerateWorldObjects(
        const FWorldGeneratorConfig& Config,
        TArray<FWorldObjectData>& OutObjects,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap
    ) override
    {
        FRandomStream Random(Config.Seed);

        int32 CountX = Config.WorldSizeX / 100;
        int32 CountY = Config.WorldSizeY / 100;

        for (int32 X = 0; X < CountX; ++X)
        {
            for (int32 Y = 0; Y < CountY; ++Y)
            {
                float WorldX = X * 100.0f;
                float WorldY = Y * 100.0f;

                float NoiseVal = FMath::PerlinNoise2D(FVector2D(WorldX + Config.Seed * 10, WorldY + Config.Seed * 20));
                float Prob = FMath::Abs(NoiseVal);

                if (Prob < Config.ObjectDensity)
                {
                    EWorldObjectType ObjType = (Prob > 0.5f) ? EWorldObjectType::Tree : EWorldObjectType::Rock;

                    FWorldObjectData NewObj;
                    NewObj.ObjectType = ObjType;
                    NewObj.ActorClass = ActorClassMap.Contains(ObjType) ? ActorClassMap[ObjType] : nullptr;
                    NewObj.Location = FVector(WorldX, WorldY, 0.f);
                    NewObj.Rotation = FRotator(0.f, Random.FRandRange(0, 360), 0.f);
                    NewObj.Scale = FVector(1.0f);

                    OutObjects.Add(NewObj);
                }
            }
        }
    }
};

void UHarmoniaWorldGeneratorSubsystem::GenerateWorld(const FWorldGeneratorConfig& Config, TArray<FWorldObjectData>& OutObjects, TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap)
{
    if (!Generator.IsValid())
    {
        Generator = MakeShareable(new FPerlinWorldGenerator());
    }

    Generator->GenerateWorldObjects(Config, OutObjects, ActorClassMap);
}

void UHarmoniaWorldGeneratorSubsystem::SetGenerator(TSharedPtr<IWorldGenerator> InGenerator)
{
    Generator = InGenerator;
}

