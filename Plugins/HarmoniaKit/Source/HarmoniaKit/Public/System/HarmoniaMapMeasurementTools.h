// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HarmoniaMapMeasurementTools.generated.h"

/**
 * Measurement type
 */
UENUM(BlueprintType)
enum class EMapMeasurementType : uint8
{
    Distance UMETA(DisplayName = "Distance"),          // Straight line distance
    PathDistance UMETA(DisplayName = "Path Distance"), // Navigation path distance
    Area UMETA(DisplayName = "Area"),                  // Polygon area
    Radius UMETA(DisplayName = "Radius")               // Circle radius
};

/**
 * Measurement data
 */
USTRUCT(BlueprintType)
struct FMapMeasurement
{
    GENERATED_BODY()

    // Measurement type
    UPROPERTY(BlueprintReadWrite, Category = "Measurement")
    EMapMeasurementType MeasurementType = EMapMeasurementType::Distance;

    // Measurement points (world space)
    UPROPERTY(BlueprintReadWrite, Category = "Measurement")
    TArray<FVector> Points;

    // Measured value (distance in cm, area in cm²)
    UPROPERTY(BlueprintReadOnly, Category = "Measurement")
    float MeasuredValue = 0.0f;

    // Measurement label
    UPROPERTY(BlueprintReadWrite, Category = "Measurement")
    FText Label;

    // Whether to show this measurement
    UPROPERTY(BlueprintReadWrite, Category = "Measurement")
    bool bVisible = true;

    // Color for drawing measurement
    UPROPERTY(BlueprintReadWrite, Category = "Measurement")
    FLinearColor Color = FLinearColor::Yellow;

    FMapMeasurement()
    {
        MeasurementType = EMapMeasurementType::Distance;
        MeasuredValue = 0.0f;
        Label = FText::GetEmpty();
        bVisible = true;
        Color = FLinearColor::Yellow;
    }
};

/**
 * Tools for measuring distances and areas on the map
 *
 * NETWORK NOTE: This is a CLIENT-ONLY system. Do not replicate.
 * Each client manages their own measurements locally for their map UI.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaMapMeasurementTools : public UObject
{
    GENERATED_BODY()

public:
    UHarmoniaMapMeasurementTools();

    // Active measurements
    UPROPERTY(BlueprintReadOnly, Category = "Measurement")
    TArray<FMapMeasurement> Measurements;

    // Current measurement being created
    UPROPERTY(BlueprintReadOnly, Category = "Measurement")
    FMapMeasurement CurrentMeasurement;

    // Whether currently measuring
    UPROPERTY(BlueprintReadOnly, Category = "Measurement")
    bool bIsMeasuring = false;

    // Unit display (meters, km, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Measurement")
    FString DistanceUnit = TEXT("m");

    // Conversion factor from cm to display unit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Measurement")
    float UnitConversionFactor = 0.01f; // cm to meters

    // Decimal places for display
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Measurement")
    int32 DecimalPlaces = 2;

    // Start a new measurement
    UFUNCTION(BlueprintCallable, Category = "Measurement")
    void StartMeasurement(EMapMeasurementType Type);

    // Add a point to current measurement
    UFUNCTION(BlueprintCallable, Category = "Measurement")
    void AddMeasurementPoint(const FVector& WorldPosition);

    // Finish current measurement
    UFUNCTION(BlueprintCallable, Category = "Measurement")
    void FinishMeasurement();

    // Cancel current measurement
    UFUNCTION(BlueprintCallable, Category = "Measurement")
    void CancelMeasurement();

    // Clear all measurements
    UFUNCTION(BlueprintCallable, Category = "Measurement")
    void ClearAllMeasurements();

    // Remove a measurement
    UFUNCTION(BlueprintCallable, Category = "Measurement")
    bool RemoveMeasurement(int32 Index);

    // Calculate straight line distance between two points
    UFUNCTION(BlueprintPure, Category = "Measurement")
    static float CalculateDistance(const FVector& PointA, const FVector& PointB);

    // Calculate distance along a path
    UFUNCTION(BlueprintPure, Category = "Measurement")
    static float CalculatePathDistance(const TArray<FVector>& Points);

    // Calculate navigation path distance
    UFUNCTION(BlueprintCallable, Category = "Measurement", meta = (WorldContext = "WorldContextObject"))
    static float CalculateNavigationDistance(
        UObject* WorldContextObject,
        const FVector& StartPoint,
        const FVector& EndPoint
    );

    // Calculate polygon area
    UFUNCTION(BlueprintPure, Category = "Measurement")
    static float CalculatePolygonArea(const TArray<FVector>& Points);

    // Calculate circle area from radius point
    UFUNCTION(BlueprintPure, Category = "Measurement")
    static float CalculateCircleArea(const FVector& Center, const FVector& RadiusPoint);

    // Format distance for display
    UFUNCTION(BlueprintPure, Category = "Measurement")
    FText FormatDistance(float DistanceInCm) const;

    // Format area for display
    UFUNCTION(BlueprintPure, Category = "Measurement")
    FText FormatArea(float AreaInCm2) const;

    // Get world coordinates at map position
    UFUNCTION(BlueprintPure, Category = "Measurement")
    FVector GetWorldCoordinates(const FVector& WorldPosition) const;

    // Format coordinates for display
    UFUNCTION(BlueprintPure, Category = "Measurement")
    FText FormatCoordinates(const FVector& WorldPosition) const;

protected:
    // Update current measurement value
    void UpdateCurrentMeasurement();

    // Calculate measurement value based on type
    float CalculateMeasurementValue(const FMapMeasurement& Measurement);
};
