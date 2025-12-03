// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventorySlotIconWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UHarmoniaInventorySlotIconWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UHarmoniaInventorySlotIconWidget::SetSlotIcon(UTexture2D* IconTexture)
{
    if (ItemIcon)
    {
        ItemIcon->SetBrushResourceObject(IconTexture);
    }
    else
    {
        ItemIcon->SetBrushResourceObject(nullptr);
    }
}
