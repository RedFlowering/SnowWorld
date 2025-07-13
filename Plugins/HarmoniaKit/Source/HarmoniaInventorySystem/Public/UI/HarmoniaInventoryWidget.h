// Copyright 2025 Snow Game Studio.

#pragma once

#include "Blueprint/UserWidget.h"
#include "HarmoniaInventorySlotWidget.h"
#include "HarmoniaInventoryWidget.generated.h"

class UHarmoniaInventoryComponent;
class UUniformGridPanel;

UCLASS()
class HARMONIAINVENTORYSYSTEM_API UHarmoniaInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable)
    void Refresh();

    UFUNCTION(BlueprintCallable)
    void SetInventoryComponent(UHarmoniaInventoryComponent* InComponent);

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> SlotPanel = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    TSubclassOf<UHarmoniaInventorySlotWidget> SlotWidgetClass = nullptr;

    UPROPERTY()
    TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    FInventoryData InventoryData = FInventoryData();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    int32 SlotMaxColumns = 10;
};