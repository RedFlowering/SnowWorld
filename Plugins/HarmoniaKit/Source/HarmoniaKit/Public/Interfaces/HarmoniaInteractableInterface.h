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
    // ���ͷ��� ��û�� ����
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
    void OnInteract(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult);
};