// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HarmoniaEnhancementSystemComponent.h"
#include "HarmoniaDurabilityBarWidget.generated.h"

/**
 * Durability Bar Widget
 * Simple widget to display item durability as a progress bar
 * Can be used in inventory slots, equipment panels, etc.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaDurabilityBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Initialization
	// ============================================================================

	/** Set the item to display durability for */
	UFUNCTION(BlueprintCallable, Category = "Durability Bar")
	void SetItem(UHarmoniaEnhancementSystemComponent* InEnhancementComponent, FGuid ItemGUID);

	/** Clear the current item */
	UFUNCTION(BlueprintCallable, Category = "Durability Bar")
	void ClearItem();

	// ============================================================================
	// Display Properties
	// ============================================================================

	/** Get durability percentage (0.0 - 1.0) */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	float GetDurabilityPercent() const;

	/** Get durability text (e.g., "75/100") */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	FText GetDurabilityText() const;

	/** Get durability bar color based on current durability */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	FLinearColor GetDurabilityBarColor() const;

	/** Should show warning icon? */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	bool ShouldShowWarningIcon() const;

	/** Should show broken icon? */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	bool IsBroken() const;

	/** Get performance penalty percentage (0 - 100) */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	float GetPenaltyPercent() const;

	/** Has performance penalty? */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	bool HasPenalty() const;

	/** Get penalty tooltip text */
	UFUNCTION(BlueprintPure, Category = "Durability Bar")
	FText GetPenaltyTooltipText() const;

	// ============================================================================
	// Events (implement in Blueprint)
	// ============================================================================

	/** Called when durability changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Durability Bar|Events")
	void OnDurabilityChanged(float NewPercent);

	/** Called when item becomes broken */
	UFUNCTION(BlueprintImplementableEvent, Category = "Durability Bar|Events")
	void OnItemBroken();

	/** Called when item is repaired */
	UFUNCTION(BlueprintImplementableEvent, Category = "Durability Bar|Events")
	void OnItemRepaired(float NewDurability);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============================================================================
	// Properties
	// ============================================================================

	/** Enhancement system component reference */
	UPROPERTY(BlueprintReadOnly, Category = "Durability Bar")
	TObjectPtr<UHarmoniaEnhancementSystemComponent> EnhancementComponent = nullptr;

	/** Item GUID */
	UPROPERTY(BlueprintReadOnly, Category = "Durability Bar")
	FGuid ItemGUID;

	/** Last known durability (for change detection) */
	UPROPERTY()
	float LastDurability = 100.0f;

	/** Was broken last frame? */
	UPROPERTY()
	bool bWasBroken = false;

	// ============================================================================
	// Display Settings
	// ============================================================================

	/** Warning threshold (show warning below this %) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability Bar|Display")
	float WarningThreshold = 0.25f;

	/** Update frequency (seconds between checks) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability Bar|Display")
	float UpdateFrequency = 0.5f;

	/** Accumulated time since last update */
	UPROPERTY()
	float TimeSinceLastUpdate = 0.0f;

	/** Color for high durability (> 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability Bar|Display")
	FLinearColor HighDurabilityColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green

	/** Color for medium durability (25% - 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability Bar|Display")
	FLinearColor MediumDurabilityColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	/** Color for low durability (< 25%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability Bar|Display")
	FLinearColor LowDurabilityColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red

	/** Color for broken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability Bar|Display")
	FLinearColor BrokenColor = FLinearColor(0.5f, 0.0f, 0.0f, 1.0f); // Dark Red
};
