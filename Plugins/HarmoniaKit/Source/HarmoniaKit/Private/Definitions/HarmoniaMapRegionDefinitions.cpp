// Copyright 2025 Snow Game Studio.

#include "Definitions/HarmoniaMapRegionDefinitions.h"

bool UHarmoniaMapRegionDataAsset::GetRegion(FName RegionID, FMapRegionData& OutRegion) const
{
    for (const FMapRegionData& Region : Regions)
    {
        if (Region.RegionID == RegionID)
        {
            OutRegion = Region;
            return true;
        }
    }
    return false;
}

bool UHarmoniaMapRegionDataAsset::GetRegionAtPosition(const FVector& WorldPosition, FMapRegionData& OutRegion) const
{
    // Find the most specific region (highest level) that contains this position
    int32 HighestLevel = -1;
    bool bFound = false;

    for (const FMapRegionData& Region : Regions)
    {
        if (Region.ContainsPoint(WorldPosition))
        {
            if (Region.RegionLevel > HighestLevel)
            {
                HighestLevel = Region.RegionLevel;
                OutRegion = Region;
                bFound = true;
            }
        }
    }

    return bFound;
}

TArray<FMapRegionData> UHarmoniaMapRegionDataAsset::GetRegionsAtPosition(const FVector& WorldPosition) const
{
    TArray<FMapRegionData> Result;

    for (const FMapRegionData& Region : Regions)
    {
        if (Region.ContainsPoint(WorldPosition))
        {
            Result.Add(Region);
        }
    }

    // Sort by level (most specific first)
    Result.Sort([](const FMapRegionData& A, const FMapRegionData& B)
    {
        return A.RegionLevel > B.RegionLevel;
    });

    return Result;
}
