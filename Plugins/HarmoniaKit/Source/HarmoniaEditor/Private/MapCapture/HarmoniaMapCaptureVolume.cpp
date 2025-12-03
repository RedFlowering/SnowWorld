// Copyright 2025 Snow Game Studio.

#include "MapCapture/HarmoniaMapCaptureVolume.h"

#if WITH_EDITOR
#include "Components/BrushComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "PackageTools.h"
#include "ImageUtils.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "TextureResource.h"
#include "EngineUtils.h"
#endif

AHarmoniaMapCaptureVolume::AHarmoniaMapCaptureVolume()
{
    // Note: ShapeColor was removed in UE 5.7
    // Brush color is now set via editor customization or material

    // Create scene capture component
    SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
    SceneCaptureComponent->SetupAttachment(RootComponent);
    SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
    SceneCaptureComponent->bCaptureEveryFrame = false;
    SceneCaptureComponent->bCaptureOnMovement = false;
    SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

    // Default settings
    TileResolution = FIntPoint(2048, 2048);
    TileCount = FIntPoint(1, 1);
    bCaptureSingleTexture = true;
    CaptureHeight = 10000.0f;
    bUseOrthographic = true;
    bCreateMinimap = true;
    MinimapResolution = FIntPoint(512, 512);
    bAutoCreateDataAsset = true;

#if WITH_EDITORONLY_DATA
    bShowCapturePreview = true;
    PreviewColor = FLinearColor::Green;
#endif
}

#if WITH_EDITOR

void AHarmoniaMapCaptureVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr)
        ? PropertyChangedEvent.Property->GetFName()
        : NAME_None;

    // Update info when settings change
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaMapCaptureVolume, TileResolution) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaMapCaptureVolume, TileCount) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaMapCaptureVolume, bCaptureSingleTexture))
    {
        UpdateCaptureInfo();
    }

    // Update preview color
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaMapCaptureVolume, PreviewColor))
    {
        // Note: ShapeColor was removed in UE 5.7
        // Preview color is now handled via editor customization
    }
}

void AHarmoniaMapCaptureVolume::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    if (bFinished)
    {
        UpdateCaptureInfo();
    }
}

void AHarmoniaMapCaptureVolume::CaptureMap()
{
    if (bCaptureSingleTexture)
    {
        UE_LOG(LogTemp, Log, TEXT("Capturing map as single texture..."));

        // Calculate total resolution
        FIntPoint TotalRes = TileResolution * TileCount;

        // Create render target
        UTextureRenderTarget2D* RenderTarget = CreateRenderTarget(TotalRes);
        if (!RenderTarget)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create render target!"));
            return;
        }

        // Setup scene capture
        FBox Bounds = GetVolumeBounds();
        FVector Center = Bounds.GetCenter();
        FVector CameraLocation = Center + FVector(0, 0, CaptureHeight);

        SceneCaptureComponent->SetWorldLocation(CameraLocation);
        SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

        if (bUseOrthographic)
        {
            SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
            FVector Extent = Bounds.GetExtent();
            float OrthoWidth = FMath::Max(Extent.X, Extent.Y) * 2.0f;
            SceneCaptureComponent->OrthoWidth = OrthoWidth;
        }
        else
        {
            SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Perspective;
            SceneCaptureComponent->FOVAngle = CameraFOV;
        }

        SceneCaptureComponent->TextureTarget = RenderTarget;
        SceneCaptureComponent->CaptureScene();

        // Save to texture
        FString PackageName = OutputFolder + OutputBaseName;
        UTexture2D* SavedTexture = SaveRenderTargetToTexture(RenderTarget, PackageName);

        if (SavedTexture)
        {
            UE_LOG(LogTemp, Log, TEXT("Map captured successfully: %s"), *PackageName);

            // Create minimap if requested
            if (bCreateMinimap)
            {
                UTextureRenderTarget2D* MinimapRT = CreateRenderTarget(MinimapResolution);
                if (MinimapRT)
                {
                    SceneCaptureComponent->TextureTarget = MinimapRT;
                    SceneCaptureComponent->CaptureScene();

                    FString MinimapPackageName = OutputFolder + OutputBaseName + TEXT("_Minimap");
                    SaveRenderTargetToTexture(MinimapRT, MinimapPackageName);
                }
            }

            // Create data asset
            if (bAutoCreateDataAsset)
            {
                TArray<UTexture2D*> Textures;
                Textures.Add(SavedTexture);
                CreateMapDataAsset(Textures);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save texture!"));
        }
    }
    else
    {
        CaptureMapTiles();
    }
}

void AHarmoniaMapCaptureVolume::CaptureMapTiles()
{
    UE_LOG(LogTemp, Log, TEXT("Capturing map as %d x %d tiles..."), TileCount.X, TileCount.Y);

    TArray<UTexture2D*> CapturedTiles;

    // Capture each tile
    for (int32 Y = 0; Y < TileCount.Y; ++Y)
    {
        for (int32 X = 0; X < TileCount.X; ++X)
        {
            UE_LOG(LogTemp, Log, TEXT("Capturing tile (%d, %d)..."), X, Y);

            UTexture2D* TileTexture = CaptureTile(X, Y);
            if (TileTexture)
            {
                CapturedTiles.Add(TileTexture);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to capture tile (%d, %d)"), X, Y);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Captured %d tiles successfully"), CapturedTiles.Num());

    // Create minimap from first tile or center tile
    if (bCreateMinimap && CapturedTiles.Num() > 0)
    {
        int32 CenterTileX = TileCount.X / 2;
        int32 CenterTileY = TileCount.Y / 2;

        UTextureRenderTarget2D* MinimapRT = CreateRenderTarget(MinimapResolution);
        if (MinimapRT)
        {
            SetupSceneCaptureForTile(CenterTileX, CenterTileY);
            SceneCaptureComponent->TextureTarget = MinimapRT;
            SceneCaptureComponent->CaptureScene();

            FString MinimapPackageName = OutputFolder + OutputBaseName + TEXT("_Minimap");
            SaveRenderTargetToTexture(MinimapRT, MinimapPackageName);
        }
    }

    // Create data asset
    if (bAutoCreateDataAsset && CapturedTiles.Num() > 0)
    {
        CreateMapDataAsset(CapturedTiles);
    }
}

void AHarmoniaMapCaptureVolume::PreviewCapture()
{
    UE_LOG(LogTemp, Log, TEXT("Generating preview..."));

    // Create preview render target
    FIntPoint PreviewRes = bCaptureSingleTexture
        ? FIntPoint(1024, 1024)
        : FIntPoint(512, 512);

    PreviewRenderTarget = CreateRenderTarget(PreviewRes);
    if (!PreviewRenderTarget)
    {
        return;
    }

    // Setup and capture
    FBox Bounds = GetVolumeBounds();
    FVector Center = Bounds.GetCenter();
    FVector CameraLocation = Center + FVector(0, 0, CaptureHeight);

    SceneCaptureComponent->SetWorldLocation(CameraLocation);
    SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

    if (bUseOrthographic)
    {
        SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
        FVector Extent = Bounds.GetExtent();
        float OrthoWidth = FMath::Max(Extent.X, Extent.Y) * 2.0f;
        SceneCaptureComponent->OrthoWidth = OrthoWidth;
    }

    SceneCaptureComponent->TextureTarget = PreviewRenderTarget;
    SceneCaptureComponent->CaptureScene();

    UE_LOG(LogTemp, Log, TEXT("Preview captured. Check PreviewRenderTarget property."));
}

void AHarmoniaMapCaptureVolume::CalculateOptimalTiles()
{
    FBox Bounds = GetVolumeBounds();
    FVector Extent = Bounds.GetExtent();

    // Calculate optimal tiles based on world size
    // Assume we want ~10 world units per pixel at base resolution
    float TargetUnitsPerPixel = 10.0f;
    float DesiredWidthPixels = (Extent.X * 2.0f) / TargetUnitsPerPixel;
    float DesiredHeightPixels = (Extent.Y * 2.0f) / TargetUnitsPerPixel;

    // Calculate how many tiles we need
    int32 TilesX = FMath::CeilToInt(DesiredWidthPixels / TileResolution.X);
    int32 TilesY = FMath::CeilToInt(DesiredHeightPixels / TileResolution.Y);

    TileCount = FIntPoint(FMath::Max(1, TilesX), FMath::Max(1, TilesY));

    UE_LOG(LogTemp, Log, TEXT("Calculated optimal tiles: %d x %d"), TileCount.X, TileCount.Y);

    UpdateCaptureInfo();
}

void AHarmoniaMapCaptureVolume::UpdateCaptureInfo()
{
    // Calculate total resolution
    TotalResolution = bCaptureSingleTexture
        ? (TileResolution * TileCount)
        : TileResolution;

    // Calculate world units per pixel
    FBox Bounds = GetVolumeBounds();
    FVector Extent = Bounds.GetExtent();
    float WorldWidth = Extent.X * 2.0f;

    if (TotalResolution.X > 0)
    {
        WorldUnitsPerPixel = WorldWidth / TotalResolution.X;
    }

    // Estimate memory usage
    // RGBA8 = 4 bytes per pixel
    int64 PixelCount = (int64)TotalResolution.X * (int64)TotalResolution.Y;
    int64 BytesPerTexture = PixelCount * 4;

    if (!bCaptureSingleTexture)
    {
        BytesPerTexture *= (TileCount.X * TileCount.Y);
    }

    EstimatedMemoryMB = BytesPerTexture / (1024.0f * 1024.0f);
}

void AHarmoniaMapCaptureVolume::SetupSceneCaptureForTile(int32 TileX, int32 TileY)
{
    FBox TileBounds = GetTileBounds(TileX, TileY);
    FVector Center = TileBounds.GetCenter();
    FVector CameraLocation = Center + FVector(0, 0, CaptureHeight);

    SceneCaptureComponent->SetWorldLocation(CameraLocation);
    SceneCaptureComponent->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

    if (bUseOrthographic)
    {
        SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
        FVector Extent = TileBounds.GetExtent();
        float OrthoWidth = FMath::Max(Extent.X, Extent.Y) * 2.0f;
        SceneCaptureComponent->OrthoWidth = OrthoWidth;
    }
    else
    {
        SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Perspective;
        SceneCaptureComponent->FOVAngle = CameraFOV;
    }
}

UTexture2D* AHarmoniaMapCaptureVolume::CaptureTile(int32 TileX, int32 TileY)
{
    // Create render target
    UTextureRenderTarget2D* RenderTarget = CreateRenderTarget(TileResolution);
    if (!RenderTarget)
    {
        return nullptr;
    }

    // Setup scene capture for this tile
    SetupSceneCaptureForTile(TileX, TileY);

    // Capture
    SceneCaptureComponent->TextureTarget = RenderTarget;
    SceneCaptureComponent->CaptureScene();

    // Save to texture
    FString TileName = FString::Printf(TEXT("%s_Tile_%d_%d"), *OutputBaseName, TileX, TileY);
    FString PackageName = OutputFolder + TileName;

    return SaveRenderTargetToTexture(RenderTarget, PackageName);
}

UTextureRenderTarget2D* AHarmoniaMapCaptureVolume::CreateRenderTarget(FIntPoint Resolution)
{
    UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
    if (RenderTarget)
    {
        RenderTarget->RenderTargetFormat = RTF_RGBA8;
        RenderTarget->InitAutoFormat(Resolution.X, Resolution.Y);
        RenderTarget->UpdateResourceImmediate(true);
    }
    return RenderTarget;
}

UTexture2D* AHarmoniaMapCaptureVolume::SaveRenderTargetToTexture(UTextureRenderTarget2D* RenderTarget, const FString& PackageName)
{
    if (!RenderTarget)
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

    // Read pixels from render target
    FTextureRenderTargetResource* RTResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RTResource)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get render target resource!"));
        return nullptr;
    }

    TArray<FColor> OutPixels;
    FIntRect SourceRect(0, 0, RenderTarget->SizeX, RenderTarget->SizeY);

    FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
    ReadPixelFlags.SetLinearToGamma(true);

    if (!RTResource->ReadPixels(OutPixels, ReadPixelFlags, SourceRect))
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
    NewTexture->Source.Init(RenderTarget->SizeX, RenderTarget->SizeY, 1, 1, TSF_BGRA8);

    // Copy pixel data
    uint8* MipData = NewTexture->Source.LockMip(0);
    FMemory::Memcpy(MipData, OutPixels.GetData(), OutPixels.Num() * sizeof(FColor));
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

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    bool bSaved = UPackage::SavePackage(Package, NewTexture, *PackageFileName, SaveArgs);

    if (bSaved)
    {
        UE_LOG(LogTemp, Log, TEXT("Texture saved: %s"), *PackageName);
    }

    return NewTexture;
}

void AHarmoniaMapCaptureVolume::CreateMapDataAsset(const TArray<UTexture2D*>& Textures)
{
    if (Textures.Num() == 0)
    {
        return;
    }

    FString DataAssetName = OutputBaseName + TEXT("_Data");
    FString PackagePath = OutputFolder + DataAssetName;

    // Create package
    UPackage* Package = CreatePackage(*PackagePath);
    if (!Package)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create package: %s"), *PackagePath);
        return;
    }

    // Create data asset
    UHarmoniaMapDataAsset* MapDataAsset = NewObject<UHarmoniaMapDataAsset>(
        Package,
        *DataAssetName,
        RF_Public | RF_Standalone
    );

    if (!MapDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create map data asset!"));
        return;
    }

    // Fill in data
    MapDataAsset->CapturedMapData.MapName = OutputBaseName;
    MapDataAsset->CapturedMapData.WorldBounds = GetVolumeBounds();
    MapDataAsset->CapturedMapData.MapTexture = Textures[0];
    MapDataAsset->CapturedMapData.WorldUnitsPerPixel = WorldUnitsPerPixel;

    // Try to find minimap texture
    FString MinimapPath = OutputFolder + OutputBaseName + TEXT("_Minimap");
    MapDataAsset->CapturedMapData.MinimapTexture = LoadObject<UTexture2D>(nullptr, *MinimapPath);

    // Mark package dirty and save
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(MapDataAsset);

    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    bool bSaved = UPackage::SavePackage(Package, MapDataAsset, *PackageFileName, SaveArgs);

    if (bSaved)
    {
        UE_LOG(LogTemp, Log, TEXT("Map data asset created: %s"), *PackagePath);
    }
}

FBox AHarmoniaMapCaptureVolume::GetTileBounds(int32 TileX, int32 TileY) const
{
    FBox VolumeBounds = GetVolumeBounds();
    FVector VolumeExtent = VolumeBounds.GetExtent();
    FVector VolumeMin = VolumeBounds.Min;

    // Calculate tile size in world units
    FVector TileSize;
    TileSize.X = (VolumeExtent.X * 2.0f) / TileCount.X;
    TileSize.Y = (VolumeExtent.Y * 2.0f) / TileCount.Y;
    TileSize.Z = VolumeExtent.Z * 2.0f;

    // Calculate tile min position
    FVector TileMin;
    TileMin.X = VolumeMin.X + (TileX * TileSize.X);
    TileMin.Y = VolumeMin.Y + (TileY * TileSize.Y);
    TileMin.Z = VolumeMin.Z;

    FVector TileMax = TileMin + TileSize;

    return FBox(TileMin, TileMax);
}

FBox AHarmoniaMapCaptureVolume::GetVolumeBounds() const
{
    if (GetBrushComponent())
    {
        return GetBrushComponent()->Bounds.GetBox();
    }
    return FBox(ForceInit);
}

#endif // WITH_EDITOR
