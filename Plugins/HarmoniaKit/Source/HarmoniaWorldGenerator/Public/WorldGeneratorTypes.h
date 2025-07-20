// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldGeneratorTypes.generated.h"

UENUM(BlueprintType)
enum class EWorldObjectType : uint8
{
    None        UMETA(DisplayName = "None"),
    Tree        UMETA(DisplayName = "Tree"),
    Rock        UMETA(DisplayName = "Rock"),
    Resource    UMETA(DisplayName = "Resource"),
    Structure   UMETA(DisplayName = "Structure"),
};

USTRUCT(BlueprintType)
struct FWorldObjectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<AActor> ActorClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWorldObjectType ObjectType = EWorldObjectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale = FVector::OneVector;
};

/**
 * 월드 생성 파라미터 구조체
 */
USTRUCT(BlueprintType)
struct FWorldGeneratorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    int32 Seed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    int32 SizeX = 512;  // 2^n 권장

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    int32 SizeY = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    float MaxHeight = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    float ObjectDensity = 0.008f; // 0~1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    float SeaLevel = 0.42f; // 0~1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
    TMap<EWorldObjectType, float> ObjectTypeProbabilities;
};

