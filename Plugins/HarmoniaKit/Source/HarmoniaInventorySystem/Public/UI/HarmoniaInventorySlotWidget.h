// Copyright 2025 Snow Game Studio.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "HarmoniaInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;

UCLASS()
class HARMONIAINVENTORYSYSTEM_API UHarmoniaInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable)
    void SetSlotData(const FItemID& InItemId, int32 InCount, float InDurability);

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CountText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> DurabilityBar = nullptr;

    // ΩΩ∑‘ µ•¿Ã≈Õ
    FItemID ItemId = FItemID();
    int32 Count = 0;
};