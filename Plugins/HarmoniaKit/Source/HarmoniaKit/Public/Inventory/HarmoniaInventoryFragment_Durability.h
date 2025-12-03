// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "HarmoniaInventoryFragment_Durability.generated.h"

/**
 * Inventory Fragment for items with durability
 * Extends Lyra's inventory system with RPG-style item durability
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaInventoryFragment_Durability : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	/** Maximum durability of this item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability")
	float MaxDurability = 100.0f;

	/** Durability loss per use */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability")
	float DurabilityLossPerUse = 1.0f;

	/** Whether this item is destroyed when durability reaches 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability")
	bool bDestroyWhenBroken = false;

	/** Whether this item can be repaired */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability")
	bool bCanBeRepaired = true;

	virtual void OnInstanceCreated(ULyraInventoryItemInstance* Instance) const override;
};
