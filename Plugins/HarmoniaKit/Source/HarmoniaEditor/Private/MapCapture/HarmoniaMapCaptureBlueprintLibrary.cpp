// Copyright 2025 Snow Game Studio.

#include "MapCapture/HarmoniaMapCaptureBlueprintLibrary.h"
#include "MapCapture/HarmoniaMapCaptureVolume.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Volume.h"
#include "Components/BrushComponent.h"

AHarmoniaMapCaptureVolume* UHarmoniaMapCaptureBlueprintLibrary::CreateMapCaptureVolume(
    UObject* WorldContextObject,
    FVector Location,
    FVector Extent)
{
#if WITH_EDITOR
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AHarmoniaMapCaptureVolume* Volume = World->SpawnActor<AHarmoniaMapCaptureVolume>(
        AHarmoniaMapCaptureVolume::StaticClass(),
        Location,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (Volume && Volume->GetBrushComponent())
    {
        // Set brush size
        Volume->GetBrushComponent()->Bounds = FBoxSphereBounds(Location, Extent, Extent.Size());
    }

    return Volume;
#else
    return nullptr;
#endif
}

TArray<AHarmoniaMapCaptureVolume*> UHarmoniaMapCaptureBlueprintLibrary::FindAllMapCaptureVolumes(UObject* WorldContextObject)
{
    TArray<AHarmoniaMapCaptureVolume*> Volumes;

#if WITH_EDITOR
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return Volumes;
    }

    for (TActorIterator<AHarmoniaMapCaptureVolume> It(World); It; ++It)
    {
        Volumes.Add(*It);
    }
#endif

    return Volumes;
}

void UHarmoniaMapCaptureBlueprintLibrary::CaptureAllVolumes(UObject* WorldContextObject)
{
#if WITH_EDITOR
    TArray<AHarmoniaMapCaptureVolume*> Volumes = FindAllMapCaptureVolumes(WorldContextObject);

    UE_LOG(LogTemp, Log, TEXT("Capturing %d map volumes..."), Volumes.Num());

    for (AHarmoniaMapCaptureVolume* Volume : Volumes)
    {
        if (Volume)
        {
            UE_LOG(LogTemp, Log, TEXT("Capturing volume: %s"), *Volume->GetName());
            Volume->CaptureMap();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("All volumes captured."));
#endif
}

UHarmoniaMapDataAsset* UHarmoniaMapCaptureBlueprintLibrary::LoadMapDataAsset(const FString& AssetPath)
{
    return LoadObject<UHarmoniaMapDataAsset>(nullptr, *AssetPath);
}

FIntPoint UHarmoniaMapCaptureBlueprintLibrary::CalculateRecommendedTileCount(
    FVector WorldExtent,
    int32 TileResolution,
    float TargetWorldUnitsPerPixel)
{
    if (TileResolution <= 0 || TargetWorldUnitsPerPixel <= 0.0f)
    {
        return FIntPoint(1, 1);
    }

    // Calculate desired resolution based on world size
    float WorldWidth = WorldExtent.X * 2.0f;
    float WorldHeight = WorldExtent.Y * 2.0f;

    float DesiredWidthPixels = WorldWidth / TargetWorldUnitsPerPixel;
    float DesiredHeightPixels = WorldHeight / TargetWorldUnitsPerPixel;

    // Calculate tile count
    int32 TilesX = FMath::CeilToInt(DesiredWidthPixels / TileResolution);
    int32 TilesY = FMath::CeilToInt(DesiredHeightPixels / TileResolution);

    return FIntPoint(FMath::Max(1, TilesX), FMath::Max(1, TilesY));
}

float UHarmoniaMapCaptureBlueprintLibrary::CalculateMemoryEstimateMB(
    FIntPoint TileResolution,
    FIntPoint TileCount,
    bool bSingleTexture)
{
    int64 PixelsPerTile = (int64)TileResolution.X * (int64)TileResolution.Y;
    int64 TotalPixels;

    if (bSingleTexture)
    {
        // Single texture with combined resolution
        int64 TotalWidth = TileResolution.X * TileCount.X;
        int64 TotalHeight = TileResolution.Y * TileCount.Y;
        TotalPixels = TotalWidth * TotalHeight;
    }
    else
    {
        // Multiple tiles
        int64 NumTiles = TileCount.X * TileCount.Y;
        TotalPixels = PixelsPerTile * NumTiles;
    }

    // RGBA8 = 4 bytes per pixel
    int64 TotalBytes = TotalPixels * 4;
    float TotalMB = TotalBytes / (1024.0f * 1024.0f);

    return TotalMB;
}
