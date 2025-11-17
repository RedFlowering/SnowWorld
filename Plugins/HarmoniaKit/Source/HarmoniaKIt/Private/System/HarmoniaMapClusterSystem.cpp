// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaMapClusterSystem.h"

UHarmoniaMapClusterSystem::UHarmoniaMapClusterSystem()
{
    ClusterDistanceThreshold = 50.0f;
    MinMarkersForCluster = 2;
    MaxClusterRadius = 100.0f;
    bEnableClustering = true;

    // Default zoom thresholds
    ZoomThresholds.Add(EMapMarkerLOD::Full, 1.5f);       // > 1.5x zoom: show all
    ZoomThresholds.Add(EMapMarkerLOD::Clustered, 0.7f);  // 0.7-1.5x: cluster
    ZoomThresholds.Add(EMapMarkerLOD::IconOnly, 0.3f);   // 0.3-0.7x: icon only
    ZoomThresholds.Add(EMapMarkerLOD::Hidden, 0.0f);     // < 0.3x: hidden
}

TArray<FMarkerCluster> UHarmoniaMapClusterSystem::ClusterMarkers(
    const TArray<FMapLocationData>& Markers,
    const FBox& MapBounds,
    float CurrentZoom)
{
    TArray<FMarkerCluster> Clusters;

    if (!bEnableClustering || Markers.Num() < MinMarkersForCluster)
    {
        // No clustering, create individual clusters
        for (const FMapLocationData& Marker : Markers)
        {
            FMarkerCluster Cluster;
            FVector2D MarkerUV = WorldToNormalizedUV(Marker.WorldPosition, MapBounds);
            Cluster.AddMarker(Marker, MarkerUV);
            Clusters.Add(Cluster);
        }
        return Clusters;
    }

    // Adjust distance threshold based on zoom
    float AdjustedThreshold = ClusterDistanceThreshold / FMath::Max(0.1f, CurrentZoom);

    // Use simple distance-based clustering
    BuildClustersSimple(Markers, MapBounds, Clusters, AdjustedThreshold);

    return Clusters;
}

EMapMarkerLOD UHarmoniaMapClusterSystem::GetLODForZoom(float ZoomLevel) const
{
    if (ZoomLevel >= ZoomThresholds[EMapMarkerLOD::Full])
    {
        return EMapMarkerLOD::Full;
    }
    else if (ZoomLevel >= ZoomThresholds[EMapMarkerLOD::Clustered])
    {
        return EMapMarkerLOD::Clustered;
    }
    else if (ZoomLevel >= ZoomThresholds[EMapMarkerLOD::IconOnly])
    {
        return EMapMarkerLOD::IconOnly;
    }

    return EMapMarkerLOD::Hidden;
}

bool UHarmoniaMapClusterSystem::ShouldShowMarkerAtZoom(float ZoomLevel, EMapMarkerLOD MinLOD) const
{
    EMapMarkerLOD CurrentLOD = GetLODForZoom(ZoomLevel);
    return CurrentLOD >= MinLOD;
}

float UHarmoniaMapClusterSystem::CalculateMarkerSize(float BaseSize, float ZoomLevel) const
{
    // Scale marker size with zoom, but clamp to reasonable range
    float ScaledSize = BaseSize * ZoomLevel;
    return FMath::Clamp(ScaledSize, BaseSize * 0.5f, BaseSize * 2.0f);
}

float UHarmoniaMapClusterSystem::GetMarkerOpacity(float ZoomLevel, float FadeStart, float FadeEnd) const
{
    if (ZoomLevel >= FadeEnd)
    {
        return 1.0f;
    }
    else if (ZoomLevel <= FadeStart)
    {
        return 0.0f;
    }

    // Linear fade
    return (ZoomLevel - FadeStart) / (FadeEnd - FadeStart);
}

void UHarmoniaMapClusterSystem::BuildClustersKMeans(
    const TArray<FMapLocationData>& Markers,
    TArray<FMarkerCluster>& OutClusters,
    float DistanceThreshold)
{
    // TODO: Implement K-means clustering for better results
    // For now, use simple distance-based clustering
}

void UHarmoniaMapClusterSystem::BuildClustersSimple(
    const TArray<FMapLocationData>& Markers,
    const FBox& MapBounds,
    TArray<FMarkerCluster>& OutClusters,
    float DistanceThreshold)
{
    TArray<bool> Clustered;
    Clustered.SetNumZeroed(Markers.Num());

    for (int32 i = 0; i < Markers.Num(); ++i)
    {
        if (Clustered[i])
        {
            continue;
        }

        FMarkerCluster NewCluster;
        FVector2D MarkerUV_i = WorldToNormalizedUV(Markers[i].WorldPosition, MapBounds);

        // Add first marker
        NewCluster.AddMarker(Markers[i], MarkerUV_i);
        Clustered[i] = true;

        // Find nearby markers
        for (int32 j = i + 1; j < Markers.Num(); ++j)
        {
            if (Clustered[j])
            {
                continue;
            }

            FVector2D MarkerUV_j = WorldToNormalizedUV(Markers[j].WorldPosition, MapBounds);
            float Distance = FVector2D::Distance(MarkerUV_i, MarkerUV_j);

            // Normalize distance to screen space (assuming 1024x1024 texture)
            float ScreenDistance = Distance * 1024.0f;

            if (ScreenDistance < DistanceThreshold)
            {
                NewCluster.AddMarker(Markers[j], MarkerUV_j);
                Clustered[j] = true;
            }
        }

        // Calculate cluster radius
        for (const FMapLocationData& Marker : NewCluster.Markers)
        {
            FVector2D MarkerUV = WorldToNormalizedUV(Marker.WorldPosition, MapBounds);
            float Distance = FVector2D::Distance(NewCluster.ClusterCenter, MarkerUV) * 1024.0f;
            NewCluster.ClusterRadius = FMath::Max(NewCluster.ClusterRadius, Distance);
        }

        OutClusters.Add(NewCluster);
    }
}

FVector2D UHarmoniaMapClusterSystem::WorldToNormalizedUV(const FVector& WorldPos, const FBox& MapBounds) const
{
    if (!MapBounds.IsValid)
    {
        return FVector2D::ZeroVector;
    }

    FVector LocalPos = WorldPos - MapBounds.Min;
    FVector BoundsSize = MapBounds.GetSize();

    return FVector2D(
        BoundsSize.X > 0 ? LocalPos.X / BoundsSize.X : 0.0f,
        BoundsSize.Y > 0 ? LocalPos.Y / BoundsSize.Y : 0.0f
    );
}
