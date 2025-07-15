// Copyright 2025 Snow Game Studio.

#pragma once

#include "WorldGeneratorTypes.generated.h"

// 오브젝트 종류 예시 (확장 가능)
UENUM(BlueprintType)
enum class EWorldObjectType : uint8
{
    None        UMETA(DisplayName = "None"),
    Tree        UMETA(DisplayName = "Tree"),
    Rock        UMETA(DisplayName = "Rock"),
    Resource    UMETA(DisplayName = "Resource"),
    Structure   UMETA(DisplayName = "Structure"),
};

// 월드에 생성되는 오브젝트 정보
USTRUCT(BlueprintType)
struct FWorldObjectData
{
    GENERATED_BODY()

    // 생성할 액터 클래스 SoftClass (후처리/스폰용)
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

    // 필요에 따라 확장 (커스텀 데이터, 인스턴스 ID 등)
};

// 월드 생성 파라미터 (맵 사이즈, 시드 등)
USTRUCT(BlueprintType)
struct FWorldGeneratorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Seed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WorldSizeX = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WorldSizeY = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ObjectDensity = 0.001f; // 0~1, 한 타일당 오브젝트 생성 확률
};