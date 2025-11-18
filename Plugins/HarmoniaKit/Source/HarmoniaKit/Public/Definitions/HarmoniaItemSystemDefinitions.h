// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaCraftingSystemDefinitions.h"
#include "HarmoniaItemSystemDefinitions.generated.h"

// ī�װ��� ����
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Resource,
    Weapon,
    Armor,
    Consumable,
    Quest,
    Etc
};

// ������ �⺻ ���� ������
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FHarmoniaID ItemId = FHarmoniaID();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGuid ItemGUID = FGuid();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName = FText();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemCategory Category = EItemCategory::Resource;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxStack = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Weight = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Durability = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftClassPtr<AActor> WorldActorClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description = FText();

    // Item grade/tier
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EItemGrade Grade = EItemGrade::Common;
};

// �����ۺ� ����
USTRUCT(BlueprintType)
struct FItemStatData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FHarmoniaID ItemId = FHarmoniaID();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Attack = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Defense = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Durability = 0.f;
};