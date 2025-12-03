// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventoryWidget.h"
#include "Definitions/HarmoniaInventorySystemDefinitions.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Components/UniformGridPanel.h"

void UHarmoniaInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    Refresh();

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this, &UHarmoniaInventoryWidget::Refresh);
	}
}

void UHarmoniaInventoryWidget::Refresh()
{
	if (InventoryComponent && SlotPanel && SlotWidgetClass)
	{
		SlotPanel->ClearChildren();

		const auto& Slots = InventoryComponent->InventoryData.Slots;
		const int32 Columns = SlotMaxColumns;
		int32 Index = 0;

		for (const auto& SlotData : Slots)
		{
			if (Index != InventoryData.MaxSlotCount)
			{
				int32 Row = Index / Columns;
				int32 Col = Index % Columns;

				UHarmoniaInventorySlotWidget* SlotWidget = CreateWidget<UHarmoniaInventorySlotWidget>(GetWorld(), SlotWidgetClass);
				if (SlotWidget)
				{
					SlotWidget->SetSlotData(FInventorySlot(Index, SlotData.ItemID, SlotData.Count, SlotData.Durability), InventoryComponent, this);
					SlotPanel->AddChildToUniformGrid(SlotWidget, Row, Col);
					Index++;
				}
			}
		}
	}
}

void UHarmoniaInventoryWidget::SetInventoryComponent(UHarmoniaInventoryComponent* InComponent)
{
    InventoryComponent = InComponent;
    Refresh();
}