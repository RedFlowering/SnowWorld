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
    if (Markers.Num() < MinMarkersForCluster)
    {
        // Too few markers for clustering
        return;
    }

    // Estimate number of clusters based on distance threshold
    // For simplicity, use markers.Num() / MinMarkersForCluster as max K
    int32 MaxK = FMath::Max(1, Markers.Num() / MinMarkersForCluster);
    int32 K = FMath::Min(MaxK, 10); // Limit to 10 clusters max

    // Initialize K random centroids
    TArray<FVector2D> Centroids;
    TArray<int32> SelectedIndices;

    // K-means++ initialization for better initial centroids
    FRandomStream Random(FMath::Rand());

    // First centroid: random marker
    int32 FirstIndex = Random.RandRange(0, Markers.Num() - 1);
    Centroids.Add(FVector2D(Markers[FirstIndex].WorldPosition.X, Markers[FirstIndex].WorldPosition.Y));
    SelectedIndices.Add(FirstIndex);

    // Select remaining centroids using K-means++ method
    for (int32 i = 1; i < K; ++i)
    {
        // Calculate distance to nearest centroid for each marker
        TArray<float> MinDistances;
        MinDistances.SetNum(Markers.Num());

        for (int32 j = 0; j < Markers.Num(); ++j)
        {
            if (SelectedIndices.Contains(j))
            {
                MinDistances[j] = 0.0f;
                continue;
            }

            FVector2D MarkerPos(Markers[j].WorldPosition.X, Markers[j].WorldPosition.Y);
            float MinDist = TNumericLimits<float>::Max();

            for (const FVector2D& Centroid : Centroids)
            {
                float Dist = FVector2D::DistSquared(MarkerPos, Centroid);
                MinDist = FMath::Min(MinDist, Dist);
            }

            MinDistances[j] = MinDist;
        }

        // Select next centroid with probability proportional to distance^2
        float TotalWeight = 0.0f;
        for (float Dist : MinDistances)
        {
            TotalWeight += Dist;
        }

        if (TotalWeight > 0.0f)
        {
            float RandomValue = Random.FRandRange(0.0f, TotalWeight);
            float AccumulatedWeight = 0.0f;

            for (int32 j = 0; j < Markers.Num(); ++j)
            {
                AccumulatedWeight += MinDistances[j];
                if (AccumulatedWeight >= RandomValue)
                {
                    Centroids.Add(FVector2D(Markers[j].WorldPosition.X, Markers[j].WorldPosition.Y));
                    SelectedIndices.Add(j);
                    break;
                }
            }
        }
    }

    // K-means iterations
    const int32 MaxIterations = 20;
    TArray<int32> Assignments;
    Assignments.SetNum(Markers.Num());

    for (int32 Iteration = 0; Iteration < MaxIterations; ++Iteration)
    {
        bool bChanged = false;

        // Assignment step: assign each marker to nearest centroid
        for (int32 i = 0; i < Markers.Num(); ++i)
        {
            FVector2D MarkerPos(Markers[i].WorldPosition.X, Markers[i].WorldPosition.Y);

            int32 NearestCentroid = 0;
            float MinDist = FVector2D::DistSquared(MarkerPos, Centroids[0]);

            for (int32 k = 1; k < K; ++k)
            {
                float Dist = FVector2D::DistSquared(MarkerPos, Centroids[k]);
                if (Dist < MinDist)
                {
                    MinDist = Dist;
                    NearestCentroid = k;
                }
            }

            if (Assignments[i] != NearestCentroid)
            {
                Assignments[i] = NearestCentroid;
                bChanged = true;
            }
        }

        // If no changes, we've converged
        if (!bChanged)
        {
            break;
        }

        // Update step: recalculate centroids
        TArray<FVector2D> NewCentroids;
        TArray<int32> ClusterCounts;
        NewCentroids.SetNum(K);
        ClusterCounts.SetNumZeroed(K);

        for (int32 i = 0; i < Markers.Num(); ++i)
        {
            int32 ClusterIndex = Assignments[i];
            FVector2D MarkerPos(Markers[i].WorldPosition.X, Markers[i].WorldPosition.Y);

            NewCentroids[ClusterIndex] += MarkerPos;
            ClusterCounts[ClusterIndex]++;
        }

        for (int32 k = 0; k < K; ++k)
        {
            if (ClusterCounts[k] > 0)
            {
                Centroids[k] = NewCentroids[k] / static_cast<float>(ClusterCounts[k]);
            }
        }
    }

    // Build final clusters from assignments
    OutClusters.SetNum(K);
    for (int32 k = 0; k < K; ++k)
    {
        OutClusters[k] = FMarkerCluster();
        OutClusters[k].ClusterCenter = Centroids[k];
    }

    for (int32 i = 0; i < Markers.Num(); ++i)
    {
        int32 ClusterIndex = Assignments[i];
        FVector2D MarkerPos(Markers[i].WorldPosition.X, Markers[i].WorldPosition.Y);
        OutClusters[ClusterIndex].AddMarker(Markers[i], MarkerPos);
    }

    // Calculate cluster radii
    for (int32 k = 0; k < K; ++k)
    {
        for (const FMapLocationData& Marker : OutClusters[k].Markers)
        {
            FVector2D MarkerPos(Marker.WorldPosition.X, Marker.WorldPosition.Y);
            float Distance = FVector2D::Distance(OutClusters[k].ClusterCenter, MarkerPos);
            OutClusters[k].ClusterRadius = FMath::Max(OutClusters[k].ClusterRadius, Distance);
        }
    }

    // Remove empty clusters
    for (int32 k = OutClusters.Num() - 1; k >= 0; --k)
    {
        if (OutClusters[k].MarkerCount == 0)
        {
            OutClusters.RemoveAt(k);
        }
    }
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
