// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventorySlotWidget.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UHarmoniaInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // ÃÊ±âÈ­
    SetSlotData(FItemID(), 0, 0.f);
}

void UHarmoniaInventorySlotWidget::SetSlotData(const FItemID& InItemId, int32 InCount, float InDurability)
{
    ItemId = InItemId;
    Count = InCount;

    if (CountText)
    {
        CountText->SetText(FText::AsNumber(Count));
    }

    if (ItemIcon)
    {
        const FItemData* Item = FINDITEMROW(InItemId.Id);

        if (Item)
        {
            ItemIcon->SetBrushResourceObject(Item->Icon);
        }
    }

    if (DurabilityBar)
    {
        DurabilityBar->SetPercent(InDurability);
    }
}