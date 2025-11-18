// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapMarkerWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * Widget for map markers (player, friends, pings, POIs)
 */
UCLASS(Abstract, Blueprintable)
class HARMONIAKIT_API UHarmoniaMapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UHarmoniaMapMarkerWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// Marker icon (bind in blueprint)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> MarkerIcon = nullptr;

	// Marker label (bind in blueprint)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MarkerLabel = nullptr;

	// Marker type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	EMapMarkerType MarkerType = EMapMarkerType::Custom;

	// Marker color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FLinearColor MarkerColor = FLinearColor::White;

	// Marker size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	FVector2D MarkerSize = FVector2D(32.0f, 32.0f);

	// Whether to rotate with player/camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marker")
	bool bRotateWithPlayer = false;

	// Current rotation (degrees)
	UPROPERTY(BlueprintReadOnly, Category = "Marker")
	float CurrentRotation = 0.0f;

	// Current screen position
	UPROPERTY(BlueprintReadOnly, Category = "Marker")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	// Update marker position and rotation
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void UpdateMarker(FVector2D NewScreenPosition, float NewRotation = 0.0f);

	// Set marker color
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void SetMarkerColor(FLinearColor NewColor);

	// Set marker label text
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void SetMarkerLabel(const FText& NewLabel);

	// Set marker icon
	UFUNCTION(BlueprintCallable, Category = "Marker")
	void SetMarkerIcon(UTexture2D* NewIcon);

	// Animate marker (for pings, discoveries, etc.)
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Marker")
	void PlayMarkerAnimation();

protected:
	// Update visual appearance
	void UpdateVisuals();
};
