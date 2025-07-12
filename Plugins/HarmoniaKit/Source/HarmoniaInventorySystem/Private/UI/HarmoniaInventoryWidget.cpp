// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventoryWidget.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Components/UniformGridPanel.h"

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
    const int32 Columns = SlotMaxColumns;
    int32 Index = 0;

    for (const auto& SlotData : Slots)
    {
        int32 Row = Index / Columns;
        int32 Col = Index % Columns;

        UHarmoniaInventorySlotWidget* SlotWidget = CreateWidget<UHarmoniaInventorySlotWidget>(GetWorld(), SlotWidgetClass);
        if (SlotWidget)
        {
            SlotWidget->SetSlotData(SlotData.ItemID, SlotData.Count, SlotData.Durability);

            UUniformGridSlot* GridSlot = SlotPanel->AddChildToUniformGrid(SlotWidget, Row, Col);

            Index++;
        }
    }
}

void UHarmoniaInventoryWidget::SetInventorySource(UHarmoniaInventoryComponent* InComponent)
{
    InventorySource = InComponent;
    Refresh();
}