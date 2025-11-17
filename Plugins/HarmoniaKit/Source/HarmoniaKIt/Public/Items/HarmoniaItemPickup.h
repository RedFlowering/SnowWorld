// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaMonsterSystemDefinitions.h"
#include "Interactable/HarmoniaInteractableInterface.h"
#include "HarmoniaItemPickup.generated.h"

class UStaticMeshComponent;
class USenseStimulusComponent;
class UHarmoniaInteractionComponent;
class UWidgetComponent;

/**
 * AHarmoniaItemPickup
 *
 * Actor that represents a dropped item in the world
 * Can be picked up by players through interaction
 * Integrates with Sense System for detection
 * Supports auto-pickup and manual pickup modes
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API AHarmoniaItemPickup : public AActor, public IHarmoniaInteractableInterface
{
	GENERATED_BODY()

public:
	AHarmoniaItemPickup();

	//~AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of AActor interface

	//~IHarmoniaInteractableInterface
	virtual bool CanInteract_Implementation(AActor* InteractingActor) const override;
	virtual void Interact_Implementation(AActor* InteractingActor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual float GetInteractionDuration_Implementation() const override;
	//~End of IHarmoniaInteractableInterface

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Loot item data
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup", Replicated)
	FHarmoniaLootTableRow LootItem;

	/**
	 * Quantity of this item
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup", Replicated)
	int32 Quantity = 1;

	/**
	 * Gold amount (if this is a gold pickup)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup", Replicated)
	int32 GoldAmount = 0;

	/**
	 * Whether this item can be auto-picked up
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup")
	bool bAutoPickup = false;

	/**
	 * Auto-pickup range
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup", meta = (EditCondition = "bAutoPickup"))
	float AutoPickupRange = 200.0f;

	/**
	 * Lifetime before despawning (-1 for infinite)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup")
	float LifeTime = 120.0f;

	/**
	 * Visual mesh for the item
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Pickup")
	TObjectPtr<UStaticMesh> ItemMesh = nullptr;

	// ============================================================================
	// Components
	// ============================================================================

	/**
	 * Static mesh component for visual representation
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;

	/**
	 * Sense stimulus component for detection
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USenseStimulusComponent> SenseStimulusComponent = nullptr;

	/**
	 * Interactable component for interaction system
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHarmoniaInteractionComponent> InteractableComponent = nullptr;

	/**
	 * Widget component for displaying item name/info
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> WidgetComponent = nullptr;

	// ============================================================================
	// Public Functions
	// ============================================================================

	/**
	 * Initialize the pickup with loot data
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Pickup")
	void InitializePickup(const FHarmoniaLootTableRow& InLootItem, int32 InQuantity);

	/**
	 * Initialize as gold pickup
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Pickup")
	void InitializeGoldPickup(int32 InGoldAmount);

	/**
	 * Try to pick up this item
	 * @return true if successfully picked up
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Pickup")
	bool TryPickup(AActor* Collector);

	/**
	 * Get display name for this pickup
	 */
	UFUNCTION(BlueprintCallable, Category = "Item Pickup")
	FText GetDisplayName() const;

protected:
	// ============================================================================
	// Helper Functions
	// ============================================================================

	/**
	 * Check for nearby players for auto-pickup
	 */
	void CheckAutoPickup();

	/**
	 * Handle lifetime expiration
	 */
	UFUNCTION()
	void OnLifetimeExpired();

	/**
	 * Apply item visual effects (floating, rotation, glow)
	 */
	void ApplyVisualEffects();

	// ============================================================================
	// Internal State
	// ============================================================================

	/**
	 * Timer handle for lifetime
	 */
	FTimerHandle LifetimeTimerHandle;

	/**
	 * Whether this pickup has been collected
	 */
	bool bCollected = false;

	/**
	 * Time since spawn (for visual effects)
	 */
	float TimeSinceSpawn = 0.0f;
};
