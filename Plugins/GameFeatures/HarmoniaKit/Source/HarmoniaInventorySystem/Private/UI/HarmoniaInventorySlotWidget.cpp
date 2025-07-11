// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UHarmoniaInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // 초기화
    SetSlotData(FItemID(), 0);
}

void UHarmoniaInventorySlotWidget::SetSlotData(const FItemID& InItemId, int32 InCount)
{
    ItemId = InItemId;
    Count = InCount;

    if (CountText)
    {
        CountText->SetText(FText::AsNumber(Count));
    }

    // 아이콘 설정은 실제 아이템 데이터를 참조해 구현 가능
    if (ItemIcon)
    {

    }
}