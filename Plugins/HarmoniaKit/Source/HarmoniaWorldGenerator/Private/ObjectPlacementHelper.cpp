// Copyright 2025 Snow Game Studio.

#include "ObjectPlacementHelper.h"

bool ObjectPlacementHelper::IsPlacementValid(const FVector& Location, const TArray<FWorldObjectData>& ExistingObjects, float MinDistance)
{
    for (const FWorldObjectData& Obj : ExistingObjects)
    {
        if (FVector::Dist2D(Location, Obj.Location) < MinDistance)
        {
            return false;
        }
    }
    return true;
}

float ObjectPlacementHelper::GetMinDistanceToObjects(const FVector& Location, const TArray<FWorldObjectData>& ExistingObjects)
{
    float MinDist = TNumericLimits<float>::Max();

    for (const FWorldObjectData& Obj : ExistingObjects)
    {
        float Dist = FVector::Dist2D(Location, Obj.Location);
        if (Dist < MinDist)
        {
            MinDist = Dist;
        }
    }
    return MinDist;
}