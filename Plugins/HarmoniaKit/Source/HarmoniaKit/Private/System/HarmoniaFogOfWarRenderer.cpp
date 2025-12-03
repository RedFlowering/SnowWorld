// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaFogOfWarRenderer.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/Texture.h"

UHarmoniaFogOfWarRenderer::UHarmoniaFogOfWarRenderer()
{
    FogBlendRadius = 200.0f;
    FogColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.8f);
    ExploredColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);
    TextureResolution = FIntPoint(1024, 1024);
}

void UHarmoniaFogOfWarRenderer::Initialize(const FBox& InMapBounds, FIntPoint Resolution)
{
    MapBounds = InMapBounds;
    TextureResolution = Resolution;

    CreateRenderTarget();
}

void UHarmoniaFogOfWarRenderer::UpdateFogOfWar(const TArray<FExploredRegion>& ExploredRegions)
{
    if (!FogOfWarTexture)
    {
        CreateRenderTarget();
    }

    if (!FogOfWarTexture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create fog of war texture!"));
        return;
    }

    RenderExploredRegions(ExploredRegions);
}

void UHarmoniaFogOfWarRenderer::ClearFogOfWar()
{
    if (!FogOfWarTexture)
    {
        return;
    }

    // Clear to black (fully fogged)
    UKismetRenderingLibrary::ClearRenderTarget2D(
        this,
        FogOfWarTexture,
        FogColor
    );
}

FVector2D UHarmoniaFogOfWarRenderer::WorldToTextureUV(const FVector& WorldPosition) const
{
    if (!MapBounds.IsValid)
    {
        return FVector2D::ZeroVector;
    }

    FVector LocalPos = WorldPosition - MapBounds.Min;
    FVector BoundsSize = MapBounds.GetSize();

    return FVector2D(
        BoundsSize.X > 0 ? LocalPos.X / BoundsSize.X : 0.0f,
        BoundsSize.Y > 0 ? LocalPos.Y / BoundsSize.Y : 0.0f
    );
}

float UHarmoniaFogOfWarRenderer::GetExplorationAlphaAtPosition(const FVector& WorldPosition) const
{
    // This would require reading back from the render target
    // For now, return a simple calculation
    // In production, you might want to cache this or use a more efficient method
    return 0.0f;
}

void UHarmoniaFogOfWarRenderer::CreateRenderTarget()
{
    if (!FogOfWarTexture)
    {
        FogOfWarTexture = NewObject<UTextureRenderTarget2D>(this);
        FogOfWarTexture->RenderTargetFormat = RTF_RGBA8;
        FogOfWarTexture->InitAutoFormat(TextureResolution.X, TextureResolution.Y);
        FogOfWarTexture->UpdateResourceImmediate(true);
        FogOfWarTexture->bAutoGenerateMips = false;

        // Clear to fogged initially
        ClearFogOfWar();
    }
}

void UHarmoniaFogOfWarRenderer::RenderExploredRegions(const TArray<FExploredRegion>& Regions)
{
    if (!FogOfWarTexture)
    {
        return;
    }

    // Get canvas for drawing
    UCanvas* Canvas = nullptr;
    FVector2D CanvasSize;
    FDrawToRenderTargetContext Context;

    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
        this,
        FogOfWarTexture,
        Canvas,
        CanvasSize,
        Context
    );

    if (Canvas)
    {
        // Draw each explored region
        for (const FExploredRegion& Region : Regions)
        {
            DrawExploredRegion(Canvas, Region);
        }

        // End drawing
        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
    }
}

void UHarmoniaFogOfWarRenderer::DrawExploredRegion(UCanvas* Canvas, const FExploredRegion& Region)
{
    if (!Canvas || !MapBounds.IsValid)
    {
        return;
    }

    // Convert world position to texture coordinates
    FVector2D CenterUV = WorldToTextureUV(Region.Center);

    // Convert radius to texture space
    FVector BoundsSize = MapBounds.GetSize();
    float RadiusX = (Region.Radius / BoundsSize.X) * TextureResolution.X;
    float RadiusY = (Region.Radius / BoundsSize.Y) * TextureResolution.Y;
    float AvgRadius = (RadiusX + RadiusY) * 0.5f;

    // Calculate screen position
    FVector2D ScreenPos(
        CenterUV.X * TextureResolution.X,
        CenterUV.Y * TextureResolution.Y
    );

    // Draw circle with gradient
    int32 NumSegments = FMath::Max(16, FMath::CeilToInt(AvgRadius / 4.0f));

    // Draw filled circle with gradient
    FCanvasTileItem TileItem(
        ScreenPos - FVector2D(AvgRadius, AvgRadius),
        FVector2D(AvgRadius * 2.0f, AvgRadius * 2.0f),
        ExploredColor
    );
    TileItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
    Canvas->DrawItem(TileItem);

    // Optional: Draw soft edge
    if (FogBlendRadius > 0.0f)
    {
        float BlendRadius = FMath::Min(FogBlendRadius, AvgRadius * 0.3f);

        for (int32 i = 0; i < NumSegments; ++i)
        {
            float Angle1 = (i / (float)NumSegments) * 2.0f * PI;
            float Angle2 = ((i + 1) / (float)NumSegments) * 2.0f * PI;

            FVector2D P1 = ScreenPos + FVector2D(
                FMath::Cos(Angle1) * (AvgRadius - BlendRadius),
                FMath::Sin(Angle1) * (AvgRadius - BlendRadius)
            );

            FVector2D P2 = ScreenPos + FVector2D(
                FMath::Cos(Angle2) * (AvgRadius - BlendRadius),
                FMath::Sin(Angle2) * (AvgRadius - BlendRadius)
            );

            FVector2D P3 = ScreenPos + FVector2D(
                FMath::Cos(Angle2) * AvgRadius,
                FMath::Sin(Angle2) * AvgRadius
            );

            FVector2D P4 = ScreenPos + FVector2D(
                FMath::Cos(Angle1) * AvgRadius,
                FMath::Sin(Angle1) * AvgRadius
            );

            // Draw gradient quad
            FLinearColor InnerColor = ExploredColor;
            FLinearColor OuterColor = FLinearColor(ExploredColor.R, ExploredColor.G, ExploredColor.B, 0.0f);

            FCanvasTriangleItem TriItem1(P1, P2, P3, GWhiteTexture);
            TriItem1.SetColor(InnerColor);
            TriItem1.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
            Canvas->DrawItem(TriItem1);

            FCanvasTriangleItem TriItem2(P1, P3, P4, GWhiteTexture);
            TriItem2.SetColor(InnerColor);
            TriItem2.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
            Canvas->DrawItem(TriItem2);
        }
    }
}
