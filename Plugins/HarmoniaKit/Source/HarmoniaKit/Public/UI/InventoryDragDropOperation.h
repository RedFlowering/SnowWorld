// Copyright 2025 Snow Game Studio.

#pragma once
#include "Blueprint/DragDropOperation.h"

#include "InventoryDragDropOperation.generated.h"

UCLASS()
class HARMONIAKIT_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int32 SourceSlotIndex = -1;
};
