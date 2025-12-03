// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaBuildingComponent.h
 * @brief Building system component for construction and placement
 * @author Harmonia Team
 */

#pragma once

#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingComponent.generated.h"

class AHarmoniaBuildingPreviewActor;
class UHarmoniaBuildingInstanceManager;
class APlayerController;
class UHarmoniaInventoryComponent;
class UInputMappingContext;
class UDataTable;

/**
 * @class UHarmoniaBuildingComponent
 * @brief Component for building and construction system
 * 
 * Handles building mode control, part placement, preview actors,
 * and resource validation for construction.
 */
UCLASS(ClassGroup=(Custom), meta = ( BlueprintSpawnableComponent ))
class HARMONIAKIT_API UHarmoniaBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBuildingComponent();

	// Mode Control (Client requests)

	/** Request to enter building mode */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestEnterBuildingMode();

	/** Request to exit building mode */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestExitBuildingMode();

	/** Request to set building mode */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestSetBuildingMode(EBuildingMode NewMode);

	/** Request to set selected building part */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestSetSelectedPart(FName PartID);

	// Current Selected Part Info

	/** Get selected part ID */
	UFUNCTION(BlueprintPure, Category = "Building")
	FName GetSelectedPartID() const { return SelectedPartID; }

	/** Get current building mode */
	UFUNCTION(BlueprintPure, Category = "Building")
	EBuildingMode GetCurrentMode() const { return CurrentMode; }

	// Tick Processing
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	// Input Handling
	void SetupInput();
	void HandlePlaceAction();
	void HandleRotateAction();
	void HandleCancelAction();

	// Preview Related Functions
	void SpawnPreviewActor();
	void UpdatePreviewTransform();
	void DestroyPreviewActor();

	// Server-authoritative functions
	void EnterBuildingMode();
	void ExitBuildingMode();
	void SetBuildingMode(EBuildingMode NewMode);
	void SetSelectedPart(FName PartID);

	// Server RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEnterBuildingMode();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerExitBuildingMode();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetBuildingMode(EBuildingMode NewMode);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSelectedPart(FName PartID);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlacePart(const FVector& Location, const FRotator& Rotation, FName PartID);

	// Placement Processing
	void PlaceCurrentPart();
	bool ValidatePlacement(FVector& OutLocation, FRotator& OutRotation);

	// Utility Functions
	FHarmoniaBuildingPartData* GetCurrentPartData() const;

	// Resource Check (Server-only)
	bool CheckAndConsumeResources(const FHarmoniaBuildingPartData& PartData);

private:
	// State
	EBuildingMode CurrentMode = EBuildingMode::None;

	UPROPERTY()
	FName SelectedPartID;

	// Cached References
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC = nullptr;

	UPROPERTY()
	TObjectPtr<AHarmoniaBuildingPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UHarmoniaBuildingInstanceManager> InstanceManager = nullptr;

	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent = nullptr;

	// Settings
	UPROPERTY(EditDefaultsOnly, Category = "Building")
	TSubclassOf<AHarmoniaBuildingPreviewActor> PreviewActorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMapping = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Building|DataTable")
	FName BuildingDataTableKey = FName(TEXT("BuildingParts"));

	// Placement Check Settings
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float MaxPlacementDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float SnapDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float GridSize = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bUseGridSnapping = true;

	/** Snap point search radius */
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float SnapSearchRadius = 200.0f;

	/** Whether to prefer snap points during placement */
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bPreferSnapPoints = true;

	/** Maximum allowed terrain slope angle */
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float MaxAllowedSlope = 45.0f;

	/** Whether to check terrain slope */
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bCheckTerrainSlope = true;

	/** Current rotation yaw angle (accumulated) */
	float CurrentRotationYaw = 0.0f;

	/** Cached building data table */
	UPROPERTY()
	TObjectPtr<UDataTable> CachedBuildingDataTable = nullptr;
};
