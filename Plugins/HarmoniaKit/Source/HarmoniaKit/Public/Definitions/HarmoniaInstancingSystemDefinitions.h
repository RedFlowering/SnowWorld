// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaInstancingSystemDefinitions.generated.h"

// �ν��Ͻ��� ������Ʈ ����
UENUM(BlueprintType)
enum class EHarmoniaInstancedObjectType : uint8
{
    Item,
    Foliage,
    BuildingPart,
    Decoration,
};

// ���� �ν��Ͻ� ������
USTRUCT(BlueprintType)
struct FHarmoniaInstancedObjectData
{
    GENERATED_BODY()

    FHarmoniaInstancedObjectData() {}

    // ���� �� �ν��Ͻ� ���� GUID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid InstanceGuid;

    // �ν��Ͻ� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHarmoniaInstancedObjectType ObjectType = EHarmoniaInstancedObjectType::Item;

    // ������ �ĺ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DataId;

    // ����/ä�� ���� Ƚ�� ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;

    // ���� Ʈ������
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform WorldTransform;

    // ��Ÿ Ȯ�� ������ 
    // �ʿ�� JSON ��, �߰� ����ü ��� ������ Ȯ�� ����
};