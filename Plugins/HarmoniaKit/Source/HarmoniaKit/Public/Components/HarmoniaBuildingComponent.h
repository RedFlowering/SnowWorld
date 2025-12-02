// Copyright 2025 Snow Game Studio.

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

UCLASS(ClassGroup=(Custom), meta = ( BlueprintSpawnableComponent ))
class HARMONIAKIT_API UHarmoniaBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBuildingComponent();

	// Î™®Îìú ?úÏñ¥ (Client requests)
	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestEnterBuildingMode();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestExitBuildingMode();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestSetBuildingMode(EBuildingMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void RequestSetSelectedPart(FName PartID);

	// ?ÑÏû¨ ?†ÌÉù???åÌä∏ ?ïÎ≥¥
	UFUNCTION(BlueprintPure, Category = "Building")
	FName GetSelectedPartID() const { return SelectedPartID; }

	UFUNCTION(BlueprintPure, Category = "Building")
	EBuildingMode GetCurrentMode() const { return CurrentMode; }

	// Tick Ï≤òÎ¶¨
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	// ?ÖÎ†• Ï≤òÎ¶¨
	void SetupInput();
	void HandlePlaceAction();
	void HandleRotateAction();
	void HandleCancelAction();

	// ?ÑÎ¶¨Î∑?Í¥Ä???®Ïàò
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

	// Î∞∞Ïπò Ï≤òÎ¶¨
	void PlaceCurrentPart();
	bool ValidatePlacement(FVector& OutLocation, FRotator& OutRotation);

	// ?†Ìã∏ ?®Ïàò
	FHarmoniaBuildingPartData* GetCurrentPartData() const;

	// ?êÏõê Í≤Ä??(Server-only)
	bool CheckAndConsumeResources(const FHarmoniaBuildingPartData& PartData);

private:
	// ?ÅÌÉú
	EBuildingMode CurrentMode = EBuildingMode::None;

	UPROPERTY()
	FName SelectedPartID;

	// ?∏Î? Ï∞∏Ï°∞
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC = nullptr;

	UPROPERTY()
	TObjectPtr<AHarmoniaBuildingPreviewActor> PreviewActor = nullptr;

	UPROPERTY()
	TObjectPtr<UHarmoniaBuildingInstanceManager> InstanceManager = nullptr;

	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent = nullptr;

	// ?§Ï†ï
	UPROPERTY(EditDefaultsOnly, Category = "Building")
	TSubclassOf<AHarmoniaBuildingPreviewActor> PreviewActorClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMapping = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Building|DataTable")
	FName BuildingDataTableKey = FName(TEXT("BuildingParts"));

	// Î∞∞Ïπò Í≤ÄÏ¶??§Ï†ï
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float MaxPlacementDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float SnapDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float GridSize = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bUseGridSnapping = true;

	// ?§ÎÉÖ ?¨Ïù∏??Í≤Ä??Î∞òÍ≤Ω
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float SnapSearchRadius = 200.0f;

	// ?§ÎÉÖ ?¨Ïù∏???∞ÏÑ† ?¨Ïö© ?¨Î?
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bPreferSnapPoints = true;

	// ÏµúÎ? ?àÏö© ÏßÄ??Í≤ΩÏÇ¨ Í∞ÅÎèÑ
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float MaxAllowedSlope = 45.0f;

	// ÏßÄ??Í≤ΩÏÇ¨ Í≤Ä???úÏÑ±???¨Î?
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bCheckTerrainSlope = true;

	// ?ÑÏû¨ ?åÏ†Ñ Í∞ÅÎèÑ (?ÑÏ†Å)
	float CurrentRotationYaw = 0.0f;

	// Ï∫êÏãú??Í±¥Ï∂ïÎ¨??∞Ïù¥???åÏù¥Î∏?
	UPROPERTY()
	TObjectPtr<UDataTable> CachedBuildingDataTable = nullptr;
};
