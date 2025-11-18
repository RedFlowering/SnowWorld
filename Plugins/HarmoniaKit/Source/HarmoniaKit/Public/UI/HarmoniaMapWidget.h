// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapWidget.generated.h"

class UHarmoniaMapComponent;
class UHarmoniaMapMarkerWidget;
class UImage;
class UCanvasPanel;
class UMaterialInstanceDynamic;

/**
 * Main map widget for displaying the game map
 */
UCLASS(Abstract, Blueprintable)
class HARMONIAKIT_API UHarmoniaMapWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UHarmoniaMapWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// Map image widget (bind in blueprint)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> MapImage = nullptr;

	// Fog of war image (bind in blueprint)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> FogOfWarImage = nullptr;

	// Canvas for markers (bind in blueprint)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MarkerCanvas = nullptr;

	// Player marker widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Markers")
	TSubclassOf<UHarmoniaMapMarkerWidget> PlayerMarkerClass;

	// Friend marker widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Markers")
	TSubclassOf<UHarmoniaMapMarkerWidget> FriendMarkerClass;

	// Ping marker widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Markers")
	TSubclassOf<UHarmoniaMapMarkerWidget> PingMarkerClass;

	// POI marker widget class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|Markers")
	TSubclassOf<UHarmoniaMapMarkerWidget> POIMarkerClass;

	// Fog of war material (for masking explored regions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|FogOfWar")
	TObjectPtr<UMaterialInterface> FogOfWarMaterial = nullptr;

	// Update interval for markers (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float MarkerUpdateInterval = 0.1f;

	// Whether to show fog of war
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map|FogOfWar")
	bool bShowFogOfWar = true;

	// Zoom level (1.0 = normal, >1 = zoomed in, <1 = zoomed out)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float ZoomLevel = 1.0f;

	// Min zoom level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float MinZoom = 0.5f;

	// Max zoom level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	float MaxZoom = 3.0f;

	// Pan offset (for map panning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map")
	FVector2D PanOffset = FVector2D::ZeroVector;

	// Initialize map with map component
	UFUNCTION(BlueprintCallable, Category = "Map")
	void InitializeMap(UHarmoniaMapComponent* InMapComponent);

	// Update map display
	UFUNCTION(BlueprintCallable, Category = "Map")
	void UpdateMap();

	// Update markers
	UFUNCTION(BlueprintCallable, Category = "Map")
	void UpdateMarkers();

	// Update fog of war
	UFUNCTION(BlueprintCallable, Category = "Map")
	void UpdateFogOfWar();

	// Zoom in
	UFUNCTION(BlueprintCallable, Category = "Map")
	void ZoomIn(float Amount = 0.1f);

	// Zoom out
	UFUNCTION(BlueprintCallable, Category = "Map")
	void ZoomOut(float Amount = 0.1f);

	// Set zoom level
	UFUNCTION(BlueprintCallable, Category = "Map")
	void SetZoom(float NewZoom);

	// Pan map
	UFUNCTION(BlueprintCallable, Category = "Map")
	void PanMap(FVector2D Delta);

	// Center map on player
	UFUNCTION(BlueprintCallable, Category = "Map")
	void CenterOnPlayer();

	// Center map on world position
	UFUNCTION(BlueprintCallable, Category = "Map")
	void CenterOnWorldPosition(const FVector& WorldPosition);

	// Convert screen position to world position
	UFUNCTION(BlueprintCallable, Category = "Map")
	FVector ScreenToWorld(FVector2D ScreenPosition);

	// Convert world position to screen position
	UFUNCTION(BlueprintCallable, Category = "Map")
	FVector2D WorldToScreen(const FVector& WorldPosition);

	// Handle ping creation from UI
	UFUNCTION(BlueprintCallable, Category = "Map")
	void CreatePingAtScreenPosition(FVector2D ScreenPosition);

	// Get map component
	UFUNCTION(BlueprintCallable, Category = "Map")
	UHarmoniaMapComponent* GetMapComponent() const { return MapComponent; }

protected:
	// Map component reference
	UPROPERTY(BlueprintReadOnly, Category = "Map")
	TObjectPtr<UHarmoniaMapComponent> MapComponent = nullptr;

	// Dynamic material instance for fog of war
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FogOfWarMaterialInstance = nullptr;

	// Active marker widgets
	UPROPERTY()
	TArray<TObjectPtr<UHarmoniaMapMarkerWidget>> ActiveMarkers;

	// Player marker widget
	UPROPERTY()
	TObjectPtr<UHarmoniaMapMarkerWidget> PlayerMarker = nullptr;

	// Timer for marker updates
	float MarkerUpdateTimer = 0.0f;

	// Create a marker widget
	UHarmoniaMapMarkerWidget* CreateMarker(TSubclassOf<UHarmoniaMapMarkerWidget> MarkerClass);

	// Remove all markers
	void ClearMarkers();

	// Update player marker
	void UpdatePlayerMarker();

	// Update ping markers
	void UpdatePingMarkers();

	// Update POI markers
	void UpdatePOIMarkers();

	// Event handlers
	UFUNCTION()
	void OnLocationDiscovered(const FMapLocationData& Location);

	UFUNCTION()
	void OnPingCreated(const FMapPingData& Ping);

	UFUNCTION()
	void OnRegionExplored(const FExploredRegion& Region);
};
