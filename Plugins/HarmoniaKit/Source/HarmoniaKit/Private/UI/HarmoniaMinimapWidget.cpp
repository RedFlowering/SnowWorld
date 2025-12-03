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
    if (!MapComponent || !MarkerCanvas)
    {
        return;
    }

    // Get active pings from map component
    TArray<FMapPingData> ActivePings = MapComponent->GetActivePings();

    // Get player position for relative calculations
    FVector PlayerPosition = MapComponent->GetPlayerWorldPosition();

    // Remove expired ping markers that are no longer in ActivePings
    for (int32 i = PingMarkers.Num() - 1; i >= 0; --i)
    {
        bool bStillActive = false;
        for (const FMapPingData& Ping : ActivePings)
        {
            if (PingMarkers[i].Matches(Ping.WorldPosition, Ping.CreationTime))
            {
                bStillActive = true;
                break;
            }
        }

        if (!bStillActive)
        {
            if (PingMarkers[i].MarkerWidget)
            {
                PingMarkers[i].MarkerWidget->RemoveFromParent();
                ActiveMarkers.Remove(PingMarkers[i].MarkerWidget);
            }
            PingMarkers.RemoveAt(i);
        }
    }

    // Update or create ping markers
    for (const FMapPingData& Ping : ActivePings)
    {
        // Calculate relative position to player
        FVector RelativePosition = Ping.WorldPosition - PlayerPosition;
        float Distance = FVector2D(RelativePosition.X, RelativePosition.Y).Size();

        // Only show pings within minimap range
        if (Distance > MinimapRange)
        {
            continue;
        }

        // Find existing marker or create new one
        FMinimapPingMarker* ExistingMarker = PingMarkers.FindByPredicate(
            [&Ping](const FMinimapPingMarker& Marker)
            {
                return Marker.Matches(Ping.WorldPosition, Ping.CreationTime);
            }
        );

        UHarmoniaMapMarkerWidget* MarkerWidget = nullptr;

        if (ExistingMarker)
        {
            MarkerWidget = ExistingMarker->MarkerWidget;
        }
        else if (PingMarkerClass)
        {
            // Create new marker
            MarkerWidget = CreateMarker(PingMarkerClass);
            if (MarkerWidget)
            {
                FMinimapPingMarker NewMarker;
                NewMarker.WorldPosition = Ping.WorldPosition;
                NewMarker.CreationTime = Ping.CreationTime;
                NewMarker.MarkerWidget = MarkerWidget;
                PingMarkers.Add(NewMarker);
            }
        }

        // Update marker position
        if (MarkerWidget && MinimapImage)
        {
            // Convert relative world position to minimap screen position
            // Minimap center is player position
            FVector2D ImageSize = MinimapImage->GetCachedGeometry().GetLocalSize();
            FVector2D MinimapCenter = ImageSize * 0.5f;

            // Scale relative position to minimap range
            FVector2D RelativeUV(
                RelativePosition.X / MinimapRange,
                RelativePosition.Y / MinimapRange
            );

            // Convert to screen position (apply rotation if needed)
            FVector2D ScreenPos = MinimapCenter;
            if (bRotateWithPlayer)
            {
                // Rotate relative position by negative player rotation
                float PlayerRotation = MapComponent->GetPlayerWorldRotation();
                float Radians = FMath::DegreesToRadians(-PlayerRotation);
                float CosAngle = FMath::Cos(Radians);
                float SinAngle = FMath::Sin(Radians);

                FVector2D RotatedUV(
                    RelativeUV.X * CosAngle - RelativeUV.Y * SinAngle,
                    RelativeUV.X * SinAngle + RelativeUV.Y * CosAngle
                );

                ScreenPos += RotatedUV * MinimapCenter;
            }
            else
            {
                ScreenPos += RelativeUV * MinimapCenter;
            }

            MarkerWidget->UpdateMarker(ScreenPos, 0.0f);
        }
    }
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
