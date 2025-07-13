// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaInventorySlotWidget.h"
#include "UI/HarmoniaInventoryWidget.h"
#include "UI/HarmoniaInventorySlotIconWidget.h"
#include "UI/InventoryDragDropOperation.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Definitions/HarmoniaMacroDefinitions.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UHarmoniaInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    // 초기화
    SetSlotData(FInventorySlot(), nullptr, nullptr);
}

void UHarmoniaInventorySlotWidget::SetSlotData(const FInventorySlot InSlot, UHarmoniaInventoryComponent* InComponent, UHarmoniaInventoryWidget* InParent)
{


    Slot.Index = InSlot.Index;
    Slot.ItemID = InSlot.ItemID;
    Slot.Count = InSlot.Count;
    Slot.Durability = InSlot.Durability;

    InventoryComponent = InComponent;
    ParentWidget = InParent;

    if (Slot.Index < 10)
    {
        SlotEx.SlotType = EInventorySlotType::Quick;
    }
    else
    {
        SlotEx.SlotType = EInventorySlotType::Normal;
    }

    if (ItemIconWidget)
    {
        ItemIconWidget->SetSlotIcon(Slot.ItemID);
    }

    if (CountText)
    {
        CountText->SetText(FText::AsNumber(Slot.Count));
    }

    if (DurabilityBar)
    {
        DurabilityBar->SetPercent(Slot.Durability);
    }

    if (Slot.ItemID.Id != NAME_None && Slot.Count != 0)
    {
        ItemIconWidget->SetVisibility(ESlateVisibility::Visible);
        CountText->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        ItemIconWidget->SetVisibility(ESlateVisibility::Collapsed);
        CountText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

FReply UHarmoniaInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UHarmoniaInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    // Drag & Drop 오퍼레이션 생성
    UInventoryDragDropOperation* DragOp = NewObject<UInventoryDragDropOperation>();
    DragOp->SourceSlotIndex = Slot.Index;

    DragOp->DefaultDragVisual = ItemIconWidget;

    OutOperation = DragOp;
}

bool UHarmoniaInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);

    if (DragOp && InventoryComponent && ParentWidget)
    {
        int32 From = DragOp->SourceSlotIndex;
        int32 To = Slot.Index;

        if (From != To)
        {
            InventoryComponent->SwapSlots(From, To);
            ParentWidget->Refresh();
        }

        return true;
    }

    return false;
}
