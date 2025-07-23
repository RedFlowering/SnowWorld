// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaItemSystemDefinitions.generated.h"

// 카테고리 정의
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

// 아이템 기본 정보 데이터
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
};

// 아이템별 스탯
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