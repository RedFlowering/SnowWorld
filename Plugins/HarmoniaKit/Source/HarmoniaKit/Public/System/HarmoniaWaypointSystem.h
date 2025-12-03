// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "HarmoniaWaypointSystem.generated.h"

class ANavigationData;

/**
 * Waypoint priority level
 */
UENUM(BlueprintType)
enum class EWaypointPriority : uint8
{
    Low UMETA(DisplayName = "Low"),
    Normal UMETA(DisplayName = "Normal"),
    High UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

/**
 * Waypoint data structure
 */
USTRUCT(BlueprintType)
struct FWaypointData
{
    GENERATED_BODY()

    // Waypoint ID (unique identifier)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FGuid WaypointID;

    // World location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FVector WorldLocation;

    // Waypoint name/description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FText WaypointName;

    // Icon for this waypoint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    TObjectPtr<UTexture2D> Icon;

    // Waypoint color
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FLinearColor Color = FLinearColor::Yellow;

    // Priority
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    EWaypointPriority Priority = EWaypointPriority::Normal;

    // Waypoint type (using gameplay tags)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FGameplayTag WaypointTag;

    // Distance to waypoint (calculated)
    UPROPERTY(BlueprintReadOnly, Category = "Waypoint")
    float Distance = 0.0f;

    // Whether to show distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    bool bShowDistance = true;

    // Whether to show path to this waypoint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    bool bShowPath = true;

    // Whether to show 3D marker in world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    bool bShow3DMarker = true;

    // Custom data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FString CustomData;

    FWaypointData()
    {
        WaypointID = FGuid::NewGuid();
        WorldLocation = FVector::ZeroVector;
        WaypointName = FText::GetEmpty();
        Icon = nullptr;
        Color = FLinearColor::Yellow;
        Priority = EWaypointPriority::Normal;
        WaypointTag = FGameplayTag();
        Distance = 0.0f;
        bShowDistance = true;
        bShowPath = true;
        bShow3DMarker = true;
        CustomData = TEXT("");
    }

    FWaypointData(const FVector& InLocation, const FText& InName)
        : FWaypointData()
    {
        WorldLocation = InLocation;
        WaypointName = InName;
    }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointAdded, const FWaypointData&, Waypoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointRemoved, const FGuid&, WaypointID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaypointReached, const FGuid&, WaypointID, float, Distance);

/**
 * System for managing waypoints and path guidance
 *
 * NETWORK NOTE: This is a CLIENT-ONLY system. Do not replicate.
 * Each client manages their own waypoints locally for UI/navigation purposes.
 * If you need shared waypoints, use the map ping system instead.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaWaypointSystem : public UObject
{
    GENERATED_BODY()

public:
    UHarmoniaWaypointSystem();

    // Active waypoints
    UPROPERTY(BlueprintReadOnly, Category = "Waypoint")
    TArray<FWaypointData> Waypoints;

    // Distance threshold to consider waypoint "reached"
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    float WaypointReachDistance = 200.0f;

    // Maximum number of waypoints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    int32 MaxWaypoints = 10;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Waypoint|Events")
    FOnWaypointAdded OnWaypointAdded;

    UPROPERTY(BlueprintAssignable, Category = "Waypoint|Events")
    FOnWaypointRemoved OnWaypointRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Waypoint|Events")
    FOnWaypointReached OnWaypointReached;

    // Add a waypoint
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    FGuid AddWaypoint(const FWaypointData& WaypointData);

    // Remove a waypoint by ID
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    bool RemoveWaypoint(const FGuid& WaypointID);

    // Remove waypoint by index
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    bool RemoveWaypointAtIndex(int32 Index);

    // Clear all waypoints
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    void ClearAllWaypoints();

    // Get waypoint by ID
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    bool GetWaypoint(const FGuid& WaypointID, FWaypointData& OutWaypoint) const;

    // Get all waypoints
    UFUNCTION(BlueprintPure, Category = "Waypoint")
    TArray<FWaypointData> GetAllWaypoints() const { return Waypoints; }

    // Get waypoints by tag
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    TArray<FWaypointData> GetWaypointsByTag(FGameplayTag Tag) const;

    // Get waypoints by priority
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    TArray<FWaypointData> GetWaypointsByPriority(EWaypointPriority Priority) const;

    // Get nearest waypoint
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    bool GetNearestWaypoint(const FVector& FromLocation, FWaypointData& OutWaypoint) const;

    // Get highest priority waypoint
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    bool GetHighestPriorityWaypoint(FWaypointData& OutWaypoint) const;

    // Update waypoint distances from a location
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    void UpdateWaypointDistances(const FVector& FromLocation);

    // Check if any waypoint is reached
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    void CheckWaypointsReached(const FVector& CurrentLocation);

    // Calculate path to waypoint (returns path points)
    UFUNCTION(BlueprintCallable, Category = "Waypoint", meta = (WorldContext = "WorldContextObject"))
    TArray<FVector> CalculatePathToWaypoint(
        UObject* WorldContextObject,
        const FVector& StartLocation,
        const FGuid& WaypointID
    ) const;

    // Calculate path length to waypoint
    UFUNCTION(BlueprintCallable, Category = "Waypoint", meta = (WorldContext = "WorldContextObject"))
    float CalculatePathLength(
        UObject* WorldContextObject,
        const FVector& StartLocation,
        const FGuid& WaypointID
    ) const;

    // Set active waypoint (for UI highlighting)
    UFUNCTION(BlueprintCallable, Category = "Waypoint")
    void SetActiveWaypoint(const FGuid& WaypointID);

    // Get active waypoint
    UFUNCTION(BlueprintPure, Category = "Waypoint")
    FGuid GetActiveWaypointID() const { return ActiveWaypointID; }

protected:
    // Currently active/selected waypoint
    UPROPERTY()
    FGuid ActiveWaypointID;

    // Find waypoint index by ID
    int32 FindWaypointIndex(const FGuid& WaypointID) const;
};
