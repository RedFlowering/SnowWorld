// Copyright 2025 Snow Game Studio.

#pragma once

#include "Blueprint/UserWidget.h"

#include "HarmoniaInventorySlotIconWidget.generated.h"

class UImage;

UCLASS()
class HARMONIAINVENTORYSYSTEM_API UHarmoniaInventorySlotIconWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    void SetSlotIcon(const FItemID& ItemID);

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon = nullptr;
};
