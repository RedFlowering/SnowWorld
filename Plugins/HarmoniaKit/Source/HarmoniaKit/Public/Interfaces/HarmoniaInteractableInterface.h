// Copyright 2025 Snow Game Studio.

#pragma once

#include "UObject/NoExportTypes.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"
#include "HarmoniaInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UHarmoniaInteractableInterface : public UInterface
{
    GENERATED_BODY()
};

class IHarmoniaInteractableInterface
{
    GENERATED_BODY()

public:
    // 인터랙션 요청시 실행
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    void OnInteract(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult);
};