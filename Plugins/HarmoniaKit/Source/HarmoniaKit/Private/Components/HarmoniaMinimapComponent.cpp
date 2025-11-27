// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaMinimapComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UHarmoniaMinimapComponent::UHarmoniaMinimapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.033f; // ~30fps update for performance
}

void UHarmoniaMinimapComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeMinimap();
}

void UHarmoniaMinimapComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupMinimap();
	Super::EndPlay(EndPlayReason);
}

void UHarmoniaMinimapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInitialized)
	{
		return;
	}

	// Update scene capture position
	UpdateSceneCaptureTransform();

	// Clean up icons for destroyed actors
	CleanupInvalidIcons();

	// Update all icon positions
	UpdateIconPositions();
}

// ============================================================================
// Initialization
// ============================================================================

void UHarmoniaMinimapComponent::InitializeMinimap()
{
	if (bIsInitialized)
	{
		return;
	}

	// Create scene capture if not provided
	if (!SceneCaptureComponent)
	{
		CreateSceneCaptureComponent();
	}

	// Create render target if not provided
	if (!RenderTarget)
	{
		CreateRenderTarget();
	}

	// Configure scene capture
	if (SceneCaptureComponent && RenderTarget)
	{
		SceneCaptureComponent->TextureTarget = RenderTarget;
		SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDRNoAlpha;
		SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
		SceneCaptureComponent->OrthoWidth = Config.CaptureOrthoWidth;
		SceneCaptureComponent->bCaptureEveryFrame = Config.bCaptureEveryFrame;
		SceneCaptureComponent->bCaptureOnMovement = !Config.bCaptureEveryFrame;
		SceneCaptureComponent->bAlwaysPersistRenderingState = true;

		// Configure show flags for cleaner minimap
		ConfigureShowFlags();
	}

	bIsInitialized = true;
	OnMinimapInitialized.Broadcast();
}

void UHarmoniaMinimapComponent::CleanupMinimap()
{
	if (SceneCaptureComponent && SceneCaptureComponent->IsValidLowLevel())
	{
		SceneCaptureComponent->DestroyComponent();
		SceneCaptureComponent = nullptr;
	}

	Icons.Empty();
	bIsInitialized = false;
}

void UHarmoniaMinimapComponent::CreateSceneCaptureComponent()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(Owner, TEXT("MinimapSceneCapture"));
	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->SetupAttachment(Owner->GetRootComponent());
		SceneCaptureComponent->RegisterComponent();

		// Set initial transform (above player, looking down)
		SceneCaptureComponent->SetRelativeLocation(FVector(0.f, 0.f, Config.CaptureHeight));
		SceneCaptureComponent->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	}
}

void UHarmoniaMinimapComponent::CreateRenderTarget()
{
	RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("MinimapRenderTarget"));
	if (RenderTarget)
	{
		RenderTarget->InitCustomFormat(
			Config.TextureResolution,
			Config.TextureResolution,
			PF_B8G8R8A8,
			false
		);
		RenderTarget->UpdateResourceImmediate(true);
	}
}

void UHarmoniaMinimapComponent::ConfigureShowFlags()
{
	if (!SceneCaptureComponent)
	{
		return;
	}

	// Disable unnecessary rendering features for cleaner minimap
	SceneCaptureComponent->ShowFlags.SetFog(false);
	SceneCaptureComponent->ShowFlags.SetVolumetricFog(false);
	SceneCaptureComponent->ShowFlags.SetDynamicShadows(false);
	SceneCaptureComponent->ShowFlags.SetAmbientOcclusion(false);
	SceneCaptureComponent->ShowFlags.SetBloom(false);
	SceneCaptureComponent->ShowFlags.SetMotionBlur(false);
	SceneCaptureComponent->ShowFlags.SetDepthOfField(false);
	SceneCaptureComponent->ShowFlags.SetAntiAliasing(true);
	SceneCaptureComponent->ShowFlags.SetParticles(false);
	SceneCaptureComponent->ShowFlags.SetDecals(false);
}

// ============================================================================
// Zoom Control
// ============================================================================

void UHarmoniaMinimapComponent::SetZoomLevel(float NewZoomLevel)
{
	CurrentZoomLevel = FMath::Clamp(NewZoomLevel, Config.MinZoomLevel, Config.MaxZoomLevel);

	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->OrthoWidth = Config.CaptureOrthoWidth / CurrentZoomLevel;
	}

	OnZoomChanged.Broadcast(CurrentZoomLevel);
}

void UHarmoniaMinimapComponent::ZoomIn(float Amount)
{
	SetZoomLevel(CurrentZoomLevel + Amount);
}

void UHarmoniaMinimapComponent::ZoomOut(float Amount)
{
	SetZoomLevel(CurrentZoomLevel - Amount);
}

// ============================================================================
// Icon Management
// ============================================================================

FGuid UHarmoniaMinimapComponent::AddIcon(const FHarmoniaMinimapIcon& Icon)
{
	FHarmoniaMinimapIcon NewIcon = Icon;
	if (!NewIcon.IconID.IsValid())
	{
		NewIcon.IconID = FGuid::NewGuid();
	}

	Icons.Add(NewIcon);
	OnIconAdded.Broadcast(NewIcon);

	return NewIcon.IconID;
}

FGuid UHarmoniaMinimapComponent::AddActorIcon(AActor* Actor, UTexture2D* IconTexture, FLinearColor Color, bool bRotateWithActor)
{
	if (!Actor)
	{
		return FGuid();
	}

	FHarmoniaMinimapIcon Icon;
	Icon.TrackedActor = Actor;
	Icon.WorldLocation = Actor->GetActorLocation();
	Icon.IconTexture = IconTexture;
	Icon.IconColor = Color;
	Icon.bRotateWithActor = bRotateWithActor;

	return AddIcon(Icon);
}

FGuid UHarmoniaMinimapComponent::AddLocationIcon(FVector WorldLocation, UTexture2D* IconTexture, FLinearColor Color)
{
	FHarmoniaMinimapIcon Icon;
	Icon.WorldLocation = WorldLocation;
	Icon.IconTexture = IconTexture;
	Icon.IconColor = Color;
	Icon.bRotateWithActor = false;

	return AddIcon(Icon);
}

bool UHarmoniaMinimapComponent::RemoveIcon(FGuid IconID)
{
	for (int32 i = Icons.Num() - 1; i >= 0; --i)
	{
		if (Icons[i].IconID == IconID)
		{
			Icons.RemoveAt(i);
			OnIconRemoved.Broadcast(IconID);
			return true;
		}
	}
	return false;
}

void UHarmoniaMinimapComponent::RemoveAllIcons()
{
	for (const FHarmoniaMinimapIcon& Icon : Icons)
	{
		OnIconRemoved.Broadcast(Icon.IconID);
	}
	Icons.Empty();
}

void UHarmoniaMinimapComponent::RemoveIconsByCategory(FGameplayTag Category)
{
	for (int32 i = Icons.Num() - 1; i >= 0; --i)
	{
		if (Icons[i].CategoryTag.MatchesTag(Category))
		{
			FGuid RemovedID = Icons[i].IconID;
			Icons.RemoveAt(i);
			OnIconRemoved.Broadcast(RemovedID);
		}
	}
}

bool UHarmoniaMinimapComponent::UpdateIcon(FGuid IconID, const FHarmoniaMinimapIcon& NewData)
{
	for (FHarmoniaMinimapIcon& Icon : Icons)
	{
		if (Icon.IconID == IconID)
		{
			FGuid OriginalID = Icon.IconID;
			Icon = NewData;
			Icon.IconID = OriginalID;
			return true;
		}
	}
	return false;
}

bool UHarmoniaMinimapComponent::UpdateIconLocation(FGuid IconID, FVector NewWorldLocation)
{
	for (FHarmoniaMinimapIcon& Icon : Icons)
	{
		if (Icon.IconID == IconID)
		{
			Icon.WorldLocation = NewWorldLocation;
			Icon.TrackedActor = nullptr; // Clear tracked actor when manually setting location
			return true;
		}
	}
	return false;
}

// ============================================================================
// Icon Queries
// ============================================================================

TArray<FHarmoniaMinimapIcon> UHarmoniaMinimapComponent::GetVisibleIcons() const
{
	TArray<FHarmoniaMinimapIcon> VisibleIcons;

	for (const FHarmoniaMinimapIcon& Icon : Icons)
	{
		if (Icon.bVisible && IsCategoryVisible(Icon.CategoryTag))
		{
			VisibleIcons.Add(Icon);
		}
	}

	// Sort by priority
	VisibleIcons.Sort([](const FHarmoniaMinimapIcon& A, const FHarmoniaMinimapIcon& B)
	{
		return A.Priority > B.Priority;
	});

	return VisibleIcons;
}

bool UHarmoniaMinimapComponent::GetIconByID(FGuid IconID, FHarmoniaMinimapIcon& OutIcon) const
{
	for (const FHarmoniaMinimapIcon& Icon : Icons)
	{
		if (Icon.IconID == IconID)
		{
			OutIcon = Icon;
			return true;
		}
	}
	return false;
}

TArray<FHarmoniaMinimapIcon> UHarmoniaMinimapComponent::GetIconsByCategory(FGameplayTag Category) const
{
	TArray<FHarmoniaMinimapIcon> Result;
	for (const FHarmoniaMinimapIcon& Icon : Icons)
	{
		if (Icon.CategoryTag.MatchesTag(Category))
		{
			Result.Add(Icon);
		}
	}
	return Result;
}

// ============================================================================
// Category Visibility
// ============================================================================

void UHarmoniaMinimapComponent::SetCategoryVisible(FGameplayTag Category, bool bVisible)
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

bool UHarmoniaMinimapComponent::IsCategoryVisible(FGameplayTag Category) const
{
	if (!Category.IsValid())
	{
		return true;
	}
	return !HiddenCategories.Contains(Category);
}

// ============================================================================
// Coordinate Conversion
// ============================================================================

FVector2D UHarmoniaMinimapComponent::WorldToMinimapUV(FVector WorldLocation) const
{
	if (!SceneCaptureComponent)
	{
		return FVector2D(0.5f, 0.5f);
	}

	// Get capture center (player position)
	FVector CaptureCenter = SceneCaptureComponent->GetComponentLocation();
	CaptureCenter.Z = 0; // Flatten

	FVector TargetLocation = WorldLocation;
	TargetLocation.Z = 0; // Flatten

	// Calculate offset from capture center
	FVector Offset = TargetLocation - CaptureCenter;

	// Apply minimap rotation if needed
	if (Config.bRotateWithPlayer)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			float PlayerYaw = Owner->GetActorRotation().Yaw;
			Offset = Offset.RotateAngleAxis(-PlayerYaw, FVector::UpVector);
		}
	}

	// Get current ortho width (affected by zoom)
	float OrthoWidth = SceneCaptureComponent->OrthoWidth;
	float HalfOrtho = OrthoWidth * 0.5f;

	// Convert to UV coordinates (0-1 range, center is 0.5, 0.5)
	FVector2D UV;
	UV.X = 0.5f + (Offset.Y / OrthoWidth); // Note: Y in world = X in UV
	UV.Y = 0.5f - (Offset.X / OrthoWidth); // Note: X in world = -Y in UV (flip for correct orientation)

	return UV;
}

FVector UHarmoniaMinimapComponent::MinimapUVToWorld(FVector2D UV) const
{
	if (!SceneCaptureComponent)
	{
		return FVector::ZeroVector;
	}

	FVector CaptureCenter = SceneCaptureComponent->GetComponentLocation();
	float OrthoWidth = SceneCaptureComponent->OrthoWidth;

	// Convert UV to world offset
	FVector Offset;
	Offset.X = -(UV.Y - 0.5f) * OrthoWidth;
	Offset.Y = (UV.X - 0.5f) * OrthoWidth;
	Offset.Z = 0;

	// Apply inverse rotation if needed
	if (Config.bRotateWithPlayer)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			float PlayerYaw = Owner->GetActorRotation().Yaw;
			Offset = Offset.RotateAngleAxis(PlayerYaw, FVector::UpVector);
		}
	}

	return CaptureCenter + Offset;
}

bool UHarmoniaMinimapComponent::IsWorldLocationVisible(FVector WorldLocation) const
{
	FVector2D UV = WorldToMinimapUV(WorldLocation);
	return UV.X >= 0.0f && UV.X <= 1.0f && UV.Y >= 0.0f && UV.Y <= 1.0f;
}

FVector2D UHarmoniaMinimapComponent::ClampToMinimapEdge(FVector2D UV) const
{
	// For circular minimap, clamp to circle edge
	FVector2D Center(0.5f, 0.5f);
	FVector2D Dir = UV - Center;
	float Distance = Dir.Size();

	if (Distance > 0.45f) // Leave small border
	{
		Dir.Normalize();
		return Center + Dir * 0.45f;
	}

	return UV;
}

// ============================================================================
// Internal Updates
// ============================================================================

void UHarmoniaMinimapComponent::UpdateSceneCaptureTransform()
{
	if (!SceneCaptureComponent)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Position capture above player
	FVector PlayerLocation = Owner->GetActorLocation();
	FVector CaptureLocation = FVector(PlayerLocation.X, PlayerLocation.Y, PlayerLocation.Z + Config.CaptureHeightOffset);
	SceneCaptureComponent->SetWorldLocation(CaptureLocation);

	// Rotate with player if configured
	if (Config.bRotateWithPlayer)
	{
		float PlayerYaw = Owner->GetActorRotation().Yaw;
		SceneCaptureComponent->SetWorldRotation(FRotator(-90.f, PlayerYaw, 0.f));
	}
	else
	{
		SceneCaptureComponent->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
	}
}

void UHarmoniaMinimapComponent::UpdateIconPositions()
{
	for (FHarmoniaMinimapIcon& Icon : Icons)
	{
		// Update location for tracked actors
		if (Icon.TrackedActor.IsValid())
		{
			Icon.WorldLocation = Icon.TrackedActor->GetActorLocation();

			// Update rotation if tracking actor
			if (Icon.bRotateWithActor)
			{
				Icon.IconRotation = Icon.TrackedActor->GetActorRotation().Yaw;
			}
		}

		// Calculate minimap position
		Icon.MinimapPosition = WorldToMinimapUV(Icon.WorldLocation);
		Icon.bVisible = IsWorldLocationVisible(Icon.WorldLocation);

		// Clamp off-screen icons to edge if configured
		if (!Icon.bVisible && Icon.bClampToEdge)
		{
			Icon.MinimapPosition = ClampToMinimapEdge(Icon.MinimapPosition);
			Icon.bVisible = true; // Show at edge
		}
	}
}

void UHarmoniaMinimapComponent::CleanupInvalidIcons()
{
	for (int32 i = Icons.Num() - 1; i >= 0; --i)
	{
		// Check if tracking an actor that was destroyed
		if (Icons[i].TrackedActor.IsValid() == false && Icons[i].TrackedActor.IsExplicitlyNull() == false)
		{
			FGuid RemovedID = Icons[i].IconID;
			Icons.RemoveAt(i);
			OnIconRemoved.Broadcast(RemovedID);
		}
	}
}

void UHarmoniaMinimapComponent::CaptureScene()
{
	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->CaptureScene();
	}
}

void UHarmoniaMinimapComponent::SetConfig(const FHarmoniaMinimapConfig& NewConfig)
{
	Config = NewConfig;
	
	if (SceneCaptureComponent)
	{
		SceneCaptureComponent->OrthoWidth = Config.CaptureOrthoWidth / CurrentZoomLevel;
	}
}

void UHarmoniaMinimapComponent::SetCenterActor(AActor* Actor)
{
	CenterActor = Actor;
}

float UHarmoniaMinimapComponent::GetCurrentZoomRadius() const
{
	if (SceneCaptureComponent)
	{
		return SceneCaptureComponent->OrthoWidth * 0.5f;
	}
	return Config.CaptureOrthoWidth * 0.5f;
}

void UHarmoniaMinimapComponent::RemoveActorIcons(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}
	
	for (int32 i = Icons.Num() - 1; i >= 0; --i)
	{
		if (Icons[i].TrackedActor.Get() == Actor)
		{
			FGuid RemovedID = Icons[i].IconID;
			Icons.RemoveAt(i);
			OnIconRemoved.Broadcast(RemovedID);
		}
	}
}

void UHarmoniaMinimapComponent::SetIconVisible(FGuid IconID, bool bVisible)
{
	for (FHarmoniaMinimapIcon& Icon : Icons)
	{
		if (Icon.IconID == IconID)
		{
			Icon.bVisible = bVisible;
			return;
		}
	}
}

void UHarmoniaMinimapComponent::ToggleCategoryVisibility(FGameplayTag Category)
{
	if (HiddenCategories.Contains(Category))
	{
		HiddenCategories.Remove(Category);
	}
	else
	{
		HiddenCategories.Add(Category);
	}
}

bool UHarmoniaMinimapComponent::IsPositionInMinimapBounds(FVector WorldPosition) const
{
	return IsWorldLocationVisible(WorldPosition);
}

float UHarmoniaMinimapComponent::GetDirectionToPosition(FVector WorldPosition) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.0f;
	}
	
	FVector OwnerLocation = Owner->GetActorLocation();
	FVector Direction = WorldPosition - OwnerLocation;
	Direction.Z = 0;
	Direction.Normalize();
	
	return FMath::Atan2(Direction.Y, Direction.X) * (180.0f / PI);
}
