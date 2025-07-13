// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventorySlotIconWidget.h"
#include "Definitions/HarmoniaCoreDefinitions.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Components/Image.h"

void UHarmoniaInventorySlotIconWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UHarmoniaInventorySlotIconWidget::SetSlotIcon(const FItemID& ItemID)
{
    if (ItemIcon)
    {
        const FItemData* Item = FINDITEMROW(ItemID.Id);

        if (Item)
        {
            ItemIcon->SetBrushResourceObject(Item->Icon);
        }
        else
        {
            ItemIcon->SetBrushResourceObject(nullptr);
        }
    }
}
