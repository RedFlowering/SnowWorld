// Copyright 2025 Snow Game Studio.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"
#include "InputCoreTypes.h"

#include "HarmoniaInventorySlotWidget.generated.h"

class UHarmoniaInventoryWidget;
class UHarmoniaInventorySlotIconWidget;
class UHarmoniaInventoryComponent;
class UTextBlock;
class UProgressBar;

UCLASS()
class HARMONIAKIT_API UHarmoniaInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable)
    void SetSlotData(const FInventorySlot InSlot, UHarmoniaInventoryComponent* InComponent, UHarmoniaInventoryWidget* InParent);

protected:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHarmoniaInventorySlotIconWidget> ItemIconWidget = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> CountText = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> DurabilityBar = nullptr;

protected:
    // ΩΩ∑‘ µ•¿Ã≈Õ
    FInventorySlot Slot = FInventorySlot();
    FInventorySlotEx SlotEx = FInventorySlotEx();

    TObjectPtr<UHarmoniaInventoryWidget> ParentWidget = nullptr;
    TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent = nullptr;
};