// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HarmoniaMapWidget.h"
#include "UI/HarmoniaMapMarkerWidget.h"
#include "Components/HarmoniaMapComponent.h"
#include "System/HarmoniaFogOfWarRenderer.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

UHarmoniaMapWidget::UHarmoniaMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowFogOfWar = true;
	ZoomLevel = 1.0f;
	MinZoom = 0.5f;
	MaxZoom = 3.0f;
	MarkerUpdateInterval = 0.1f;
}

void UHarmoniaMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Fog of war material instance will be created in UpdateFogOfWar()
	// when the fog of war renderer is available from the map component
}

void UHarmoniaMapWidget::NativeDestruct()
{
	// Unbind events
	if (MapComponent)
	{
		MapComponent->OnLocationDiscovered.RemoveDynamic(this, &UHarmoniaMapWidget::OnLocationDiscovered);
		MapComponent->OnPingCreated.RemoveDynamic(this, &UHarmoniaMapWidget::OnPingCreated);
		MapComponent->OnRegionExplored.RemoveDynamic(this, &UHarmoniaMapWidget::OnRegionExplored);
	}

	Super::NativeDestruct();
}

void UHarmoniaMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!MapComponent)
	{
		return;
	}

	// Update markers at interval
	MarkerUpdateTimer += InDeltaTime;
	if (MarkerUpdateTimer >= MarkerUpdateInterval)
	{
		MarkerUpdateTimer = 0.0f;
		UpdateMarkers();
	}
}

void UHarmoniaMapWidget::InitializeMap(UHarmoniaMapComponent* InMapComponent)
{
	if (!InMapComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeMap called with null MapComponent!"));
		return;
	}

	MapComponent = InMapComponent;

	// Bind to events
	MapComponent->OnLocationDiscovered.AddDynamic(this, &UHarmoniaMapWidget::OnLocationDiscovered);
	MapComponent->OnPingCreated.AddDynamic(this, &UHarmoniaMapWidget::OnPingCreated);
	MapComponent->OnRegionExplored.AddDynamic(this, &UHarmoniaMapWidget::OnRegionExplored);

	// Update map display
	UpdateMap();
}

void UHarmoniaMapWidget::UpdateMap()
{
	if (!MapComponent || !MapComponent->CurrentMapData)
	{
		return;
	}

	// Set map texture
	if (MapImage && MapComponent->CurrentMapData->CapturedMapData.MapTexture)
	{
		MapImage->SetBrushFromTexture(MapComponent->CurrentMapData->CapturedMapData.MapTexture);
	}

	// Update fog of war
	if (bShowFogOfWar)
	{
		UpdateFogOfWar();
	}

	// Update all markers
	UpdateMarkers();
}

void UHarmoniaMapWidget::UpdateMarkers()
{
	if (!MapComponent)
	{
		return;
	}

	UpdatePlayerMarker();
	UpdatePingMarkers();
	UpdatePOIMarkers();
}

void UHarmoniaMapWidget::UpdateFogOfWar()
{
	if (!MapComponent || !FogOfWarImage)
	{
		return;
	}

	// Get fog of war renderer from map component
	UHarmoniaFogOfWarRenderer* FogRenderer = MapComponent->GetFogOfWarRenderer();
	if (!FogRenderer || !FogRenderer->GetFogOfWarTexture())
	{
		return;
	}

	// Create material instance if needed
	if (!FogOfWarMaterialInstance && FogRenderer->GetFogOfWarMaterial())
	{
		FogOfWarMaterialInstance = UMaterialInstanceDynamic::Create(
			FogRenderer->GetFogOfWarMaterial()->GetMaterial(),
			this
		);
	}

	// Update fog of war material with the renderer's fog texture
	if (FogOfWarMaterialInstance)
	{
		// Set fog texture parameter
		FogOfWarMaterialInstance->SetTextureParameterValue(
			TEXT("FogMask"),
			FogRenderer->GetFogOfWarTexture()
		);

		// Apply material to the fog of war image
		FogOfWarImage->SetBrushFromMaterial(FogOfWarMaterialInstance);
	}
}

void UHarmoniaMapWidget::ZoomIn(float Amount)
{
	SetZoom(ZoomLevel + Amount);
}

void UHarmoniaMapWidget::ZoomOut(float Amount)
{
	SetZoom(ZoomLevel - Amount);
}

void UHarmoniaMapWidget::SetZoom(float NewZoom)
{
	ZoomLevel = FMath::Clamp(NewZoom, MinZoom, MaxZoom);

	// Update map image scale
	if (MapImage)
	{
		MapImage->SetRenderScale(FVector2D(ZoomLevel, ZoomLevel));
	}
}

void UHarmoniaMapWidget::PanMap(FVector2D Delta)
{
	PanOffset += Delta;

	// Update map image position
	if (MapImage)
	{
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MapImage->Slot);
		if (CanvasSlot)
		{
			CanvasSlot->SetPosition(PanOffset);
		}
	}
}

void UHarmoniaMapWidget::CenterOnPlayer()
{
	if (!MapComponent)
	{
		return;
	}

	FVector PlayerPosition = MapComponent->GetPlayerWorldPosition();
	CenterOnWorldPosition(PlayerPosition);
}

void UHarmoniaMapWidget::CenterOnWorldPosition(const FVector& WorldPosition)
{
	if (!MapComponent)
	{
		return;
	}

	FVector2D MapUV = MapComponent->WorldToMapUV(WorldPosition);

	// Calculate offset to center this position
	// This depends on your widget layout
	// For now, just reset pan offset
	PanOffset = FVector2D::ZeroVector;
}

FVector UHarmoniaMapWidget::ScreenToWorld(FVector2D ScreenPosition)
{
	if (!MapComponent)
	{
		return FVector::ZeroVector;
	}

	// Convert screen position to UV (0-1)
	// This depends on your widget size and layout
	// For simplicity, assuming screen position is already in UV space
	FVector2D UV = ScreenPosition;

	return MapComponent->MapUVToWorld(UV);
}

FVector2D UHarmoniaMapWidget::WorldToScreen(const FVector& WorldPosition)
{
	if (!MapComponent)
	{
		return FVector2D::ZeroVector;
	}

	return MapComponent->WorldToMapUV(WorldPosition);
}

void UHarmoniaMapWidget::CreatePingAtScreenPosition(FVector2D ScreenPosition)
{
	if (!MapComponent)
	{
		return;
	}

	FVector WorldPosition = ScreenToWorld(ScreenPosition);
	MapComponent->CreatePing(WorldPosition);
}

UHarmoniaMapMarkerWidget* UHarmoniaMapWidget::CreateMarker(TSubclassOf<UHarmoniaMapMarkerWidget> MarkerClass)
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

void UHarmoniaMapWidget::ClearMarkers()
{
	for (UHarmoniaMapMarkerWidget* Marker : ActiveMarkers)
	{
		if (Marker)
		{
			Marker->RemoveFromParent();
		}
	}
	ActiveMarkers.Empty();
	PlayerMarker = nullptr;
}

void UHarmoniaMapWidget::UpdatePlayerMarker()
{
	if (!MapComponent)
	{
		return;
	}

	// Create player marker if it doesn't exist
	if (!PlayerMarker && PlayerMarkerClass)
	{
		PlayerMarker = CreateMarker(PlayerMarkerClass);
	}

	if (PlayerMarker)
	{
		FVector PlayerPosition = MapComponent->GetPlayerWorldPosition();
		FVector2D ScreenPos = WorldToScreen(PlayerPosition);
		float PlayerRotation = MapComponent->GetPlayerWorldRotation();

		PlayerMarker->UpdateMarker(ScreenPos, PlayerRotation);
	}
}

void UHarmoniaMapWidget::UpdatePingMarkers()
{
	if (!MapComponent)
	{
		return;
	}

	// Remove old ping markers (simplified - you'd track these better in production)
	// For now, we'll recreate them each frame
	// In production, you'd want to manage these more efficiently
}

void UHarmoniaMapWidget::UpdatePOIMarkers()
{
	if (!MapComponent || !MapComponent->CurrentMapData)
	{
		return;
	}

	// Update POI markers from discovered locations
	// Similar to ping markers, you'd want to manage these efficiently
}

void UHarmoniaMapWidget::OnLocationDiscovered(const FMapLocationData& Location)
{
	// Handle location discovery
	// Create new POI marker, play animation, etc.
	UE_LOG(LogTemp, Log, TEXT("Location discovered: %s"), *Location.LocationName.ToString());
}

void UHarmoniaMapWidget::OnPingCreated(const FMapPingData& Ping)
{
	// Handle ping creation
	// Create new ping marker with animation
	UE_LOG(LogTemp, Log, TEXT("Ping created at: %s"), *Ping.WorldPosition.ToString());

	if (PingMarkerClass)
	{
		UHarmoniaMapMarkerWidget* PingMarker = CreateMarker(PingMarkerClass);
		if (PingMarker)
		{
			FVector2D ScreenPos = WorldToScreen(Ping.WorldPosition);
			PingMarker->UpdateMarker(ScreenPos, 0.0f);
		}
	}
}

void UHarmoniaMapWidget::OnRegionExplored(const FExploredRegion& Region)
{
	// Handle region exploration
	// Update fog of war
	if (bShowFogOfWar)
	{
		UpdateFogOfWar();
	}
}
