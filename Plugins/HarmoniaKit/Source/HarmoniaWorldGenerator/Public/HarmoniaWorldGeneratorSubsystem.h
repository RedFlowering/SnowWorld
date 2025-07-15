// Copyright 2025 Snow Game Studio.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "WorldGeneratorInterface.h"
#include "HarmoniaWorldGeneratorSubsystem.generated.h"

UCLASS()
class HARMONIAWORLDGENERATOR_API UHarmoniaWorldGeneratorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 월드 생성 요청
    UFUNCTION(BlueprintCallable, Category = "WorldGenerator")
    void GenerateWorld(const FWorldGeneratorConfig& Config, TArray<FWorldObjectData>& OutObjects, TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap);

    // (옵션) 현재 등록된 생성기 타입 교체 가능
    void SetGenerator(TSharedPtr<IWorldGenerator> InGenerator);

private:
    // 현재 생성기 (기본 PerlinNoise 방식)
    TSharedPtr<IWorldGenerator> Generator = nullptr;
};