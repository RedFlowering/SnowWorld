// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "SenseStimulusComponent.h"
#include "Interface/HarmoniaInteractableInterface.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"
#include "HarmoniaSenseInteractableComponent.generated.h"

class UHarmoniaSenseInteractableComponent;

/**
 * Delegate signature for sense interaction events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInteractionDelegate,
	AActor*, Interactor,
	UHarmoniaSenseInteractableComponent*, InteractableComponent,
	FName, SensorTag);

/**
 * Sense-Based Interactable Component
 *
 * This component makes an actor interactable through the Sense System.
 * It emits sense stimulus that can be detected by USenseReceiverComponent.
 * Supports multiple interaction configurations (proximity, vision, sound, etc.)
 *
 * Usage:
 * - Attach to any actor that should be interactable
 * - Configure interaction settings (proximity, vision, sound-based, etc.)
 * - Receivers will detect this component and can trigger interactions
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSenseInteractableComponent : public USenseStimulusComponent, public IHarmoniaInteractableInterface
{
	GENERATED_BODY()

public:
	UHarmoniaSenseInteractableComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ============================================================================
	// Interaction Configurations
	// ============================================================================

	/**
	 * Multiple interaction configurations for different sense types
	 * Examples:
	 * - Proximity interaction: Walk near to auto-interact
	 * - Vision interaction: Look at object to highlight
	 * - Manual interaction: Press button when close
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	TArray<FSenseInteractionData> InteractionConfigs;

	/**
	 * Main interaction range (used as default for proximity sensors)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	float DefaultInteractionRange = 200.0f;

	/**
	 * Whether this interactable is currently active
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	bool bIsSenseActive = true;

	/**
	 * Debug visualization settings
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo = false;

	// ============================================================================
	// Interaction Interface Implementation
	// ============================================================================

	/**
	 * Called when an interaction is attempted
	 * This implements the IHarmoniaInteractableInterface
	 */
	virtual void OnInteract_Implementation(
		const FHarmoniaInteractionContext& Context,
		FHarmoniaInteractionResult& OutResult) override;

	// ============================================================================
	// Sense Interaction Functions
	// ============================================================================

	/**
	 * Get interaction config for a specific sensor tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool GetInteractionConfigBySensorTag(FName SensorTag, FSenseInteractionData& OutData) const;

	/**
	 * Get interaction config by trigger type
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool GetInteractionConfigByTriggerType(ESenseInteractionTriggerType TriggerType, FSenseInteractionData& OutData) const;

	/**
	 * Add a new interaction configuration at runtime
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	void AddInteractionConfig(const FSenseInteractionConfig& Config);

	/**
	 * Remove interaction configuration by sensor tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool RemoveInteractionConfig(FName SensorTag);

	/**
	 * Enable/Disable specific interaction
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	void SetInteractionEnabled(FName SensorTag, bool bEnabled);

	/**
	 * Enable/Disable all interactions
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	void SetAllInteractionsEnabled(bool bEnabled);

	/**
	 * Reset interaction data (cooldowns, counters, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	void ResetInteractionData(FName SensorTag);

	/**
	 * Reset all interaction data
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	void ResetAllInteractionData();

	/**
	 * Check if interaction is available for a specific sensor
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool IsInteractionAvailable(FName SensorTag) const;

	/**
	 * Get interaction prompt text for UI display
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sense Interaction")
	FText GetInteractionPrompt(FName SensorTag) const;

	// ============================================================================
	// Events
	// ============================================================================

	/**
	 * Called when an interaction is triggered through sense system
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnInteractionDelegate OnSenseInteractionTriggered;

	/**
	 * Called when interactable is sensed (detected)
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnInteractionDelegate OnSensed;

	/**
	 * Called when interactable is no longer sensed (lost)
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnInteractionDelegate OnLostSense;

protected:
	/**
	 * Initialize default interaction configurations
	 */
	virtual void InitializeDefaultInteractions();

	/**
	 * Process interaction trigger
	 */
	virtual bool ProcessInteraction(
		AActor* Interactor,
		FSenseInteractionData& InteractionData,
		FHarmoniaInteractionResult& OutResult);

	/**
	 * Validate interaction conditions
	 */
	virtual bool ValidateInteractionConditions(
		AActor* Interactor,
		const FSenseInteractionData& InteractionData) const;

	/**
	 * Update interaction data after successful interaction
	 */
	virtual void UpdateInteractionState(FSenseInteractionData& InteractionData);
};
