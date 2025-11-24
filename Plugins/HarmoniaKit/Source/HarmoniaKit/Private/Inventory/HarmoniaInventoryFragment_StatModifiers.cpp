// Copyright 2025 Snow Game Studio.

#include "Inventory/HarmoniaInventoryFragment_StatModifiers.h"
#include "Inventory/LyraInventoryItemInstance.h"

void UHarmoniaInventoryFragment_StatModifiers::OnInstanceCreated(ULyraInventoryItemInstance* Instance) const
{
	// Hook for when item instance is created
	// Stat modifiers would be applied when the item is equipped via equipment system
}
