// Copyright 2025 Snow Game Studio.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class HARMONIAINVENTORYSYSTEM_API UHarmoniaInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable)
    void SetSlotData(const FItemID& InItemId, int32 InCount);

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CountText = nullptr;

    // ΩΩ∑‘ µ•¿Ã≈Õ
    FItemID ItemId = FItemID();
    int32 Count = 0;
};