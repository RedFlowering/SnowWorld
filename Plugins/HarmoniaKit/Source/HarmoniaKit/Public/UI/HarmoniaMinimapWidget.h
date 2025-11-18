// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HarmoniaMinimapWidget.generated.h"

class UHarmoniaMapComponent;
class UHarmoniaMapMarkerWidget;
class UImage;
class UCanvasPanel;
class UMaterialInstanceDynamic;

/**
 * Minimap widget - always visible small map in corner of screen
 */
UCLASS(Abstract, Blueprintable)
class HARMONIAKIT_API UHarmoniaMinimapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UHarmoniaMinimapWidget(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    // Minimap image widget (bind in blueprint)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UImage> MinimapImage;

    // Fog of war overlay (bind in blueprint)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
    TObjectPtr<UImage> FogOverlay;

    // Canvas for markers (bind in blueprint)
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    TObjectPtr<UCanvasPanel> MarkerCanvas;

    // Player marker widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Markers")
    TSubclassOf<UHarmoniaMapMarkerWidget> PlayerMarkerClass;

    // Friend marker widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Markers")
    TSubclassOf<UHarmoniaMapMarkerWidget> FriendMarkerClass;

    // Ping marker widget class
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|Markers")
    TSubclassOf<UHarmoniaMapMarkerWidget> PingMarkerClass;

    // Minimap size (in world units from player)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MinimapRange = 5000.0f;

    // Whether to rotate with player (true) or keep north up (false)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    bool bRotateWithPlayer = false;

    // Update interval for minimap (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float UpdateInterval = 0.1f;

    // Whether to show fog of war on minimap
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap|FogOfWar")
    bool bShowFogOfWar = true;

    // Initialize minimap with map component
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void InitializeMinimap(UHarmoniaMapComponent* InMapComponent);

    // Update minimap display
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void UpdateMinimap();

    // Set minimap range
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void SetMinimapRange(float NewRange);

    // Get map component
    UFUNCTION(BlueprintPure, Category = "Minimap")
    UHarmoniaMapComponent* GetMapComponent() const { return MapComponent; }

protected:
    // Map component reference
    UPROPERTY(BlueprintReadOnly, Category = "Minimap")
    TObjectPtr<UHarmoniaMapComponent> MapComponent;

    // Dynamic material instance for minimap
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> MinimapMaterialInstance;

    // Dynamic material instance for fog of war
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> FogMaterialInstance;

    // Player marker widget
    UPROPERTY()
    TObjectPtr<UHarmoniaMapMarkerWidget> PlayerMarker;

    // Active markers
    UPROPERTY()
    TArray<TObjectPtr<UHarmoniaMapMarkerWidget>> ActiveMarkers;

    // Update timer
    float UpdateTimer = 0.0f;

    // Update player marker
    void UpdatePlayerMarker();

    // Update ping markers
    void UpdatePingMarkers();

    // Update fog of war
    void UpdateFogOfWar();

    // Create a marker widget
    UHarmoniaMapMarkerWidget* CreateMarker(TSubclassOf<UHarmoniaMapMarkerWidget> MarkerClass);
};
