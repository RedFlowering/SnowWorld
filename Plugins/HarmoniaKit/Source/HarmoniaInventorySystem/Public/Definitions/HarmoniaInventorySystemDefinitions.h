// Copyright 2025 Snow Game Studio.

#pragma once

#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaInventorySystemDefinitions.generated.h"

// 인벤토리 슬롯
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    // 아이템 고유 식별자
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemID ItemID = FItemID();

    // 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Durability = 0.0f;

    FInventorySlot() : ItemID(), Count(0) {}
    FInventorySlot(FItemID InId, int32 InCount) : ItemID(InId), Count(InCount) {}
};

// 인벤토리 전체 데이터
USTRUCT(BlueprintType)
struct FInventoryData
{
    GENERATED_BODY()

    // 슬롯 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FInventorySlot> Slots;

    // 인벤토리 최대 슬롯 개수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSlotCount = 20;

    FInventoryData() : MaxSlotCount(20) { Slots.SetNum(MaxSlotCount); }
};

UENUM(BlueprintType)
enum class EInventorySlotType : uint8
{
    Normal,
    Quick,
    Equipment
};

USTRUCT(BlueprintType)
struct FInventorySlotEx : public FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInventorySlotType SlotType = EInventorySlotType::Normal;
};

UENUM(BlueprintType)
enum class EInventoryChangeType : uint8
{
    Add,
    Remove,
    Move,
    Destroy
};

USTRUCT(BlueprintType)
struct FInventoryChangeLog
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInventoryChangeType ChangeType = EInventoryChangeType::Add;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemID ItemId = FItemID();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SlotIndex = 0;
};