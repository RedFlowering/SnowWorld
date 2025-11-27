// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaWorldMarkerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/HUD.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UHarmoniaWorldMarkerComponent::UHarmoniaWorldMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.016f; // ~60fps update
}

void UHarmoniaWorldMarkerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize viewport size
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		CachedViewportSize = ViewportSize;
	}
}

void UHarmoniaWorldMarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update viewport size cache
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(CachedViewportSize);
	}

	// Clean up invalid markers (tracking destroyed actors)
	CleanupInvalidMarkers();

	// Update all marker positions
	UpdateMarkers();
}

// ============================================================================
// Marker Management
// ============================================================================

FGuid UHarmoniaWorldMarkerComponent::AddMarker(const FHarmoniaWorldMarkerData& Marker)
{
	FHarmoniaWorldMarkerData NewMarker = Marker;
	if (!NewMarker.MarkerID.IsValid())
	{
		NewMarker.MarkerID = FGuid::NewGuid();
	}

	Markers.Add(NewMarker);
	OnMarkerAdded.Broadcast(NewMarker);

	return NewMarker.MarkerID;
}

FGuid UHarmoniaWorldMarkerComponent::AddLocationMarker(FVector WorldLocation, FText Name, UTexture2D* Icon, FLinearColor Color)
{
	FHarmoniaWorldMarkerData Marker;
	Marker.WorldLocation = WorldLocation;
	Marker.DisplayName = Name;
	Marker.IconColor = Color;
	Marker.DisplayMode = EHarmoniaMarkerDisplayMode::Always;
	Marker.bShowDistance = true;

	if (Icon)
	{
		Marker.IconTexture = Icon;
	}

	return AddMarker(Marker);
}

FGuid UHarmoniaWorldMarkerComponent::AddActorMarker(AActor* Actor, FText Name, UTexture2D* Icon, FLinearColor Color)
{
	if (!Actor)
	{
		return FGuid();
	}

	FHarmoniaWorldMarkerData Marker;
	Marker.TrackedActor = Actor;
	Marker.WorldLocation = Actor->GetActorLocation();
	Marker.DisplayName = Name;
	Marker.IconColor = Color;
	Marker.DisplayMode = EHarmoniaMarkerDisplayMode::Always;
	Marker.bShowDistance = true;

	if (Icon)
	{
		Marker.IconTexture = Icon;
	}

	return AddMarker(Marker);
}

FGuid UHarmoniaWorldMarkerComponent::AddQuestMarker(FVector WorldLocation, FText ObjectiveName, bool bIsPrimary)
{
	FHarmoniaWorldMarkerData Marker;
	Marker.WorldLocation = WorldLocation;
	Marker.DisplayName = ObjectiveName;
	Marker.DisplayMode = EHarmoniaMarkerDisplayMode::Always;
	Marker.bShowDistance = true;
	Marker.bClampToScreenEdge = true;
	Marker.Priority = bIsPrimary ? 100 : 50;

	// Set quest-specific appearance
	if (bIsPrimary)
	{
		Marker.IconColor = FLinearColor(1.0f, 0.84f, 0.0f); // Gold
		Marker.MaxScale = 1.5f;
	}
	else
	{
		Marker.IconColor = FLinearColor(0.7f, 0.7f, 0.7f); // Silver
		Marker.MaxScale = 1.2f;
	}

	// Use default quest icon if available
	if (DefaultQuestIcon)
	{
		Marker.IconTexture = DefaultQuestIcon;
	}

	return AddMarker(Marker);
}

bool UHarmoniaWorldMarkerComponent::RemoveMarker(FGuid MarkerID)
{
	for (int32 i = Markers.Num() - 1; i >= 0; --i)
	{
		if (Markers[i].MarkerID == MarkerID)
		{
			Markers.RemoveAt(i);
			OnMarkerRemoved.Broadcast(MarkerID);
			return true;
		}
	}
	return false;
}

void UHarmoniaWorldMarkerComponent::RemoveMarkersByCategory(FGameplayTag Category)
{
	for (int32 i = Markers.Num() - 1; i >= 0; --i)
	{
		if (Markers[i].CategoryTag.MatchesTag(Category))
		{
			FGuid RemovedID = Markers[i].MarkerID;
			Markers.RemoveAt(i);
			OnMarkerRemoved.Broadcast(RemovedID);
		}
	}
}

void UHarmoniaWorldMarkerComponent::RemoveAllMarkers()
{
	for (const FHarmoniaWorldMarkerData& Marker : Markers)
	{
		OnMarkerRemoved.Broadcast(Marker.MarkerID);
	}
	Markers.Empty();
}

bool UHarmoniaWorldMarkerComponent::UpdateMarker(FGuid MarkerID, const FHarmoniaWorldMarkerData& NewData)
{
	for (FHarmoniaWorldMarkerData& Marker : Markers)
	{
		if (Marker.MarkerID == MarkerID)
		{
			// Preserve ID
			FGuid OriginalID = Marker.MarkerID;
			Marker = NewData;
			Marker.MarkerID = OriginalID;
			return true;
		}
	}
	return false;
}

bool UHarmoniaWorldMarkerComponent::UpdateMarkerLocation(FGuid MarkerID, FVector NewLocation)
{
	for (FHarmoniaWorldMarkerData& Marker : Markers)
	{
		if (Marker.MarkerID == MarkerID)
		{
			Marker.WorldLocation = NewLocation;
			return true;
		}
	}
	return false;
}

// ============================================================================
// Marker Queries
// ============================================================================

TArray<FHarmoniaWorldMarkerData> UHarmoniaWorldMarkerComponent::GetVisibleMarkers() const
{
	TArray<FHarmoniaWorldMarkerData> VisibleMarkers;
	for (const FHarmoniaWorldMarkerData& Marker : Markers)
	{
		if (Marker.bIsCurrentlyVisible)
		{
			VisibleMarkers.Add(Marker);
		}
	}

	// Sort by priority (higher first)
	VisibleMarkers.Sort([](const FHarmoniaWorldMarkerData& A, const FHarmoniaWorldMarkerData& B)
	{
		return A.Priority > B.Priority;
	});

	// Limit to max visible
	if (VisibleMarkers.Num() > MaxVisibleMarkers)
	{
		VisibleMarkers.SetNum(MaxVisibleMarkers);
	}

	return VisibleMarkers;
}

bool UHarmoniaWorldMarkerComponent::GetMarkerByID(FGuid MarkerID, FHarmoniaWorldMarkerData& OutMarker) const
{
	for (const FHarmoniaWorldMarkerData& Marker : Markers)
	{
		if (Marker.MarkerID == MarkerID)
		{
			OutMarker = Marker;
			return true;
		}
	}
	return false;
}

TArray<FHarmoniaWorldMarkerData> UHarmoniaWorldMarkerComponent::GetMarkersByCategory(FGameplayTag Category) const
{
	TArray<FHarmoniaWorldMarkerData> Result;
	for (const FHarmoniaWorldMarkerData& Marker : Markers)
	{
		if (Marker.CategoryTag.MatchesTag(Category))
		{
			Result.Add(Marker);
		}
	}
	return Result;
}

bool UHarmoniaWorldMarkerComponent::GetClosestMarkerToCenter(FHarmoniaWorldMarkerData& OutMarker) const
{
	FVector2D ScreenCenter = CachedViewportSize * 0.5f;
	float ClosestDistance = MAX_FLT;
	bool bFound = false;

	for (const FHarmoniaWorldMarkerData& Marker : Markers)
	{
		if (Marker.bIsCurrentlyVisible && !Marker.bIsOffScreen)
		{
			float DistanceToCenter = FVector2D::Distance(Marker.CurrentScreenPosition, ScreenCenter);
			if (DistanceToCenter < ClosestDistance)
			{
				ClosestDistance = DistanceToCenter;
				OutMarker = Marker;
				bFound = true;
			}
		}
	}

	return bFound;
}

// ============================================================================
// Category Control
// ============================================================================

void UHarmoniaWorldMarkerComponent::SetCategoryVisible(FGameplayTag Category, bool bVisible)
{
	if (bVisible)
	{
		HiddenCategories.Remove(Category);
	}
	else
	{
		HiddenCategories.AddUnique(Category);
	}
}

bool UHarmoniaWorldMarkerComponent::IsCategoryVisible(FGameplayTag Category) const
{
	return !HiddenCategories.Contains(Category);
}

// ============================================================================
// Screen Conversion
// ============================================================================

bool UHarmoniaWorldMarkerComponent::WorldToScreen(FVector WorldPosition, FVector2D& OutScreenPosition, bool& bOutIsOnScreen) const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return false;
	}

	bOutIsOnScreen = UGameplayStatics::ProjectWorldToScreen(PC, WorldPosition, OutScreenPosition, true);

	// Additional check for behind camera
	FVector CameraLoc = GetCameraLocation();
	FRotator CameraRot = GetCameraRotation();
	FVector ToMarker = WorldPosition - CameraLoc;

	// Check if marker is behind camera
	if (FVector::DotProduct(ToMarker, CameraRot.Vector()) < 0)
	{
		bOutIsOnScreen = false;
	}

	return true;
}

FVector2D UHarmoniaWorldMarkerComponent::GetClampedScreenPosition(FVector WorldPosition, float EdgePadding) const
{
	FVector2D ScreenPos;
	bool bOnScreen;
	WorldToScreen(WorldPosition, ScreenPos, bOnScreen);

	if (bOnScreen)
	{
		return ScreenPos;
	}

	// Calculate clamped position at screen edge
	FVector2D ScreenCenter = CachedViewportSize * 0.5f;
	FVector2D Direction = ScreenPos - ScreenCenter;

	if (Direction.IsNearlyZero())
	{
		return ScreenCenter;
	}

	Direction.Normalize();

	// Calculate intersection with screen edge (with padding)
	float HalfWidth = (CachedViewportSize.X * 0.5f) - EdgePadding;
	float HalfHeight = (CachedViewportSize.Y * 0.5f) - EdgePadding;

	float ScaleX = FMath::Abs(Direction.X) > SMALL_NUMBER ? HalfWidth / FMath::Abs(Direction.X) : MAX_FLT;
	float ScaleY = FMath::Abs(Direction.Y) > SMALL_NUMBER ? HalfHeight / FMath::Abs(Direction.Y) : MAX_FLT;

	float Scale = FMath::Min(ScaleX, ScaleY);

	return ScreenCenter + Direction * Scale;
}

float UHarmoniaWorldMarkerComponent::GetOffScreenIndicatorAngle(FVector WorldPosition) const
{
	FVector2D ScreenPos;
	bool bOnScreen;
	WorldToScreen(WorldPosition, ScreenPos, bOnScreen);

	FVector2D ScreenCenter = CachedViewportSize * 0.5f;
	FVector2D Direction = ScreenPos - ScreenCenter;

	if (Direction.IsNearlyZero())
	{
		return 0.0f;
	}

	// Calculate angle from up direction (negative Y is up in screen space)
	float Angle = FMath::Atan2(Direction.X, -Direction.Y);
	return FMath::RadiansToDegrees(Angle);
}

// ============================================================================
// Distance Formatting
// ============================================================================

FText UHarmoniaWorldMarkerComponent::FormatDistance(float DistanceInCM) const
{
	float DistanceInMeters = DistanceInCM / 100.0f;

	if (bUseMetricUnits)
	{
		if (DistanceInMeters >= 1000.0f)
		{
			return FText::FromString(FString::Printf(TEXT("%.1f km"), DistanceInMeters / 1000.0f));
		}
		else
		{
			return FText::FromString(FString::Printf(TEXT("%.0f m"), DistanceInMeters));
		}
	}
	else
	{
		// Convert to feet
		float DistanceInFeet = DistanceInMeters * 3.28084f;
		if (DistanceInFeet >= 5280.0f)
		{
			return FText::FromString(FString::Printf(TEXT("%.1f mi"), DistanceInFeet / 5280.0f));
		}
		else
		{
			return FText::FromString(FString::Printf(TEXT("%.0f ft"), DistanceInFeet));
		}
	}
}

// ============================================================================
// Internal Updates
// ============================================================================

void UHarmoniaWorldMarkerComponent::UpdateMarkers()
{
	FVector CameraLocation = GetCameraLocation();

	for (FHarmoniaWorldMarkerData& Marker : Markers)
	{
		// Update location for tracked actors
		if (Marker.TrackedActor.IsValid())
		{
			Marker.WorldLocation = Marker.TrackedActor->GetActorLocation();
		}

		// Calculate distance
		Marker.CurrentDistance = FVector::Dist(CameraLocation, Marker.WorldLocation);

		// Check category visibility
		if (Marker.CategoryTag.IsValid() && !IsCategoryVisible(Marker.CategoryTag))
		{
			Marker.bIsCurrentlyVisible = false;
			continue;
		}

		// Check display mode visibility
		Marker.bIsCurrentlyVisible = ShouldMarkerBeVisible(Marker, Marker.CurrentDistance);

		if (Marker.bIsCurrentlyVisible)
		{
			// Update screen position
			bool bOnScreen;
			WorldToScreen(Marker.WorldLocation, Marker.CurrentScreenPosition, bOnScreen);
			Marker.bIsOffScreen = !bOnScreen;

			// Clamp to screen edge if needed
			if (Marker.bIsOffScreen && Marker.bClampToScreenEdge)
			{
				Marker.CurrentScreenPosition = GetClampedScreenPosition(Marker.WorldLocation, Marker.ScreenEdgePadding);
			}
		}
	}
}

bool UHarmoniaWorldMarkerComponent::ShouldMarkerBeVisible(const FHarmoniaWorldMarkerData& Marker, float Distance) const
{
	// Check distance limits
	if (Marker.MaxDisplayDistance > 0.0f && Distance > Marker.MaxDisplayDistance)
	{
		return false;
	}
	if (Distance < Marker.MinDisplayDistance)
	{
		return false;
	}

	switch (Marker.DisplayMode)
	{
	case EHarmoniaMarkerDisplayMode::Always:
		return true;

	case EHarmoniaMarkerDisplayMode::WhenInRange:
		// Already checked above with MaxDisplayDistance
		return true;

	case EHarmoniaMarkerDisplayMode::WhenLooking:
		{
			FVector CameraLoc = GetCameraLocation();
			FRotator CameraRot = GetCameraRotation();
			FVector ToMarker = (Marker.WorldLocation - CameraLoc).GetSafeNormal();
			float Dot = FVector::DotProduct(ToMarker, CameraRot.Vector());
			return Dot > 0.7f; // ~45 degree cone
		}

	case EHarmoniaMarkerDisplayMode::WhenTracked:
		// This requires external tracking state - always show for now
		return true;

	case EHarmoniaMarkerDisplayMode::WhenDiscovered:
		// This requires discovery state - always show for now
		return true;

	default:
		return true;
	}
}

float UHarmoniaWorldMarkerComponent::CalculateMarkerScale(const FHarmoniaWorldMarkerData& Marker, float Distance) const
{
	if (!Marker.bScaleWithDistance)
	{
		return 1.0f;
	}

	// Scale inversely with distance (closer = bigger)
	// Use reference distance of 1000cm (10m)
	const float ReferenceDistance = 1000.0f;
	float ScaleFactor = ReferenceDistance / FMath::Max(Distance, 1.0f);

	return FMath::Clamp(ScaleFactor, Marker.MinScale, Marker.MaxScale);
}

float UHarmoniaWorldMarkerComponent::CalculateMarkerOpacity(const FHarmoniaWorldMarkerData& Marker, float Distance) const
{
	if (!Marker.bFadeWithDistance || Marker.MaxDisplayDistance <= 0.0f)
	{
		return 1.0f;
	}

	// Start fading at 80% of max distance
	float FadeStart = Marker.MaxDisplayDistance * 0.8f;
	if (Distance < FadeStart)
	{
		return 1.0f;
	}

	float FadeRange = Marker.MaxDisplayDistance - FadeStart;
	float FadeProgress = (Distance - FadeStart) / FadeRange;

	return FMath::Clamp(1.0f - FadeProgress, 0.0f, 1.0f);
}

void UHarmoniaWorldMarkerComponent::CleanupInvalidMarkers()
{
	for (int32 i = Markers.Num() - 1; i >= 0; --i)
	{
		// Remove markers tracking destroyed actors
		if (Markers[i].TrackedActor.IsValid() == false && Markers[i].TrackedActor.IsExplicitlyNull() == false)
		{
			FGuid RemovedID = Markers[i].MarkerID;
			Markers.RemoveAt(i);
			OnMarkerRemoved.Broadcast(RemovedID);
		}
	}
}

FVector UHarmoniaWorldMarkerComponent::GetCameraLocation() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerCameraManager)
	{
		return PC->PlayerCameraManager->GetCameraLocation();
	}
	return FVector::ZeroVector;
}

FRotator UHarmoniaWorldMarkerComponent::GetCameraRotation() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerCameraManager)
	{
		return PC->PlayerCameraManager->GetCameraRotation();
	}
	return FRotator::ZeroRotator;
}
