// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/HarmoniaMapMarkerWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"

UHarmoniaMapMarkerWidget::UHarmoniaMapMarkerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MarkerType = EMapMarkerType::Custom;
	MarkerColor = FLinearColor::White;
	MarkerSize = FVector2D(32.0f, 32.0f);
	bRotateWithPlayer = false;
	CurrentRotation = 0.0f;
	ScreenPosition = FVector2D::ZeroVector;
}

void UHarmoniaMapMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateVisuals();
}

void UHarmoniaMapMarkerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UHarmoniaMapMarkerWidget::UpdateMarker(FVector2D NewScreenPosition, float NewRotation)
{
	ScreenPosition = NewScreenPosition;
	CurrentRotation = NewRotation;

	// Update position
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (CanvasSlot)
	{
		CanvasSlot->SetPosition(ScreenPosition);
		CanvasSlot->SetSize(MarkerSize);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // Center anchor
	}

	// Update rotation if needed
	if (bRotateWithPlayer && MarkerIcon)
	{
		MarkerIcon->SetRenderTransformAngle(CurrentRotation);
	}
}

void UHarmoniaMapMarkerWidget::SetMarkerColor(FLinearColor NewColor)
{
	MarkerColor = NewColor;
	UpdateVisuals();
}

void UHarmoniaMapMarkerWidget::SetMarkerLabel(const FText& NewLabel)
{
	if (MarkerLabel)
	{
		MarkerLabel->SetText(NewLabel);
	}
}

void UHarmoniaMapMarkerWidget::SetMarkerIcon(UTexture2D* NewIcon)
{
	if (MarkerIcon && NewIcon)
	{
		MarkerIcon->SetBrushFromTexture(NewIcon);
	}
}

void UHarmoniaMapMarkerWidget::UpdateVisuals()
{
	// Update icon color
	if (MarkerIcon)
	{
		MarkerIcon->SetColorAndOpacity(MarkerColor);
	}

	// Update label color
	if (MarkerLabel)
	{
		MarkerLabel->SetColorAndOpacity(MarkerColor);
	}
}
