// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapCaptureVolume.generated.h"

class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

/**
 * Volume for defining map capture regions
 * Captures the area within the box bounds as map texture(s)
 */
UCLASS(HideCategories=(Collision, Cooking, Actor, HLOD, Physics, Networking, Input, Replication, Rendering))
class HARMONIAEDITOR_API AHarmoniaMapCaptureVolume : public AVolume
{
    GENERATED_BODY()

public:
    AHarmoniaMapCaptureVolume();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditMove(bool bFinished) override;
#endif

public:
    // ========== Capture Settings ==========

    // Resolution per tile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Settings")
    FIntPoint TileResolution = FIntPoint(2048, 2048);

    // Number of tiles (X, Y). Total tiles = TilesX * TilesY
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Settings")
    FIntPoint TileCount = FIntPoint(1, 1);

    // Whether to capture as a single large texture or multiple tiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Settings")
    bool bCaptureSingleTexture = true;

    // Capture height above the center of the volume
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Settings")
    float CaptureHeight = 10000.0f;

    // Camera field of view (only used if not using orthographic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Settings")
    float CameraFOV = 90.0f;

    // Use orthographic projection (recommended for top-down maps)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Settings")
    bool bUseOrthographic = true;

    // ========== Output Settings ==========

    // Output folder path (relative to Content/)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Output")
    FString OutputFolder = TEXT("/Game/Maps/CapturedMaps/");

    // Base name for output files
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Output")
    FString OutputBaseName = TEXT("MapTexture");

    // Whether to create minimap (smaller version)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Output")
    bool bCreateMinimap = true;

    // Minimap resolution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Output", meta = (EditCondition = "bCreateMinimap"))
    FIntPoint MinimapResolution = FIntPoint(512, 512);

    // Whether to automatically create map data asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture|Output")
    bool bAutoCreateDataAsset = true;

    // ========== Capture Actions ==========

#if WITH_EDITORONLY_DATA
    // Preview the capture area
    UPROPERTY(EditAnywhere, Category = "Map Capture|Actions")
    bool bShowCapturePreview = true;

    // Color for capture volume visualization
    UPROPERTY(EditAnywhere, Category = "Map Capture|Actions")
    FLinearColor PreviewColor = FLinearColor::Green;
#endif

    // Capture the map
    UFUNCTION(CallInEditor, Category = "Map Capture|Actions")
    void CaptureMap();

    // Capture as multiple tiles
    UFUNCTION(CallInEditor, Category = "Map Capture|Actions")
    void CaptureMapTiles();

    // Preview capture (creates temporary render target)
    UFUNCTION(CallInEditor, Category = "Map Capture|Actions")
    void PreviewCapture();

    // Calculate optimal tile count based on volume size
    UFUNCTION(CallInEditor, Category = "Map Capture|Actions")
    void CalculateOptimalTiles();

    // ========== Info Display ==========

    // Total texture resolution (calculated)
    UPROPERTY(VisibleAnywhere, Category = "Map Capture|Info")
    FIntPoint TotalResolution;

    // World units per pixel (calculated)
    UPROPERTY(VisibleAnywhere, Category = "Map Capture|Info")
    float WorldUnitsPerPixel;

    // Total memory estimate (MB)
    UPROPERTY(VisibleAnywhere, Category = "Map Capture|Info")
    float EstimatedMemoryMB;

protected:
    // Scene capture component for rendering
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;

    // Temporary render target for preview
    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> PreviewRenderTarget;

#if WITH_EDITORONLY_DATA
    // Update info display
    void UpdateCaptureInfo();

    // Setup scene capture component for a specific tile
    void SetupSceneCaptureForTile(int32 TileX, int32 TileY);

    // Capture a single tile
    UTexture2D* CaptureTile(int32 TileX, int32 TileY);

    // Create render target
    UTextureRenderTarget2D* CreateRenderTarget(FIntPoint Resolution);

    // Save render target to texture asset
    UTexture2D* SaveRenderTargetToTexture(UTextureRenderTarget2D* RenderTarget, const FString& PackageName);

    // Create map data asset
    void CreateMapDataAsset(const TArray<UTexture2D*>& Textures);

    // Get tile bounds in world space
    FBox GetTileBounds(int32 TileX, int32 TileY) const;

    // Get total volume bounds
    FBox GetVolumeBounds() const;
#endif
};
