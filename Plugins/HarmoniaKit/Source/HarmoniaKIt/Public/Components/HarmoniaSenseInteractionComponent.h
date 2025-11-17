// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SenseReceiverComponent.h"
#include "SensedStimulStruct.h"
#include "Definitions/HarmoniaInteractionSystemDefinitions.h"
#include "Components/HarmoniaSenseInteractableComponent.h"
#include "HarmoniaSenseInteractionComponent.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;


// ============================================================================
// Delegate Signatures
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableSensedDelegate, UHarmoniaSenseInteractableComponent*, Interactable, FName, SensorTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractableLostDelegate, UHarmoniaSenseInteractableComponent*, Interactable, FName, SensorTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBestTargetChangedDelegate, UHarmoniaSenseInteractableComponent*, NewBestTarget, UHarmoniaSenseInteractableComponent*, OldBestTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractionCompletedDelegate, UHarmoniaSenseInteractableComponent*, Target, FName, SensorTag, const FHarmoniaInteractionResult&, Result);

/**
 * Interactable Target Info
 * Contains information about a sensed target (interactable or general actor)
 */
USTRUCT(BlueprintType)
struct FInteractableTargetInfo
{
	GENERATED_BODY()

	/** The target actor (always valid if target exists) */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> TargetActor = nullptr;

	/** The interactable component (optional, only for interactables) */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UHarmoniaSenseInteractableComponent> InteractableComponent = nullptr;

	/** The sensed stimulus data */
	UPROPERTY(BlueprintReadOnly)
	FSensedStimulus StimulusData;

	/** Sensor tag that detected this target */
	UPROPERTY(BlueprintReadOnly)
	FName SensorTag = NAME_None;

	/** Priority score (higher = more important) */
	UPROPERTY(BlueprintReadOnly)
	float Priority = 0.0f;

	/** Distance to target */
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;

	bool IsValid() const
	{
		return TargetActor != nullptr && TargetActor->IsValidLowLevel();
	}

	bool IsInteractable() const
	{
		return InteractableComponent != nullptr && InteractableComponent->IsValidLowLevel();
	}
};

/**
 * Sense-Based Interaction Component
 *
 * This component extends USenseReceiverComponent to provide easy-to-use interaction features.
 * Inherits all Sense System capabilities while adding interaction-specific functionality.
 * Supports multiple interaction methods: proximity, vision, sound-based, etc.
 *
 * Usage:
 * - Attach to player character or AI
 * - Configure sensors directly (inherits from SenseReceiverComponent)
 * - Bind input actions for manual interactions
 * - Automatic interactions will trigger based on sensor events
 *
 * Benefits of inheritance:
 * - Direct access to all SenseReceiverComponent features
 * - No need for separate component reference
 * - Cleaner architecture and easier to extend
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSenseInteractionComponent : public USenseReceiverComponent
{
	GENERATED_BODY()

public:
	UHarmoniaSenseInteractionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// Interaction Settings
	// ============================================================================

	/**
	 * Sensor tags to monitor for interactions
	 * Examples: "Proximity", "Vision", "Hearing"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	TArray<FName> MonitoredSensorTags;

	/**
	 * Maximum number of simultaneous interactable targets to track
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	int32 MaxTrackedTargets = 10;

	/**
	 * How to prioritize targets when multiple are available
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	bool bPrioritizeByDistance = true;

	/**
	 * Whether to automatically interact with proximity-based interactables
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	bool bEnableAutomaticInteractions = true;

	/**
	 * Track only HarmoniaSenseInteractableComponent (false = track all sensed actors)
	 * Set to false for AI combat targeting, true for player interactions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Interaction")
	bool bInteractableOnly = true;

	/**
	 * Debug visualization
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo = false;

	// ============================================================================
	// Enhanced Input Support
	// ============================================================================

	/**
	 * Input action for manual interaction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction = nullptr;

	/**
	 * Input mapping context for interaction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMapping = nullptr;

	/**
	 * Input mapping priority
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 InputMappingPriority = 1;

	// ============================================================================
	// Target Management
	// ============================================================================

	/**
	 * Get the current best interaction target
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sense Interaction")
	FInteractableTargetInfo GetBestInteractionTarget() const;

	/**
	 * Get all currently sensed interactable targets
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sense Interaction")
	TArray<FInteractableTargetInfo> GetAllInteractableTargets() const;

	/**
	 * Get interactables by sensor tag
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sense Interaction")
	TArray<FInteractableTargetInfo> GetInteractablesBySensorTag(FName SensorTag) const;

	/**
	 * Check if any interactable targets are available
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sense Interaction")
	bool HasInteractableTargets() const;

	// ============================================================================
	// Interaction Functions
	// ============================================================================

	/**
	 * Perform interaction with the best available target
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool TryInteractWithBestTarget();

	/**
	 * Perform interaction with specific target
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool TryInteractWithTarget(UHarmoniaSenseInteractableComponent* Target);

	/**
	 * Perform interaction with target by sensor tag
	 */
	UFUNCTION(BlueprintCallable, Category = "Sense Interaction")
	bool TryInteractWithSensor(FName SensorTag);

	/**
	 * Server RPC for interaction
	 */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Sense Interaction")
	void Server_TryInteract(UHarmoniaSenseInteractableComponent* Target, FName SensorTag);

	/**
	 * Client RPC for interaction result
	 */
	UFUNCTION(Client, Reliable, Category = "Sense Interaction")
	void Client_OnInteractionResult(const FHarmoniaInteractionResult& Result);

	// ============================================================================
	// Events
	// ============================================================================

	/**
	 * Called when a new interactable is sensed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnInteractableSensedDelegate OnInteractableSensed;

	/**
	 * Called when an interactable is lost from sensing
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnInteractableLostDelegate OnInteractableLost;

	/**
	 * Called when the best target changes
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnBestTargetChangedDelegate OnBestTargetChanged;

	/**
	 * Called when an interaction is completed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sense Interaction")
	FOnInteractionCompletedDelegate OnInteractionCompleted;

protected:
	/**
	 * Setup input bindings
	 */
	virtual void SetupInput();

	/**
	 * Cleanup input bindings
	 */
	virtual void CleanupInput();

	/**
	 * Handle interact input action
	 */
	virtual void HandleInteractAction(const FInputActionValue& Value);

	// ============================================================================
	// Sense System Callbacks
	// ============================================================================

	/**
	 * Called when a new sense is detected
	 */
	UFUNCTION()
	virtual void OnNewSenseDetected(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli);

	/**
	 * Called when a sense is lost
	 */
	UFUNCTION()
	virtual void OnSenseLost(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli);

	/**
	 * Called for current sense updates
	 */
	UFUNCTION()
	virtual void OnSenseUpdated(const USensorBase* SensorPtr, int32 Channel, const TArray<FSensedStimulus> SensedStimuli);

	// ============================================================================
	// Target Processing
	// ============================================================================

	/**
	 * Process sensed stimulus to extract interactable components
	 */
	virtual void ProcessSensedStimuli(
		const TArray<FSensedStimulus>& SensedStimuli,
		FName SensorTag,
		bool bIsNewSense);

	/**
	 * Add interactable target to tracking
	 */
	virtual void AddInteractableTarget(
		UHarmoniaSenseInteractableComponent* Interactable,
		const FSensedStimulus& Stimulus,
		FName SensorTag);

	/**
	 * Remove interactable target from tracking
	 */
	virtual void RemoveInteractableTarget(UHarmoniaSenseInteractableComponent* Interactable);

	/**
	 * Update target priority scores
	 */
	virtual void UpdateTargetPriorities();

	/**
	 * Calculate priority for a target
	 */
	virtual float CalculateTargetPriority(const FInteractableTargetInfo& TargetInfo) const;

	/**
	 * Process automatic interactions
	 */
	virtual void ProcessAutomaticInteractions(float DeltaTime);

	/**
	 * Check if automatic interaction should trigger
	 */
	virtual bool ShouldTriggerAutomaticInteraction(
		const FInteractableTargetInfo& TargetInfo,
		const FSenseInteractionData* InteractionData) const;

	// ============================================================================
	// Internal State
	// ============================================================================

	/** Currently tracked interactable targets */
	UPROPERTY(Transient)
	TArray<FInteractableTargetInfo> TrackedTargets;

	/** Current best target index */
	int32 BestTargetIndex = INDEX_NONE;

	/** Last best target (for change detection) */
	TWeakObjectPtr<UHarmoniaSenseInteractableComponent> LastBestTarget = nullptr;
};