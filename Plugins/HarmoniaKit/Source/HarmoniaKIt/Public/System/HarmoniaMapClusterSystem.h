// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Definitions/HarmoniaMapSystemDefinitions.h"
#include "HarmoniaMapClusterSystem.generated.h"

/**
 * Marker cluster data
 */
USTRUCT(BlueprintType)
struct FMarkerCluster
{
    GENERATED_BODY()

    // Cluster center position (UV space)
    UPROPERTY(BlueprintReadOnly, Category = "Cluster")
    FVector2D ClusterCenter;

    // Number of markers in this cluster
    UPROPERTY(BlueprintReadOnly, Category = "Cluster")
    int32 MarkerCount = 0;

    // Markers in this cluster
    UPROPERTY(BlueprintReadOnly, Category = "Cluster")
    TArray<FMapLocationData> Markers;

    // Cluster radius (in screen space)
    UPROPERTY(BlueprintReadOnly, Category = "Cluster")
    float ClusterRadius = 0.0f;

    // Cluster color (average of markers)
    UPROPERTY(BlueprintReadOnly, Category = "Cluster")
    FLinearColor ClusterColor = FLinearColor::White;

    FMarkerCluster()
    {
        ClusterCenter = FVector2D::ZeroVector;
        MarkerCount = 0;
        ClusterRadius = 0.0f;
        ClusterColor = FLinearColor::White;
    }

    // Add marker to cluster
    void AddMarker(const FMapLocationData& Marker, const FVector2D& MarkerUV)
    {
        Markers.Add(Marker);
        MarkerCount++;

        // Recalculate center
        if (MarkerCount == 1)
        {
            ClusterCenter = MarkerUV;
        }
        else
        {
            ClusterCenter = (ClusterCenter * (MarkerCount - 1) + MarkerUV) / MarkerCount;
        }
    }
};

/**
 * LOD level for marker display
 */
UENUM(BlueprintType)
enum class EMapMarkerLOD : uint8
{
    Full UMETA(DisplayName = "Full Detail"),      // Show all markers individually
    Clustered UMETA(DisplayName = "Clustered"),   // Group nearby markers
    IconOnly UMETA(DisplayName = "Icon Only"),    // Show only icon, no text
    Hidden UMETA(DisplayName = "Hidden")          // Don't show markers
};

/**
 * System for clustering markers and LOD management
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaMapClusterSystem : public UObject
{
    GENERATED_BODY()

public:
    UHarmoniaMapClusterSystem();

    // Cluster distance threshold (screen pixels)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    float ClusterDistanceThreshold = 50.0f;

    // Minimum markers to form a cluster
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    int32 MinMarkersForCluster = 2;

    // Maximum cluster radius before subdividing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    float MaxClusterRadius = 100.0f;

    // Zoom levels for LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TMap<EMapMarkerLOD, float> ZoomThresholds;

    // Whether clustering is enabled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clustering")
    bool bEnableClustering = true;

    // Cluster markers from a list
    UFUNCTION(BlueprintCallable, Category = "Clustering")
    TArray<FMarkerCluster> ClusterMarkers(
        const TArray<FMapLocationData>& Markers,
        const FBox& MapBounds,
        float CurrentZoom
    );

    // Get LOD level for current zoom
    UFUNCTION(BlueprintPure, Category = "LOD")
    EMapMarkerLOD GetLODForZoom(float ZoomLevel) const;

    // Should marker be visible at zoom level
    UFUNCTION(BlueprintPure, Category = "LOD")
    bool ShouldShowMarkerAtZoom(float ZoomLevel, EMapMarkerLOD MinLOD = EMapMarkerLOD::Full) const;

    // Calculate marker screen size based on zoom
    UFUNCTION(BlueprintPure, Category = "LOD")
    float CalculateMarkerSize(float BaseSize, float ZoomLevel) const;

    // Get marker opacity based on zoom
    UFUNCTION(BlueprintPure, Category = "LOD")
    float GetMarkerOpacity(float ZoomLevel, float FadeStart, float FadeEnd) const;

protected:
    // Build clusters using spatial hashing
    void BuildClustersKMeans(
        const TArray<FMapLocationData>& Markers,
        TArray<FMarkerCluster>& OutClusters,
        float DistanceThreshold
    );

    // Simple distance-based clustering
    void BuildClustersSimple(
        const TArray<FMapLocationData>& Markers,
        const FBox& MapBounds,
        TArray<FMarkerCluster>& OutClusters,
        float DistanceThreshold
    );

    // Convert world position to normalized UV
    FVector2D WorldToNormalizedUV(const FVector& WorldPos, const FBox& MapBounds) const;
};
