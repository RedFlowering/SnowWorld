// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"
#include "HarmoniaSenseComponent.generated.h"

class UAbilitySystemComponent;
class USensorBase;
class UHarmoniaSenseInteractableComponent;
class UHarmoniaSenseInteractionComponent;
struct FSensedStimulus;

/**
 * Delegate for sense detection events
 * Fired when SenseSystem detects targets, for processing by combat components
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSenseHitDelegate, 
	const USensorBase*, Sensor,
	int32, Channel,
	const TArray<FSensedStimulus>&, SensedStimuli);

/**
 * UHarmoniaSenseComponent
 *
 * Manages SenseSystem sensor setup for characters.
 * Responsibilities:
 * - Set up sensors from DataTable (data-driven)
 * - Add/remove sensors on equipment change
 * - Forward detection callbacks to combat components
 *
 * All detection logic is handled by SenseSystem plugin.
 * Attack state and damage processing handled by MeleeCombatComponent.
 */
UCLASS(Blueprintable, ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSenseComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHarmoniaSenseComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	// ============================================================================
	// Sensor Management
	// ============================================================================

	/**
	 * Add a sensor from DataTable configuration
	 * @param SenseConfigRowName Row name in DT_SenseConfig
	 * @return Created sensor, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Sensors")
	USensorBase* AddSensor(FName SenseConfigRowName);

	/**
	 * Remove a sensor by its tag
	 * @param SensorTag Tag of sensor to remove
	 * @return true if sensor was found and removed
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Sensors")
	bool RemoveSensor(FName SensorTag);

	/**
	 * Remove all sensors
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Sensors")
	void RemoveAllSensors();

	/**
	 * Called when equipment changes - updates sensor configuration
	 * Removes old sensors, adds new ones from config list
	 * @param NewSensorConfigs List of SenseConfigRowNames for new equipment
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Equipment")
	void OnEquipmentChanged(const TArray<FName>& NewSensorConfigs);

	/**
	 * Get sensor by tag
	 * @return Sensor with matching tag, or nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Sensors")
	USensorBase* GetSensor(FName SensorTag) const;

	/**
	 * Get all active sensors
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Sensors")
	TArray<USensorBase*> GetAllSensors() const;

	/**
	 * Get sensor configuration from DataTable
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Configuration")
	FHarmoniaSenseConfigData GetSenseConfigByRowName(FName RowName) const;

	/**
	 * Trigger all Manual-type sensors to run their tests
	 * Call this during attack window (e.g., from AnimNotify tick)
	 * Active sensors auto-update via timer, but Manual sensors need explicit trigger
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense|Sensors")
	void TriggerManualSensors();

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when any sensor detects targets
	 * Subscribe to this in combat components to process hits
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense|Events")
	FOnSenseHitDelegate OnSenseHit;

	// ============================================================================
	// Owner Components
	// ============================================================================

	/** Owner's interactable component (Stimulus) - makes owner detectable */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sense|Owner Components")
	TObjectPtr<UHarmoniaSenseInteractableComponent> OwnerInteractable = nullptr;

	/** Owner's interaction component (Receiver) - detects others */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sense|Owner Components")
	TObjectPtr<UHarmoniaSenseInteractionComponent> OwnerInteraction = nullptr;

	/** Owner's ability system (for GAS integration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense|Configuration")
	TObjectPtr<UAbilitySystemComponent> OwnerAbilitySystem = nullptr;

protected:
	// ============================================================================
	// Initialization
	// ============================================================================

	/**
	 * Initialize owner's SenseSystem components
	 */
	virtual void InitializeOwnerSenseComponents();

	/**
	 * Ensure owner has Interactable component (Stimulus)
	 */
	virtual void EnsureOwnerInteractable();

	/**
	 * Ensure owner has Interaction component (Receiver)
	 */
	virtual void EnsureOwnerInteraction();

	/**
	 * Register stimulus responses from DataTable
	 */
	virtual void RegisterStimulusResponsesFromDataTable();

	/**
	 * Initialize sensors from DataTable at BeginPlay
	 */
	virtual void InitializeSensorsFromDataTable();

	// ============================================================================
	// SenseSystem Callbacks
	// ============================================================================

	/**
	 * Handle detection callback from SenseSystem
	 * Forwards to OnSenseHit delegate for combat components
	 */
	UFUNCTION()
	virtual void OnSenseDetected(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli);

private:
	/**
	 * Currently active sensors (managed by OwnerInteraction's SenseReceiver)
	 * Key: SensorTag, Value: Sensor pointer
	 */
	UPROPERTY()
	TMap<FName, TObjectPtr<USensorBase>> ActiveSensors;
};
