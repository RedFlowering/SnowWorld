// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaCoreDefinitions.generated.h"

// 아이템 고유 식별자
USTRUCT(BlueprintType, meta = (DisplayName = "HarmoniaID"))
struct HARMONIAKIT_API FHarmoniaID
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName Id = NAME_None;

    FHarmoniaID() : Id(NAME_None) {}
    FHarmoniaID(FName InId) : Id(InId) {}

    FORCEINLINE bool operator==(const FHarmoniaID& Other) const
    {
        return Id == Other.Id;
    }

    FORCEINLINE bool operator!=(const FHarmoniaID& Other) const
    {
        return Id != Other.Id;
    }

    friend inline uint32 GetTypeHash(const FHarmoniaID& Key)
    {
        return GetTypeHash(Key.Id);
    }
};