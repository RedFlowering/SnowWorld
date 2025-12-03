// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HarmoniaFogOfWarRenderer.generated.h"

struct FExploredRegion;
class UCanvas;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * Renders fog of war mask texture from explored regions
 * Creates a dynamic texture that can be used in materials to hide unexplored areas
 *
 * NETWORK NOTE: This is a CLIENT-ONLY system. Do not replicate.
 * Each client creates its own renderer based on replicated ExploredRegions data.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaFogOfWarRenderer : public UObject
{
    GENERATED_BODY()

public:
    UHarmoniaFogOfWarRenderer();

    // Initialize the renderer with map bounds and resolution
    UFUNCTION(BlueprintCallable, Category = "Fog of War")
    void Initialize(const FBox& MapBounds, FIntPoint Resolution);

    // Update fog of war with explored regions
    UFUNCTION(BlueprintCallable, Category = "Fog of War")
    void UpdateFogOfWar(const TArray<FExploredRegion>& ExploredRegions);

    // Clear all explored regions (reset fog of war)
    UFUNCTION(BlueprintCallable, Category = "Fog of War")
    void ClearFogOfWar();

    // Get the fog of war mask texture
    UFUNCTION(BlueprintPure, Category = "Fog of War")
    UTextureRenderTarget2D* GetFogOfWarTexture() const { return FogOfWarTexture; }

    // Get fog of war material instance (for UI)
    UFUNCTION(BlueprintPure, Category = "Fog of War")
    UMaterialInstanceDynamic* GetFogOfWarMaterial() const { return FogOfWarMaterial; }

    // Convert world position to texture UV
    UFUNCTION(BlueprintPure, Category = "Fog of War")
    FVector2D WorldToTextureUV(const FVector& WorldPosition) const;

    // Get exploration alpha at world position (0 = unexplored, 1 = explored)
    UFUNCTION(BlueprintCallable, Category = "Fog of War")
    float GetExplorationAlphaAtPosition(const FVector& WorldPosition) const;

    // Fog of war blend mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog of War|Settings")
    float FogBlendRadius = 200.0f; // Pixels for smooth blend

    // Fog color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog of War|Settings")
    FLinearColor FogColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.8f);

    // Explored area color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog of War|Settings")
    FLinearColor ExploredColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);

protected:
    // Render target for fog of war
    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> FogOfWarTexture;

    // Material for rendering fog of war
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> FogOfWarMaterial;

    // Map bounds
    UPROPERTY()
    FBox MapBounds;

    // Texture resolution
    UPROPERTY()
    FIntPoint TextureResolution;

    // Create render target
    void CreateRenderTarget();

    // Render explored regions to texture
    void RenderExploredRegions(const TArray<FExploredRegion>& Regions);

    // Draw a single explored region
    void DrawExploredRegion(UCanvas* Canvas, const FExploredRegion& Region);
};
