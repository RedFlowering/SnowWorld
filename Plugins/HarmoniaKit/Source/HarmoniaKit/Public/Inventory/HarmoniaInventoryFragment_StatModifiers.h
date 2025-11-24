// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "HarmoniaInventoryFragment_StatModifiers.generated.h"

/**
 * Inventory Fragment for items that modify character stats
 * Extends Lyra's inventory system with RPG-style stat modifications
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaInventoryFragment_StatModifiers : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	/** Stat modifiers applied when this item is equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TArray<FEquipmentStatModifier> StatModifiers;

	/** Whether these modifiers stack if multiple instances are equipped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	bool bStackable = false;

	virtual void OnInstanceCreated(ULyraInventoryItemInstance* Instance) const override;
};
