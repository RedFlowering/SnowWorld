// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaInstancingSystemDefinitions.generated.h"

// 인스턴스드 오브젝트 유형
UENUM(BlueprintType)
enum class EHarmoniaInstancedObjectType : uint8
{
    Item,
    Foliage,
    BuildingPart,
    Decoration,
};

// 공용 인스턴스 데이터
USTRUCT(BlueprintType)
struct FHarmoniaInstancedObjectData
{
    GENERATED_BODY()

    FHarmoniaInstancedObjectData() {}

    // 월드 내 인스턴스 고유 GUID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid InstanceGuid;

    // 인스턴스 유형
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHarmoniaInstancedObjectType ObjectType = EHarmoniaInstancedObjectType::Item;

    // 데이터 식별자
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DataId;

    // 개수/채집 가능 횟수 등
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;

    // 월드 트랜스폼
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform WorldTransform;

    // 기타 확장 데이터 
    // 필요시 JSON 등, 추가 구조체 상속 등으로 확장 가능
};