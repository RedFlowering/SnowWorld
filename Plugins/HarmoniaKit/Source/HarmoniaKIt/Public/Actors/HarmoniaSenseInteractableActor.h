// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/HarmoniaSenseInteractableComponent.h"
#include "HarmoniaSenseInteractableActor.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;

/**
 * Sense-Based Interactable Actor
 *
 * Base class for actors that can be interacted with through the Sense System.
 * Includes visual mesh, interaction component, and optional UI widget.
 *
 * This actor demonstrates different interaction types:
 * - Proximity: Automatically trigger when player gets close
 * - Vision: Trigger when player looks at object
 * - Manual: Require button press when in range
 * - Automatic: Instantly trigger when sensed
 *
 * Usage:
 * - Place in level or spawn at runtime
 * - Configure interaction settings in Details panel
 * - Extend in Blueprint for custom interaction logic
 */
UCLASS(Blueprintable, BlueprintType)
class HARMONIAKIT_API AHarmoniaSenseInteractableActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaSenseInteractableActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// Components
	// ============================================================================

	/**
	 * Root scene component
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot = nullptr;

	/**
	 * Visual mesh component
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;

	/**
	 * Sense-based interactable component
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHarmoniaSenseInteractableComponent> InteractableComponent = nullptr;

	/**
	 * Optional widget component for UI prompts
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> WidgetComponent = nullptr;

	// ============================================================================
	// Visual Settings
	// ============================================================================

	/**
	 * Material to use when interactable is idle
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> IdleMaterial = nullptr;

	/**
	 * Material to use when interactable is highlighted (sensed)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> HighlightMaterial = nullptr;

	/**
	 * Material to use when interactable is being interacted with
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> ActiveMaterial = nullptr;

	/**
	 * Material to use when interactable is disabled/used
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UMaterialInterface> DisabledMaterial = nullptr;

	/**
	 * Enable visual feedback
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bEnableVisualFeedback = true;

	/**
	 * Enable widget visibility control
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bEnableWidgetControl = true;

	// ============================================================================
	// Interaction Settings
	// ============================================================================

	/**
	 * Respawn time after interaction (0 = no respawn)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float RespawnTime = 0.0f;

	/**
	 * Destroy actor after interaction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bDestroyAfterInteraction = false;

	/**
	 * Disable actor after interaction (instead of destroy)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bDisableAfterInteraction = false;

	// ============================================================================
	// Sound Effects
	// ============================================================================

	/**
	 * Sound to play when sensed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> SensedSound = nullptr;

	/**
	 * Sound to play when interacted
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> InteractionSound = nullptr;

	// ============================================================================
	// Visual State Functions
	// ============================================================================

	/**
	 * Set visual state (idle, highlighted, active, disabled)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetVisualState(EInteractableVisualState State);

	/**
	 * Show/hide interaction widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetWidgetVisible(bool bVisible);

	// ============================================================================
	// Interaction Events
	// ============================================================================

	/**
	 * Called when actor is sensed by an interactor
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnSensedByInteractor(AActor* Interactor, FName SensorTag);
	virtual void OnSensedByInteractor_Implementation(AActor* Interactor, FName SensorTag);

	/**
	 * Called when actor is lost from sense by an interactor
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnLostFromInteractor(AActor* Interactor, FName SensorTag);
	virtual void OnLostFromInteractor_Implementation(AActor* Interactor, FName SensorTag);

	/**
	 * Called when interaction is triggered
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteractionTriggered(AActor* Interactor, FName SensorTag);
	virtual void OnInteractionTriggered_Implementation(AActor* Interactor, FName SensorTag);

protected:
	/**
	 * Setup interaction event bindings
	 */
	virtual void SetupInteractionBindings();

	/**
	 * Handle respawn timer
	 */
	virtual void RespawnInteractable();

	/**
	 * Disable this interactable
	 */
	virtual void DisableInteractable();

	/**
	 * Enable this interactable
	 */
	virtual void EnableInteractable();

private:
	/** Internal callbacks for component events */
	UFUNCTION()
	void Internal_OnSensed(AActor* Interactor, UHarmoniaSenseInteractableComponent* Component, FName SensorTag);

	UFUNCTION()
	void Internal_OnLostSense(AActor* Interactor, UHarmoniaSenseInteractableComponent* Component, FName SensorTag);

	UFUNCTION()
	void Internal_OnInteraction(AActor* Interactor, UHarmoniaSenseInteractableComponent* Component, FName SensorTag);

	/** Respawn timer handle */
	FTimerHandle RespawnTimerHandle;

	/** Current number of actors sensing this */
	int32 SensingActorCount = 0;
};

/**
 * Visual state for interactable actors
 */
UENUM(BlueprintType)
enum class EInteractableVisualState : uint8
{
	Idle UMETA(DisplayName="Idle"),
	Highlighted UMETA(DisplayName="Highlighted"),
	Active UMETA(DisplayName="Active"),
	Disabled UMETA(DisplayName="Disabled")
};
