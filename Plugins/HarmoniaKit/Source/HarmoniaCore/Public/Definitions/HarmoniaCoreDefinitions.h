// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "HarmoniaCoreDefinitions.generated.h"

// 아이템 고유 식별자
USTRUCT(BlueprintType, meta = (DisplayName = "ItemID"))
struct HARMONIACORE_API FItemID
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Id = NAME_None;

    FItemID() : Id(NAME_None) {}
    FItemID(FName InId) : Id(InId) {}

    FORCEINLINE bool operator==(const FItemID& Other) const
    {
        return Id == Other.Id;
    }

    FORCEINLINE bool operator!=(const FItemID& Other) const
    {
        return Id != Other.Id;
    }

    friend inline uint32 GetTypeHash(const FItemID& Key)
    {
        return GetTypeHash(Key.Id);
    }
};

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
    FItemID ItemId = FItemID();

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
    float Durabirity = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description = FText();
};

// 아이템별 스탯
USTRUCT(BlueprintType)
struct FItemStatData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FItemID ItemId = FItemID();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Attack = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Defense = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Durability = 0.f;
};