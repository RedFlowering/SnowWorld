// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HarmoniaMapWidget.h"
#include "UI/HarmoniaMapMarkerWidget.h"
#include "Components/HarmoniaMapComponent.h"
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

	// Create fog of war material instance
	if (FogOfWarMaterial && FogOfWarImage)
	{
		FogOfWarMaterialInstance = UMaterialInstanceDynamic::Create(FogOfWarMaterial, this);
		FogOfWarImage->SetBrushFromMaterial(FogOfWarMaterialInstance);
	}
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
	if (!MapComponent || !FogOfWarMaterialInstance)
	{
		return;
	}

	// TODO: Update fog of war material parameters based on explored regions
	// This requires generating a texture mask from explored regions
	// For now, we'll just pass the explored regions count as a parameter

	int32 ExploredRegionCount = MapComponent->ExploredRegions.Num();
	FogOfWarMaterialInstance->SetScalarParameterValue(TEXT("ExploredRegions"), ExploredRegionCount);

	// You would typically:
	// 1. Create a render target
	// 2. Draw explored regions as circles/shapes
	// 3. Use that as a mask in the material
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
