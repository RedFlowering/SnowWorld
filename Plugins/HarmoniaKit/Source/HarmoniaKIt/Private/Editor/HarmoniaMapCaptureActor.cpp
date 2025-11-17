// Copyright Epic Games, Inc. All Rights Reserved.

#include "Editor/HarmoniaMapCaptureActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/BoxComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

#if WITH_EDITOR
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PackageTools.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "ImageUtils.h"
#include "TextureResource.h"
#endif

AHarmoniaMapCaptureActor::AHarmoniaMapCaptureActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create scene capture component
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->SetupAttachment(RootComponent);
	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent->OrthoWidth = 20000.0f;
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;

	// Set capture source to final color (LDR)
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	// Create bounds visualizer
	BoundsVisualizer = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundsVisualizer"));
	BoundsVisualizer->SetupAttachment(RootComponent);
	BoundsVisualizer->SetBoxExtent(FVector(10000.0f, 10000.0f, 1000.0f));
	BoundsVisualizer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoundsVisualizer->SetHiddenInGame(true);
	BoundsVisualizer->ShapeColor = FColor::Cyan;

	// Initialize default settings
	CaptureSettings.CaptureHeight = 10000.0f;
	CaptureSettings.TextureResolution = FIntPoint(2048, 2048);
	CaptureSettings.bUseOrthographic = true;
	CaptureSettings.OrthoWidth = 20000.0f;
}

void AHarmoniaMapCaptureActor::BeginPlay()
{
	Super::BeginPlay();
}

void AHarmoniaMapCaptureActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

#if WITH_EDITOR

void AHarmoniaMapCaptureActor::SetupCapture()
{
	if (!SceneCaptureComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("SceneCaptureComponent is null!"));
		return;
	}

	SetupSceneCaptureComponent();
	UpdateBoundsVisualizer();

	UE_LOG(LogTemp, Log, TEXT("Map capture setup complete. Ready to capture."));
}

void AHarmoniaMapCaptureActor::SetupSceneCaptureComponent()
{
	if (!SceneCaptureComponent)
	{
		return;
	}

	// Set projection type
	if (CaptureSettings.bUseOrthographic)
	{
		SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
		SceneCaptureComponent->OrthoWidth = CaptureSettings.OrthoWidth;
	}
	else
	{
		SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Perspective;
		SceneCaptureComponent->FOVAngle = CaptureSettings.CameraFOV;
	}

	// Position the camera above the center of the bounds
	if (CaptureSettings.CaptureBounds.IsValid)
	{
		FVector BoundsCenter = CaptureSettings.CaptureBounds.GetCenter();
		FVector CameraLocation = BoundsCenter;
		CameraLocation.Z += CaptureSettings.CaptureHeight;

		SceneCaptureComponent->SetWorldLocation(CameraLocation);
		SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f)); // Look straight down
	}
}

void AHarmoniaMapCaptureActor::CaptureMap()
{
	if (!SceneCaptureComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("SceneCaptureComponent is null!"));
		return;
	}

	// Setup capture first
	SetupCapture();

	// Create render target
	RenderTarget = CreateRenderTarget(CaptureSettings.TextureResolution);
	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create render target!"));
		return;
	}

	// Assign render target to scene capture
	SceneCaptureComponent->TextureTarget = RenderTarget;

	// Capture the scene
	SceneCaptureComponent->CaptureScene();

	// Save to texture asset
	FString PackageName = OutputTexturePath + OutputTextureName;
	UTexture2D* SavedTexture = SaveRenderTargetToTexture(RenderTarget, PackageName);

	UTexture2D* SavedMinimapTexture = nullptr;
	if (bCaptureMinimap && SavedTexture)
	{
		// Create and save minimap
		UTextureRenderTarget2D* MinimapRenderTarget = CreateRenderTarget(MinimapResolution);
		if (MinimapRenderTarget)
		{
			SceneCaptureComponent->TextureTarget = MinimapRenderTarget;
			SceneCaptureComponent->CaptureScene();

			FString MinimapPackageName = OutputTexturePath + OutputTextureName + TEXT("_Minimap");
			SavedMinimapTexture = SaveRenderTargetToTexture(MinimapRenderTarget, MinimapPackageName);
		}
	}

	if (SavedTexture)
	{
		UE_LOG(LogTemp, Log, TEXT("Map captured and saved to: %s"), *PackageName);

		// Auto-create data asset
		CreateMapDataAsset();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save map texture!"));
	}
}

void AHarmoniaMapCaptureActor::SetBoundsFromSelection()
{
	if (!GEditor)
	{
		return;
	}

	TArray<AActor*> SelectedActors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

	if (SelectedActors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No actors selected!"));
		return;
	}

	// Calculate combined bounds
	FBox CombinedBounds(ForceInit);
	for (AActor* Actor : SelectedActors)
	{
		if (Actor)
		{
			FBox ActorBounds = Actor->GetComponentsBoundingBox(true);
			CombinedBounds += ActorBounds;
		}
	}

	if (CombinedBounds.IsValid)
	{
		CaptureSettings.CaptureBounds = CombinedBounds;

		// Auto-adjust ortho width to fit bounds
		FVector BoundsSize = CombinedBounds.GetSize();
		float MaxDimension = FMath::Max(BoundsSize.X, BoundsSize.Y);
		CaptureSettings.OrthoWidth = MaxDimension * 1.1f; // 10% padding

		UpdateBoundsVisualizer();
		SetupCapture();

		UE_LOG(LogTemp, Log, TEXT("Bounds set from %d selected actors"), SelectedActors.Num());
	}
}

void AHarmoniaMapCaptureActor::AutoCalculateBounds()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FBox WorldBounds(ForceInit);

	// Iterate through all actors in the level
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor != this && !Actor->IsA<AHarmoniaMapCaptureActor>())
		{
			// Skip some actor types
			if (Actor->IsA<APlayerController>() ||
				Actor->IsA<APlayerStart>() ||
				Actor->GetName().StartsWith(TEXT("Brush_")) ||
				Actor->GetName().StartsWith(TEXT("WorldSettings")))
			{
				continue;
			}

			FBox ActorBounds = Actor->GetComponentsBoundingBox(true);
			if (ActorBounds.IsValid)
			{
				WorldBounds += ActorBounds;
			}
		}
	}

	if (WorldBounds.IsValid)
	{
		CaptureSettings.CaptureBounds = WorldBounds;

		// Auto-adjust ortho width to fit bounds
		FVector BoundsSize = WorldBounds.GetSize();
		float MaxDimension = FMath::Max(BoundsSize.X, BoundsSize.Y);
		CaptureSettings.OrthoWidth = MaxDimension * 1.1f; // 10% padding

		UpdateBoundsVisualizer();
		SetupCapture();

		UE_LOG(LogTemp, Log, TEXT("Auto-calculated world bounds: %s"), *WorldBounds.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to calculate world bounds!"));
	}
}

void AHarmoniaMapCaptureActor::PreviewCapture()
{
	SetupCapture();

	if (!RenderTarget)
	{
		RenderTarget = CreateRenderTarget(CaptureSettings.TextureResolution);
	}

	if (RenderTarget && SceneCaptureComponent)
	{
		SceneCaptureComponent->TextureTarget = RenderTarget;
		SceneCaptureComponent->CaptureScene();

		UE_LOG(LogTemp, Log, TEXT("Preview captured. Check the RenderTarget property to view."));
	}
}

void AHarmoniaMapCaptureActor::CreateMapDataAsset()
{
	if (!GEditor)
	{
		return;
	}

	FString PackagePath = OutputDataAssetPath + OutputDataAssetName;

	// Create package
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create package: %s"), *PackagePath);
		return;
	}

	// Create data asset
	UHarmoniaMapDataAsset* MapDataAsset = NewObject<UHarmoniaMapDataAsset>(Package, *OutputDataAssetName, RF_Public | RF_Standalone);
	if (!MapDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create map data asset!"));
		return;
	}

	// Fill in the captured map data
	MapDataAsset->CapturedMapData.MapName = OutputTextureName;
	MapDataAsset->CapturedMapData.WorldBounds = CaptureSettings.CaptureBounds;

	// Load the saved textures
	FString TexturePath = OutputTexturePath + OutputTextureName;
	MapDataAsset->CapturedMapData.MapTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);

	if (bCaptureMinimap)
	{
		FString MinimapPath = OutputTexturePath + OutputTextureName + TEXT("_Minimap");
		MapDataAsset->CapturedMapData.MinimapTexture = LoadObject<UTexture2D>(nullptr, *MinimapPath);
	}

	// Calculate world units per pixel
	if (CaptureSettings.CaptureBounds.IsValid && CaptureSettings.TextureResolution.X > 0)
	{
		FVector BoundsSize = CaptureSettings.CaptureBounds.GetSize();
		float MaxDimension = FMath::Max(BoundsSize.X, BoundsSize.Y);
		MapDataAsset->CapturedMapData.WorldUnitsPerPixel = MaxDimension / CaptureSettings.TextureResolution.X;
	}

	// Mark package as dirty and save
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(MapDataAsset);

	// Save the package
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, MapDataAsset, RF_Public | RF_Standalone, *PackageFileName);

	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("Map data asset created and saved: %s"), *PackagePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save map data asset!"));
	}
}

void AHarmoniaMapCaptureActor::UpdateBoundsVisualizer()
{
	if (!BoundsVisualizer || !CaptureSettings.CaptureBounds.IsValid)
	{
		return;
	}

	FVector BoundsCenter = CaptureSettings.CaptureBounds.GetCenter();
	FVector BoundsExtent = CaptureSettings.CaptureBounds.GetExtent();

	BoundsVisualizer->SetWorldLocation(BoundsCenter);
	BoundsVisualizer->SetBoxExtent(BoundsExtent);
}

UTextureRenderTarget2D* AHarmoniaMapCaptureActor::CreateRenderTarget(FIntPoint Resolution)
{
	UTextureRenderTarget2D* NewRenderTarget = NewObject<UTextureRenderTarget2D>(this);
	if (NewRenderTarget)
	{
		NewRenderTarget->RenderTargetFormat = RTF_RGBA8;
		NewRenderTarget->InitAutoFormat(Resolution.X, Resolution.Y);
		NewRenderTarget->UpdateResourceImmediate(true);
	}
	return NewRenderTarget;
}

UTexture2D* AHarmoniaMapCaptureActor::SaveRenderTargetToTexture(UTextureRenderTarget2D* InRenderTarget, const FString& PackageName)
{
	if (!InRenderTarget || !GEditor)
	{
		return nullptr;
	}

	// Create package
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create package: %s"), *PackageName);
		return nullptr;
	}

	// Get render target resource
	FTextureRenderTargetResource* RTResource = InRenderTarget->GameThread_GetRenderTargetResource();
	if (!RTResource)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get render target resource!"));
		return nullptr;
	}

	// Read pixels from render target
	TArray<FColor> OutBMP;
	FIntRect SourceRect(0, 0, InRenderTarget->SizeX, InRenderTarget->SizeY);

	FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
	ReadPixelFlags.SetLinearToGamma(true);

	if (!RTResource->ReadPixels(OutBMP, ReadPixelFlags, SourceRect))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read pixels from render target!"));
		return nullptr;
	}

	// Create texture
	FString TextureName = FPaths::GetBaseFilename(PackageName);
	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *TextureName, RF_Public | RF_Standalone);
	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create texture!"));
		return nullptr;
	}

	// Initialize texture
	NewTexture->Source.Init(InRenderTarget->SizeX, InRenderTarget->SizeY, 1, 1, TSF_BGRA8);

	// Copy pixel data
	uint8* MipData = NewTexture->Source.LockMip(0);
	FMemory::Memcpy(MipData, OutBMP.GetData(), OutBMP.Num() * sizeof(FColor));
	NewTexture->Source.UnlockMip(0);

	// Update texture settings
	NewTexture->SRGB = true;
	NewTexture->CompressionSettings = TC_Default;
	NewTexture->MipGenSettings = TMGS_FromTextureGroup;
	NewTexture->UpdateResource();

	// Mark package dirty and notify asset registry
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	// Save package
	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	UPackage::SavePackage(Package, NewTexture, RF_Public | RF_Standalone, *PackageFileName);

	return NewTexture;
}

void AHarmoniaMapCaptureActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// Update visualizer when bounds change
	if (PropertyName == GET_MEMBER_NAME_CHECKED(FMapCaptureSettings, CaptureBounds) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FMapCaptureSettings, CaptureHeight) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(FMapCaptureSettings, OrthoWidth))
	{
		UpdateBoundsVisualizer();
		SetupSceneCaptureComponent();
	}
}

void AHarmoniaMapCaptureActor::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	if (bFinished)
	{
		UpdateBoundsVisualizer();
	}
}

#endif // WITH_EDITOR
