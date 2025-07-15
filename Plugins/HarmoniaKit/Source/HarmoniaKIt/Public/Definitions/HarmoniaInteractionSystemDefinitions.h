// Copyright 2025 Snow Game Studio.

#pragma once

#include "GameFramework/Actor.h"
#include "HarmoniaInteractionSystemDefinitions.generated.h"

UENUM(BlueprintType)
enum class EHarmoniaInteractionType : uint8
{
    None UMETA(DisplayName="None"),
    Pickup UMETA(DisplayName="Pickup"),
    Gather UMETA(DisplayName="Gather"),
    Chop UMETA(DisplayName="Chop"),
    Open UMETA(DisplayName="Open"),
    Custom UMETA(DisplayName="Custom")
};

USTRUCT(BlueprintType)
struct FHarmoniaInteractionContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> Interactor = nullptr; // 플레이어, AI 등

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<AActor> Interactable = nullptr; // 나무, 아이템, 문 등

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHarmoniaInteractionType InteractionType = EHarmoniaInteractionType::None;
};

USTRUCT(BlueprintType)
struct FHarmoniaInteractionResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;
};