// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventoryWidget.h"
#include "Components/PanelWidget.h"
#include "Components/HarmoniaInventoryComponent.h"

void UHarmoniaInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    Refresh();
}

void UHarmoniaInventoryWidget::Refresh()
{
    if (!InventorySource || !SlotPanel || !SlotWidgetClass)
        return;

    SlotPanel->ClearChildren();

    const auto& Slots = InventorySource->Slots;
    for (const auto& SlotData : Slots)
    {
        UHarmoniaInventorySlotWidget* SlotWidget = CreateWidget<UHarmoniaInventorySlotWidget>(GetWorld(), SlotWidgetClass);
        if (SlotWidget)
        {
            SlotWidget->SetSlotData(SlotData.ItemID, SlotData.Count);
            SlotPanel->AddChild(SlotWidget);
        }
    }
}

void UHarmoniaInventoryWidget::SetInventorySource(UHarmoniaInventoryComponent* InComponent)
{
    InventorySource = InComponent;
    Refresh();
}