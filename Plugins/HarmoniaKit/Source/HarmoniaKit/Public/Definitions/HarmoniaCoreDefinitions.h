// Copyright 2025 Snow Game Studio.

#pragma once

#include "HarmoniaCoreDefinitions.generated.h"

// Harmonia universal identifier
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

    FORCEINLINE bool IsValid() const
    {
        return Id != NAME_None;
    }

    FORCEINLINE FString ToString() const
    {
        return Id.ToString();
    }

    FORCEINLINE FName GetID() const
    {
        return Id;
    }

    FORCEINLINE bool IsNone() const
    {
        return Id == NAME_None;
    }

    friend inline uint32 GetTypeHash(const FHarmoniaID& Key)
    {
        return GetTypeHash(Key.Id);
    }
};