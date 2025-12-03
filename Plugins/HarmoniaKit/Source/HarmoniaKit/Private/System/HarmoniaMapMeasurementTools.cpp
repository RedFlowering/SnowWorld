// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaMapMeasurementTools.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Engine/World.h"

UHarmoniaMapMeasurementTools::UHarmoniaMapMeasurementTools()
{
    bIsMeasuring = false;
    DistanceUnit = TEXT("m");
    UnitConversionFactor = 0.01f; // cm to meters
    DecimalPlaces = 2;
}

void UHarmoniaMapMeasurementTools::StartMeasurement(EMapMeasurementType Type)
{
    CurrentMeasurement = FMapMeasurement();
    CurrentMeasurement.MeasurementType = Type;
    CurrentMeasurement.Color = FLinearColor::Yellow;
    bIsMeasuring = true;

    UE_LOG(LogTemp, Log, TEXT("Started %s measurement"),
        *UEnum::GetValueAsString(Type));
}

void UHarmoniaMapMeasurementTools::AddMeasurementPoint(const FVector& WorldPosition)
{
    if (!bIsMeasuring)
    {
        return;
    }

    CurrentMeasurement.Points.Add(WorldPosition);
    UpdateCurrentMeasurement();

    UE_LOG(LogTemp, Log, TEXT("Added point: %s (Total: %d)"),
        *WorldPosition.ToString(), CurrentMeasurement.Points.Num());
}

void UHarmoniaMapMeasurementTools::FinishMeasurement()
{
    if (!bIsMeasuring)
    {
        return;
    }

    // Require minimum points based on type
    int32 MinPoints = 2;
    if (CurrentMeasurement.MeasurementType == EMapMeasurementType::Area)
    {
        MinPoints = 3;
    }

    if (CurrentMeasurement.Points.Num() < MinPoints)
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough points for measurement (need %d, have %d)"),
            MinPoints, CurrentMeasurement.Points.Num());
        CancelMeasurement();
        return;
    }

    // Final calculation
    UpdateCurrentMeasurement();

    // Add to measurements list
    Measurements.Add(CurrentMeasurement);

    UE_LOG(LogTemp, Log, TEXT("Finished measurement: %s"),
        *FormatDistance(CurrentMeasurement.MeasuredValue).ToString());

    // Reset
    CurrentMeasurement = FMapMeasurement();
    bIsMeasuring = false;
}

void UHarmoniaMapMeasurementTools::CancelMeasurement()
{
    CurrentMeasurement = FMapMeasurement();
    bIsMeasuring = false;

    UE_LOG(LogTemp, Log, TEXT("Cancelled measurement"));
}

void UHarmoniaMapMeasurementTools::ClearAllMeasurements()
{
    Measurements.Empty();
    CancelMeasurement();
}

bool UHarmoniaMapMeasurementTools::RemoveMeasurement(int32 Index)
{
    if (Measurements.IsValidIndex(Index))
    {
        Measurements.RemoveAt(Index);
        return true;
    }
    return false;
}

float UHarmoniaMapMeasurementTools::CalculateDistance(const FVector& PointA, const FVector& PointB)
{
    return FVector::Dist(PointA, PointB);
}

float UHarmoniaMapMeasurementTools::CalculatePathDistance(const TArray<FVector>& Points)
{
    if (Points.Num() < 2)
    {
        return 0.0f;
    }

    float TotalDistance = 0.0f;
    for (int32 i = 1; i < Points.Num(); ++i)
    {
        TotalDistance += FVector::Dist(Points[i - 1], Points[i]);
    }

    return TotalDistance;
}

float UHarmoniaMapMeasurementTools::CalculateNavigationDistance(
    UObject* WorldContextObject,
    const FVector& StartPoint,
    const FVector& EndPoint)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return CalculateDistance(StartPoint, EndPoint);
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
    {
        return CalculateDistance(StartPoint, EndPoint);
    }

    // Find path
    FPathFindingQuery Query;
    Query.StartLocation = StartPoint;
    Query.EndLocation = EndPoint;
    Query.NavData = NavSys->GetDefaultNavDataInstance();

    FPathFindingResult Result = NavSys->FindPathSync(Query);

    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        return Result.Path->GetLength();
    }

    // Fallback to straight line
    return CalculateDistance(StartPoint, EndPoint);
}

float UHarmoniaMapMeasurementTools::CalculatePolygonArea(const TArray<FVector>& Points)
{
    if (Points.Num() < 3)
    {
        return 0.0f;
    }

    // Use shoelace formula (2D projection on XY plane)
    float Area = 0.0f;
    int32 NumPoints = Points.Num();

    for (int32 i = 0; i < NumPoints; ++i)
    {
        int32 j = (i + 1) % NumPoints;
        Area += Points[i].X * Points[j].Y;
        Area -= Points[j].X * Points[i].Y;
    }

    return FMath::Abs(Area) * 0.5f;
}

float UHarmoniaMapMeasurementTools::CalculateCircleArea(const FVector& Center, const FVector& RadiusPoint)
{
    float Radius = FVector::Dist(Center, RadiusPoint);
    return PI * Radius * Radius;
}

FText UHarmoniaMapMeasurementTools::FormatDistance(float DistanceInCm) const
{
    float ConvertedDistance = DistanceInCm * UnitConversionFactor;

    FString FormattedValue = FString::Printf(TEXT("%.*f"), DecimalPlaces, ConvertedDistance);

    return FText::FromString(FString::Printf(TEXT("%s %s"), *FormattedValue, *DistanceUnit));
}

FText UHarmoniaMapMeasurementTools::FormatArea(float AreaInCm2) const
{
    // Convert to display unit squared
    float ConvertedArea = AreaInCm2 * UnitConversionFactor * UnitConversionFactor;

    FString FormattedValue = FString::Printf(TEXT("%.*f"), DecimalPlaces, ConvertedArea);
    FString AreaUnit = FString::Printf(TEXT("%s²"), *DistanceUnit);

    return FText::FromString(FString::Printf(TEXT("%s %s"), *FormattedValue, *AreaUnit));
}

FVector UHarmoniaMapMeasurementTools::GetWorldCoordinates(const FVector& WorldPosition) const
{
    return WorldPosition;
}

FText UHarmoniaMapMeasurementTools::FormatCoordinates(const FVector& WorldPosition) const
{
    // Format as X, Y, Z with conversion
    float X = WorldPosition.X * UnitConversionFactor;
    float Y = WorldPosition.Y * UnitConversionFactor;
    float Z = WorldPosition.Z * UnitConversionFactor;

    FString Formatted = FString::Printf(
        TEXT("X: %.*f, Y: %.*f, Z: %.*f %s"),
        DecimalPlaces, X,
        DecimalPlaces, Y,
        DecimalPlaces, Z,
        *DistanceUnit
    );

    return FText::FromString(Formatted);
}

void UHarmoniaMapMeasurementTools::UpdateCurrentMeasurement()
{
    CurrentMeasurement.MeasuredValue = CalculateMeasurementValue(CurrentMeasurement);

    // Generate label
    switch (CurrentMeasurement.MeasurementType)
    {
    case EMapMeasurementType::Distance:
    case EMapMeasurementType::PathDistance:
        CurrentMeasurement.Label = FormatDistance(CurrentMeasurement.MeasuredValue);
        break;

    case EMapMeasurementType::Area:
    case EMapMeasurementType::Radius:
        CurrentMeasurement.Label = FormatArea(CurrentMeasurement.MeasuredValue);
        break;
    }
}

float UHarmoniaMapMeasurementTools::CalculateMeasurementValue(const FMapMeasurement& Measurement)
{
    switch (Measurement.MeasurementType)
    {
    case EMapMeasurementType::Distance:
        if (Measurement.Points.Num() == 2)
        {
            return CalculateDistance(Measurement.Points[0], Measurement.Points[1]);
        }
        return 0.0f;

    case EMapMeasurementType::PathDistance:
        return CalculatePathDistance(Measurement.Points);

    case EMapMeasurementType::Area:
        return CalculatePolygonArea(Measurement.Points);

    case EMapMeasurementType::Radius:
        if (Measurement.Points.Num() == 2)
        {
            return CalculateCircleArea(Measurement.Points[0], Measurement.Points[1]);
        }
        return 0.0f;

    default:
        return 0.0f;
    }
}
