// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaMapCaptureBlueprintLibrary.generated.h"

class AHarmoniaMapCaptureVolume;
class UHarmoniaMapDataAsset;

/**
 * Blueprint function library for map capture utilities
 */
UCLASS()
class HARMONIAEDITOR_API UHarmoniaMapCaptureBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Create a map capture volume at specified location
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture", meta = (WorldContext = "WorldContextObject"))
    static AHarmoniaMapCaptureVolume* CreateMapCaptureVolume(
        UObject* WorldContextObject,
        FVector Location,
        FVector Extent
    );

    // Find all map capture volumes in the level
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture", meta = (WorldContext = "WorldContextObject"))
    static TArray<AHarmoniaMapCaptureVolume*> FindAllMapCaptureVolumes(UObject* WorldContextObject);

    // Capture all volumes in the level
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture", meta = (WorldContext = "WorldContextObject"))
    static void CaptureAllVolumes(UObject* WorldContextObject);

    // Get map data asset from content browser path
    UFUNCTION(BlueprintCallable, Category = "Harmonia|Map Capture")
    static UHarmoniaMapDataAsset* LoadMapDataAsset(const FString& AssetPath);

    // Calculate recommended tile count for a given world size
    UFUNCTION(BlueprintPure, Category = "Harmonia|Map Capture")
    static FIntPoint CalculateRecommendedTileCount(
        FVector WorldExtent,
        int32 TileResolution = 2048,
        float TargetWorldUnitsPerPixel = 10.0f
    );

    // Calculate memory estimate for capture settings
    UFUNCTION(BlueprintPure, Category = "Harmonia|Map Capture")
    static float CalculateMemoryEstimateMB(
        FIntPoint TileResolution,
        FIntPoint TileCount,
        bool bSingleTexture
    );
};
