// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaWaypointSystem.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Engine/World.h"

UHarmoniaWaypointSystem::UHarmoniaWaypointSystem()
{
    WaypointReachDistance = 200.0f;
    MaxWaypoints = 10;
}

FGuid UHarmoniaWaypointSystem::AddWaypoint(const FWaypointData& WaypointData)
{
    // Check max waypoints
    if (Waypoints.Num() >= MaxWaypoints)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum waypoints reached (%d)"), MaxWaypoints);
        return FGuid();
    }

    FWaypointData NewWaypoint = WaypointData;

    // Ensure unique ID
    if (!NewWaypoint.WaypointID.IsValid())
    {
        NewWaypoint.WaypointID = FGuid::NewGuid();
    }

    Waypoints.Add(NewWaypoint);

    OnWaypointAdded.Broadcast(NewWaypoint);

    UE_LOG(LogTemp, Log, TEXT("Waypoint added: %s at %s"),
        *NewWaypoint.WaypointName.ToString(),
        *NewWaypoint.WorldLocation.ToString());

    return NewWaypoint.WaypointID;
}

bool UHarmoniaWaypointSystem::RemoveWaypoint(const FGuid& WaypointID)
{
    int32 Index = FindWaypointIndex(WaypointID);
    if (Index != INDEX_NONE)
    {
        Waypoints.RemoveAt(Index);
        OnWaypointRemoved.Broadcast(WaypointID);

        // Clear active waypoint if it was removed
        if (ActiveWaypointID == WaypointID)
        {
            ActiveWaypointID.Invalidate();
        }

        return true;
    }

    return false;
}

bool UHarmoniaWaypointSystem::RemoveWaypointAtIndex(int32 Index)
{
    if (Waypoints.IsValidIndex(Index))
    {
        FGuid RemovedID = Waypoints[Index].WaypointID;
        Waypoints.RemoveAt(Index);
        OnWaypointRemoved.Broadcast(RemovedID);

        if (ActiveWaypointID == RemovedID)
        {
            ActiveWaypointID.Invalidate();
        }

        return true;
    }

    return false;
}

void UHarmoniaWaypointSystem::ClearAllWaypoints()
{
    for (const FWaypointData& Waypoint : Waypoints)
    {
        OnWaypointRemoved.Broadcast(Waypoint.WaypointID);
    }

    Waypoints.Empty();
    ActiveWaypointID.Invalidate();
}

bool UHarmoniaWaypointSystem::GetWaypoint(const FGuid& WaypointID, FWaypointData& OutWaypoint) const
{
    int32 Index = FindWaypointIndex(WaypointID);
    if (Index != INDEX_NONE)
    {
        OutWaypoint = Waypoints[Index];
        return true;
    }

    return false;
}

TArray<FWaypointData> UHarmoniaWaypointSystem::GetWaypointsByTag(FGameplayTag Tag) const
{
    TArray<FWaypointData> Result;

    if (!Tag.IsValid())
    {
        return Result;
    }

    for (const FWaypointData& Waypoint : Waypoints)
    {
        if (Waypoint.WaypointTag.MatchesTagExact(Tag))
        {
            Result.Add(Waypoint);
        }
    }

    return Result;
}

TArray<FWaypointData> UHarmoniaWaypointSystem::GetWaypointsByPriority(EWaypointPriority Priority) const
{
    TArray<FWaypointData> Result;

    for (const FWaypointData& Waypoint : Waypoints)
    {
        if (Waypoint.Priority == Priority)
        {
            Result.Add(Waypoint);
        }
    }

    return Result;
}

bool UHarmoniaWaypointSystem::GetNearestWaypoint(const FVector& FromLocation, FWaypointData& OutWaypoint) const
{
    if (Waypoints.Num() == 0)
    {
        return false;
    }

    float NearestDistance = FLT_MAX;
    int32 NearestIndex = INDEX_NONE;

    for (int32 i = 0; i < Waypoints.Num(); ++i)
    {
        float Distance = FVector::Dist(FromLocation, Waypoints[i].WorldLocation);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestIndex = i;
        }
    }

    if (NearestIndex != INDEX_NONE)
    {
        OutWaypoint = Waypoints[NearestIndex];
        return true;
    }

    return false;
}

bool UHarmoniaWaypointSystem::GetHighestPriorityWaypoint(FWaypointData& OutWaypoint) const
{
    if (Waypoints.Num() == 0)
    {
        return false;
    }

    int32 HighestPriority = -1;
    int32 HighestIndex = INDEX_NONE;

    for (int32 i = 0; i < Waypoints.Num(); ++i)
    {
        int32 Priority = static_cast<int32>(Waypoints[i].Priority);
        if (Priority > HighestPriority)
        {
            HighestPriority = Priority;
            HighestIndex = i;
        }
    }

    if (HighestIndex != INDEX_NONE)
    {
        OutWaypoint = Waypoints[HighestIndex];
        return true;
    }

    return false;
}

void UHarmoniaWaypointSystem::UpdateWaypointDistances(const FVector& FromLocation)
{
    for (FWaypointData& Waypoint : Waypoints)
    {
        Waypoint.Distance = FVector::Dist(FromLocation, Waypoint.WorldLocation);
    }
}

void UHarmoniaWaypointSystem::CheckWaypointsReached(const FVector& CurrentLocation)
{
    for (int32 i = Waypoints.Num() - 1; i >= 0; --i)
    {
        float Distance = FVector::Dist(CurrentLocation, Waypoints[i].WorldLocation);

        if (Distance <= WaypointReachDistance)
        {
            FGuid ReachedID = Waypoints[i].WaypointID;
            OnWaypointReached.Broadcast(ReachedID, Distance);

            // Auto-remove reached waypoint
            Waypoints.RemoveAt(i);
            OnWaypointRemoved.Broadcast(ReachedID);

            if (ActiveWaypointID == ReachedID)
            {
                ActiveWaypointID.Invalidate();
            }
        }
    }
}

TArray<FVector> UHarmoniaWaypointSystem::CalculatePathToWaypoint(
    UObject* WorldContextObject,
    const FVector& StartLocation,
    const FGuid& WaypointID) const
{
    TArray<FVector> PathPoints;

    // Find waypoint
    int32 Index = FindWaypointIndex(WaypointID);
    if (Index == INDEX_NONE)
    {
        return PathPoints;
    }

    const FWaypointData& Waypoint = Waypoints[Index];

    // Get navigation system
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return PathPoints;
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        // No navigation system, return straight line
        PathPoints.Add(StartLocation);
        PathPoints.Add(Waypoint.WorldLocation);
        return PathPoints;
    }

    // Find path
    FPathFindingQuery Query;
    Query.StartLocation = StartLocation;
    Query.EndLocation = Waypoint.WorldLocation;
    Query.NavData = NavSys->GetDefaultNavDataInstance();

    FPathFindingResult Result = NavSys->FindPathSync(Query);

    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        const TArray<FNavPathPoint>& Points = Result.Path->GetPathPoints();
        for (const FNavPathPoint& Point : Points)
        {
            PathPoints.Add(Point.Location);
        }
    }
    else
    {
        // Fallback to straight line
        PathPoints.Add(StartLocation);
        PathPoints.Add(Waypoint.WorldLocation);
    }

    return PathPoints;
}

float UHarmoniaWaypointSystem::CalculatePathLength(
    UObject* WorldContextObject,
    const FVector& StartLocation,
    const FGuid& WaypointID) const
{
    TArray<FVector> PathPoints = CalculatePathToWaypoint(WorldContextObject, StartLocation, WaypointID);

    if (PathPoints.Num() < 2)
    {
        return 0.0f;
    }

    float TotalLength = 0.0f;
    for (int32 i = 1; i < PathPoints.Num(); ++i)
    {
        TotalLength += FVector::Dist(PathPoints[i - 1], PathPoints[i]);
    }

    return TotalLength;
}

void UHarmoniaWaypointSystem::SetActiveWaypoint(const FGuid& WaypointID)
{
    if (FindWaypointIndex(WaypointID) != INDEX_NONE || !WaypointID.IsValid())
    {
        ActiveWaypointID = WaypointID;
    }
}

int32 UHarmoniaWaypointSystem::FindWaypointIndex(const FGuid& WaypointID) const
{
    for (int32 i = 0; i < Waypoints.Num(); ++i)
    {
        if (Waypoints[i].WaypointID == WaypointID)
        {
            return i;
        }
    }

    return INDEX_NONE;
}
