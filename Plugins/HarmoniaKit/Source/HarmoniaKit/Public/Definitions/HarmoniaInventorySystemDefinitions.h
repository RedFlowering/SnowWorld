// Copyright 2025 Snow Game Studio.

#pragma once

#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaInventorySystemDefinitions.generated.h"

// �κ��丮 ����
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Index = 0;

    // ������ ���� �ĺ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FHarmoniaID ItemID = FHarmoniaID();

    // ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Durability = 0.0f;

    TSoftObjectPtr<UTexture2D> Icon = nullptr;

    FInventorySlot() : Index(0), ItemID(), Count(0), Durability(0.f) {}
    FInventorySlot(int32 InIndex, FHarmoniaID InId, int32 InCount, float InDurability) : Index(InIndex), ItemID(InId), Count(InCount), Durability(InDurability) {}
};

// �κ��丮 ��ü ������
USTRUCT(BlueprintType)
struct FInventoryData
{
    GENERATED_BODY()

    // ���� �迭
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FInventorySlot> Slots;

    // �κ��丮 �ִ� ���� ����
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
    FHarmoniaID ItemId = FHarmoniaID();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SlotIndex = 0;
};