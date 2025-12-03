// Copyright 2025 Snow Game Studio.

#include "Inventory/HarmoniaInventoryFragment_Durability.h"
#include "Inventory/LyraInventoryItemInstance.h"

void UHarmoniaInventoryFragment_Durability::OnInstanceCreated(ULyraInventoryItemInstance* Instance) const
{
	// Initialize durability to max when item instance is created
	// The actual durability value would be stored in the instance's custom data
	// This is just a hook - actual implementation would need instance data storage
}
