// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaMinimapWidget.h"
#include "UI/HarmoniaMapMarkerWidget.h"
#include "Components/HarmoniaMapComponent.h"
#include "System/HarmoniaFogOfWarRenderer.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

UHarmoniaMinimapWidget::UHarmoniaMinimapWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    MinimapRange = 5000.0f;
    bRotateWithPlayer = false;
    UpdateInterval = 0.1f;
    bShowFogOfWar = true;
}

void UHarmoniaMinimapWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UHarmoniaMinimapWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UHarmoniaMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!MapComponent)
    {
        return;
    }

    // Update at interval
    UpdateTimer += InDeltaTime;
    if (UpdateTimer >= UpdateInterval)
    {
        UpdateTimer = 0.0f;
        UpdateMinimap();
    }
}

void UHarmoniaMinimapWidget::InitializeMinimap(UHarmoniaMapComponent* InMapComponent)
{
    if (!InMapComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InitializeMinimap called with null MapComponent!"));
        return;
    }

    MapComponent = InMapComponent;

    // Set minimap texture
    if (MinimapImage && MapComponent->CurrentMapData)
    {
        // Use minimap texture if available, otherwise use main map texture
        UTexture2D* MinimapTex = MapComponent->CurrentMapData->CapturedMapData.MinimapTexture
            ? MapComponent->CurrentMapData->CapturedMapData.MinimapTexture
            : MapComponent->CurrentMapData->CapturedMapData.MapTexture;

        if (MinimapTex)
        {
            MinimapImage->SetBrushFromTexture(MinimapTex);
        }
    }

    // Setup fog of war
    if (bShowFogOfWar && FogOverlay)
    {
        UpdateFogOfWar();
    }

    // Create player marker
    if (PlayerMarkerClass && !PlayerMarker)
    {
        PlayerMarker = CreateMarker(PlayerMarkerClass);
    }
}

void UHarmoniaMinimapWidget::UpdateMinimap()
{
    if (!MapComponent)
    {
        return;
    }

    UpdatePlayerMarker();
    UpdatePingMarkers();

    if (bShowFogOfWar)
    {
        UpdateFogOfWar();
    }

    // Update minimap rotation if enabled
    if (bRotateWithPlayer && MinimapImage)
    {
        float PlayerRotation = MapComponent->GetPlayerWorldRotation();
        MinimapImage->SetRenderTransformAngle(-PlayerRotation); // Negative to rotate map, not view
    }
}

void UHarmoniaMinimapWidget::SetMinimapRange(float NewRange)
{
    MinimapRange = FMath::Max(100.0f, NewRange);
}

void UHarmoniaMinimapWidget::UpdatePlayerMarker()
{
    if (!MapComponent || !PlayerMarker)
    {
        return;
    }

    // Player is always at center of minimap
    FVector2D MinimapCenter(0.5f, 0.5f);

    // Get widget size
    if (MinimapImage)
    {
        FVector2D ImageSize = MinimapImage->GetCachedGeometry().GetLocalSize();
        FVector2D ScreenPos = MinimapCenter * ImageSize;

        float PlayerRotation = MapComponent->GetPlayerWorldRotation();
        PlayerMarker->UpdateMarker(ScreenPos, PlayerRotation);
    }
}

void UHarmoniaMinimapWidget::UpdatePingMarkers()
{
    // TODO: Implement ping markers on minimap
    // Similar to main map but need to convert world positions relative to player
}

void UHarmoniaMinimapWidget::UpdateFogOfWar()
{
    if (!MapComponent || !FogOverlay)
    {
        return;
    }

    UHarmoniaFogOfWarRenderer* FogRenderer = MapComponent->GetFogOfWarRenderer();
    if (FogRenderer && FogRenderer->GetFogOfWarTexture())
    {
        if (!FogMaterialInstance && FogRenderer->GetFogOfWarMaterial())
        {
            FogMaterialInstance = UMaterialInstanceDynamic::Create(
                FogRenderer->GetFogOfWarMaterial()->GetMaterial(),
                this
            );
        }

        if (FogMaterialInstance)
        {
            // Set fog texture
            FogMaterialInstance->SetTextureParameterValue(
                TEXT("FogMask"),
                FogRenderer->GetFogOfWarTexture()
            );

            FogOverlay->SetBrushFromMaterial(FogMaterialInstance);
        }
    }
}

UHarmoniaMapMarkerWidget* UHarmoniaMinimapWidget::CreateMarker(TSubclassOf<UHarmoniaMapMarkerWidget> MarkerClass)
{
    if (!MarkerClass || !MarkerCanvas)
    {
        return nullptr;
    }

    UHarmoniaMapMarkerWidget* NewMarker = CreateWidget<UHarmoniaMapMarkerWidget>(this, MarkerClass);
    if (NewMarker)
    {
        MarkerCanvas->AddChild(NewMarker);
        ActiveMarkers.Add(NewMarker);
    }

    return NewMarker;
}
