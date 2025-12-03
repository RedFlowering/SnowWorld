// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingPreviewActor.generated.h"

/**
 * @class AHarmoniaBuildingPreviewActor
 * @brief Actor that displays building placement preview
 * 
 * Features:
 * - Semi-transparent mesh showing placement location
 * - Color changes based on placement validity
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaBuildingPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaBuildingPreviewActor();

protected:
	virtual void BeginPlay() override;

public:
	/** Apply preview data from building part */
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void ApplyPreviewData(const FHarmoniaBuildingPartData& PartData);

	/** Update visual feedback based on placement validity */
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void SetIsPlacementValid(bool bIsValid);

	/** Rotate the preview */
	UFUNCTION(BlueprintCallable, Category = "Building Preview")
	void RotatePreview(float DeltaYaw);

protected:
	/** Preview mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PreviewMeshComponent = nullptr;

	/** Material for valid placement state */
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> ValidPlacementMaterial = nullptr;

	/** Material for invalid placement state */
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TObjectPtr<UMaterialInterface> InvalidPlacementMaterial = nullptr;

	/** Current placement validity state */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCurrentlyValid = false;

	/** Currently applied part data */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FHarmoniaBuildingPartData CurrentPartData;
};
