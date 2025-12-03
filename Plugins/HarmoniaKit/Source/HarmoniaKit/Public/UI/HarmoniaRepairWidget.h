// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HarmoniaEnhancementSystemComponent.h"
#include "HarmoniaRepairWidget.generated.h"

/**
 * Repair Widget
 * UI for repairing equipment at repair stations or using repair kits
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaRepairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Initialization
	// ============================================================================

	/** Initialize widget with enhancement system component */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	void InitializeWidget(UHarmoniaEnhancementSystemComponent* InEnhancementComponent);

	/** Set repair station (null for no station) */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	void SetRepairStation(AActor* InRepairStation);

	// ============================================================================
	// Item Selection
	// ============================================================================

	/** Set item to repair */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	void SetItemToRepair(FGuid ItemGUID);

	/** Get currently selected item */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	FGuid GetSelectedItem() const { return SelectedItemGUID; }

	/** Get item enhancement data */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	bool GetItemData(FEnhancedItemData& OutData) const;

	// ============================================================================
	// Repair Options
	// ============================================================================

	/** Can repair selected item? */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	bool CanRepair(FString& OutReason) const;

	/** Get repair cost for selected item */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	int32 GetRepairCost(bool bFullRepair = true) const;

	/** Get repair cost text (formatted) */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	FText GetRepairCostText(bool bFullRepair = true) const;

	/** Repair selected item */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	void RepairItem(bool bFullRepair = true);

	/** Repair selected item with kit */
	UFUNCTION(BlueprintCallable, Category = "Repair Widget")
	void RepairItemWithKit(FHarmoniaID RepairKitId);

	// ============================================================================
	// Display Info
	// ============================================================================

	/** Get durability percentage */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	float GetDurabilityPercent() const;

	/** Get durability text */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	FText GetDurabilityText() const;

	/** Get durability bar color */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	FLinearColor GetDurabilityColor() const;

	/** Should show durability warning? */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	bool ShouldShowWarning() const;

	/** Get repair station name */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	FText GetRepairStationName() const;

	/** Get repair station discount percentage */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	float GetRepairStationDiscount() const;

	/** Has active repair station? */
	UFUNCTION(BlueprintPure, Category = "Repair Widget")
	bool HasRepairStation() const { return RepairStation != nullptr; }

	// ============================================================================
	// Events (implement in Blueprint)
	// ============================================================================

	/** Called when repair starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Repair Widget|Events")
	void OnRepairStarted();

	/** Called when repair completes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Repair Widget|Events")
	void OnRepairCompleted(float NewDurability);

	/** Called when item selection changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Repair Widget|Events")
	void OnItemSelectionChanged(FGuid NewItemGUID);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ============================================================================
	// Event Handlers
	// ============================================================================

	/** Handle item repaired event */
	UFUNCTION()
	void HandleItemRepaired(FGuid ItemGUID, float NewDurability);

	// ============================================================================
	// Properties
	// ============================================================================

	/** Enhancement system component reference */
	UPROPERTY(BlueprintReadOnly, Category = "Repair Widget")
	TObjectPtr<UHarmoniaEnhancementSystemComponent> EnhancementComponent = nullptr;

	/** Current repair station */
	UPROPERTY(BlueprintReadOnly, Category = "Repair Widget")
	TObjectPtr<AActor> RepairStation = nullptr;

	/** Selected item GUID */
	UPROPERTY(BlueprintReadOnly, Category = "Repair Widget")
	FGuid SelectedItemGUID;

	// ============================================================================
	// Display Settings
	// ============================================================================

	/** Warning threshold for durability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Widget|Display")
	float WarningThreshold = 0.25f;

	/** Color for high durability (> 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Widget|Display")
	FLinearColor HighDurabilityColor = FLinearColor::Green;

	/** Color for medium durability (25% - 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Widget|Display")
	FLinearColor MediumDurabilityColor = FLinearColor::Yellow;

	/** Color for low durability (< 25%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Widget|Display")
	FLinearColor LowDurabilityColor = FLinearColor::Red;
};
