// Copyright 2025 Snow Game Studio.

#include "ObjectPlacementHelper.h"

bool ObjectPlacementHelper::IsPlacementValid(
	const FVector& Location,
	const TArray<FWorldObjectData>& ExistingObjects,
	float MinDistance)
{
	const float MinDistanceSq = MinDistance * MinDistance;

	for (const FWorldObjectData& Obj : ExistingObjects)
	{
		if (FVector::DistSquared2D(Location, Obj.Location) < MinDistanceSq)
		{
			return false;
		}
	}
	return true;
}

float ObjectPlacementHelper::GetMinDistanceToObjects(
	const FVector& Location,
	const TArray<FWorldObjectData>& ExistingObjects)
{
	float MinDist = TNumericLimits<float>::Max();

	for (const FWorldObjectData& Obj : ExistingObjects)
	{
		const float Dist = FVector::Dist2D(Location, Obj.Location);
		if (Dist < MinDist)
		{
			MinDist = Dist;
		}
	}
	return MinDist;
}

bool ObjectPlacementHelper::FindOptimalPlacement(
	const FVector& Center,
	float SearchRadius,
	float MinDistance,
	const TArray<FWorldObjectData>& ExistingObjects,
	FRandomStream& Random,
	FVector& OutLocation,
	int32 MaxAttempts)
{
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Random angle and distance within search radius
		const float Angle = Random.FRandRange(0.f, 2.f * PI);
		const float Distance = Random.FRandRange(0.f, SearchRadius);

		FVector TestLocation = Center;
		TestLocation.X += FMath::Cos(Angle) * Distance;
		TestLocation.Y += FMath::Sin(Angle) * Distance;

		if (IsPlacementValid(TestLocation, ExistingObjects, MinDistance))
		{
			OutLocation = TestLocation;
			return true;
		}
	}

	return false;
}

void ObjectPlacementHelper::GeneratePoissonDiskPoints(
	float MinX,
	float MaxX,
	float MinY,
	float MaxY,
	float MinDistance,
	int32 Seed,
	TArray<FVector2D>& OutPoints,
	int32 MaxPoints)
{
	OutPoints.Empty();

	FRandomStream Random(Seed);

	// Grid cell size for spatial hashing
	const float CellSize = MinDistance / GRID_CELL_DIVISOR;
	const int32 GridWidth = FMath::CeilToInt((MaxX - MinX) / CellSize);
	const int32 GridHeight = FMath::CeilToInt((MaxY - MinY) / CellSize);

	// Grid stores index to OutPoints (-1 = empty)
	TArray<int32> Grid;
	Grid.SetNumZeroed(GridWidth * GridHeight);
	for (int32& Cell : Grid)
	{
		Cell = -1;
	}

	// Active list for processing
	TArray<int32> ActiveList;

	// Lambda to get grid index
	auto GetGridIndex = [&](const FVector2D& Point) -> int32
	{
		const int32 GX = FMath::Clamp(FMath::FloorToInt((Point.X - MinX) / CellSize), 0, GridWidth - 1);
		const int32 GY = FMath::Clamp(FMath::FloorToInt((Point.Y - MinY) / CellSize), 0, GridHeight - 1);
		return GY * GridWidth + GX;
	};

	// Lambda to check if point is valid
	auto IsPointValid = [&](const FVector2D& Point) -> bool
	{
		// Check bounds
		if (Point.X < MinX || Point.X >= MaxX || Point.Y < MinY || Point.Y >= MaxY)
		{
			return false;
		}

		const int32 GX = FMath::FloorToInt((Point.X - MinX) / CellSize);
		const int32 GY = FMath::FloorToInt((Point.Y - MinY) / CellSize);

		// Check neighboring cells (5x5 grid around current cell)
		const int32 SearchRadius = 2;
		for (int32 DY = -SearchRadius; DY <= SearchRadius; ++DY)
		{
			for (int32 DX = -SearchRadius; DX <= SearchRadius; ++DX)
			{
				const int32 NX = GX + DX;
				const int32 NY = GY + DY;

				if (NX >= 0 && NX < GridWidth && NY >= 0 && NY < GridHeight)
				{
					const int32 NeighborIndex = Grid[NY * GridWidth + NX];
					if (NeighborIndex != -1)
					{
						const float DistSq = FVector2D::DistSquared(Point, OutPoints[NeighborIndex]);
						if (DistSq < MinDistance * MinDistance)
						{
							return false;
						}
					}
				}
			}
		}

		return true;
	};

	// Generate initial point
	const FVector2D InitialPoint(
		Random.FRandRange(MinX, MaxX),
		Random.FRandRange(MinY, MaxY)
	);

	OutPoints.Add(InitialPoint);
	Grid[GetGridIndex(InitialPoint)] = 0;
	ActiveList.Add(0);

	// Process active list
	const int32 MaxSamplesPerPoint = 30;

	while (ActiveList.Num() > 0 && (MaxPoints == 0 || OutPoints.Num() < MaxPoints))
	{
		// Pick random point from active list
		const int32 ActiveIndex = Random.RandRange(0, ActiveList.Num() - 1);
		const int32 PointIndex = ActiveList[ActiveIndex];
		const FVector2D& CurrentPoint = OutPoints[PointIndex];

		bool bFoundValid = false;

		// Try to find valid point around current point
		for (int32 Sample = 0; Sample < MaxSamplesPerPoint; ++Sample)
		{
			const float Angle = Random.FRandRange(0.f, 2.f * PI);
			const float Distance = Random.FRandRange(MinDistance, MinDistance * 2.f);

			const FVector2D NewPoint(
				CurrentPoint.X + FMath::Cos(Angle) * Distance,
				CurrentPoint.Y + FMath::Sin(Angle) * Distance
			);

			if (IsPointValid(NewPoint))
			{
				const int32 NewIndex = OutPoints.Num();
				OutPoints.Add(NewPoint);
				Grid[GetGridIndex(NewPoint)] = NewIndex;
				ActiveList.Add(NewIndex);
				bFoundValid = true;
				break;
			}
		}

		// If no valid point found, remove from active list
		if (!bFoundValid)
		{
			ActiveList.RemoveAtSwap(ActiveIndex);
		}
	}
}

bool ObjectPlacementHelper::IsWithinBounds(
	const FVector& Location,
	const FVector& MinBounds,
	const FVector& MaxBounds)
{
	return Location.X >= MinBounds.X && Location.X <= MaxBounds.X &&
		Location.Y >= MinBounds.Y && Location.Y <= MaxBounds.Y &&
		Location.Z >= MinBounds.Z && Location.Z <= MaxBounds.Z;
}

void ObjectPlacementHelper::GetObjectsInRadius(
	const FVector& Location,
	float Radius,
	const TArray<FWorldObjectData>& AllObjects,
	TArray<FWorldObjectData>& OutNearbyObjects)
{
	OutNearbyObjects.Empty();
	const float RadiusSq = Radius * Radius;

	for (const FWorldObjectData& Obj : AllObjects)
	{
		if (FVector::DistSquared(Location, Obj.Location) <= RadiusSq)
		{
			OutNearbyObjects.Add(Obj);
		}
	}
}

float ObjectPlacementHelper::CalculateObjectDensity(
	const FVector& Center,
	float Radius,
	const TArray<FWorldObjectData>& Objects)
{
	int32 Count = 0;
	const float RadiusSq = Radius * Radius;

	for (const FWorldObjectData& Obj : Objects)
	{
		if (FVector::DistSquared2D(Center, Obj.Location) <= RadiusSq)
		{
			Count++;
		}
	}

	const float Area = PI * Radius * Radius;
	return Area > 0.f ? static_cast<float>(Count) / Area : 0.f;
}

FVector ObjectPlacementHelper::JitterLocation(
	const FVector& Location,
	float MaxJitter,
	FRandomStream& Random)
{
	return FVector(
		Location.X + Random.FRandRange(-MaxJitter, MaxJitter),
		Location.Y + Random.FRandRange(-MaxJitter, MaxJitter),
		Location.Z
	);
}

int32 ObjectPlacementHelper::GenerateCluster(
	const FVector& Center,
	float ClusterRadius,
	int32 ObjectCount,
	float MinDistance,
	const TArray<FWorldObjectData>& ExistingObjects,
	FRandomStream& Random,
	TArray<FVector>& OutLocations)
{
	OutLocations.Empty();
	OutLocations.Reserve(ObjectCount);

	// Create temporary array combining existing objects with newly placed ones
	TArray<FWorldObjectData> AllObjects = ExistingObjects;

	int32 PlacedCount = 0;
	const int32 MaxAttemptsPerObject = 30;

	for (int32 i = 0; i < ObjectCount; ++i)
	{
		bool bPlaced = false;

		for (int32 Attempt = 0; Attempt < MaxAttemptsPerObject; ++Attempt)
		{
			// Random position within cluster radius
			const float Angle = Random.FRandRange(0.f, 2.f * PI);
			const float Distance = Random.FRandRange(0.f, ClusterRadius);

			FVector TestLocation = Center;
			TestLocation.X += FMath::Cos(Angle) * Distance;
			TestLocation.Y += FMath::Sin(Angle) * Distance;

			if (IsPlacementValid(TestLocation, AllObjects, MinDistance))
			{
				OutLocations.Add(TestLocation);

				// Add to tracking array
				FWorldObjectData NewObj;
				NewObj.Location = TestLocation;
				AllObjects.Add(NewObj);

				PlacedCount++;
				bPlaced = true;
				break;
			}
		}

		if (!bPlaced)
		{
			// Could not place this object, continue to next
			continue;
		}
	}

	return PlacedCount;
}
