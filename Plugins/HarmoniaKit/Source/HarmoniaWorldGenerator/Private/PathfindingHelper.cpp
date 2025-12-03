// Copyright 2025 Snow Game Studio.

#include "PathfindingHelper.h"

bool PathfindingHelper::FindPath(
	const FVector& Start,
	const FVector& End,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config,
	FPathfindingResult& OutResult)
{
	OutResult = FPathfindingResult();

	// Convert to grid coordinates
	const FIntPoint StartGrid = WorldToGrid(Start);
	const FIntPoint EndGrid = WorldToGrid(End);

	// Validate bounds
	if (StartGrid.X < 0 || StartGrid.X >= Config.SizeX ||
		StartGrid.Y < 0 || StartGrid.Y >= Config.SizeY ||
		EndGrid.X < 0 || EndGrid.X >= Config.SizeX ||
		EndGrid.Y < 0 || EndGrid.Y >= Config.SizeY)
	{
		OutResult.ErrorMessage = TEXT("Start or end position out of bounds");
		return false;
	}

	// A* algorithm
	TMap<FIntPoint, FPathfindingNode> OpenSet;
	TMap<FIntPoint, FPathfindingNode> ClosedSet;
	TArray<FIntPoint> OpenHeap;

	// Initialize start node
	FPathfindingNode StartNode;
	StartNode.Position = StartGrid;
	StartNode.GCost = 0.f;
	StartNode.HCost = Heuristic(StartGrid, EndGrid);
	StartNode.Parent = FIntPoint(-1, -1);
	StartNode.bIsWater = IsWater(StartGrid.X, StartGrid.Y, HeightData, Config);

	OpenSet.Add(StartGrid, StartNode);
	OpenHeap.HeapPush(StartGrid, [&OpenSet](const FIntPoint& A, const FIntPoint& B)
	{
		return OpenSet[A].FCost() < OpenSet[B].FCost();
	});

	const int32 MaxIterations = Config.SizeX * Config.SizeY;
	int32 Iterations = 0;

	while (OpenHeap.Num() > 0 && Iterations < MaxIterations)
	{
		Iterations++;

		// Get node with lowest F cost
		FIntPoint CurrentPos;
		OpenHeap.HeapPop(CurrentPos, [&OpenSet](const FIntPoint& A, const FIntPoint& B)
		{
			return OpenSet[A].FCost() < OpenSet[B].FCost();
		});

		FPathfindingNode CurrentNode = OpenSet[CurrentPos];
		OpenSet.Remove(CurrentPos);
		ClosedSet.Add(CurrentPos, CurrentNode);

		// Check if reached goal
		if (CurrentPos == EndGrid)
		{
			// Reconstruct path
			TArray<FIntPoint> GridPath;
			ReconstructPath(ClosedSet, EndGrid, GridPath);

			// Convert to world positions
			OutResult.PathPoints.Reserve(GridPath.Num());
			for (const FIntPoint& GridPos : GridPath)
			{
				OutResult.PathPoints.Add(GridToWorld(GridPos, HeightData, Config));
			}

			// Calculate total distance
			for (int32 i = 1; i < OutResult.PathPoints.Num(); ++i)
			{
				OutResult.TotalDistance += FVector::Dist(
					OutResult.PathPoints[i - 1],
					OutResult.PathPoints[i]
				);
			}

			OutResult.bSuccess = true;
			return true;
		}

		// Expand neighbors
		TArray<FIntPoint> Neighbors;
		GetNeighbors(CurrentPos, Config, Neighbors);

		for (const FIntPoint& NeighborPos : Neighbors)
		{
			// Skip if in closed set
			if (ClosedSet.Contains(NeighborPos))
			{
				continue;
			}

			// Calculate costs
			const float MoveCost = CalculateMovementCost(CurrentPos, NeighborPos, HeightData, Config);
			const float TentativeGCost = CurrentNode.GCost + MoveCost;

			// Check if this path is better
			FPathfindingNode* ExistingNode = OpenSet.Find(NeighborPos);
			if (ExistingNode)
			{
				if (TentativeGCost < ExistingNode->GCost)
				{
					ExistingNode->GCost = TentativeGCost;
					ExistingNode->Parent = CurrentPos;
				}
			}
			else
			{
				// Add new node
				FPathfindingNode NewNode;
				NewNode.Position = NeighborPos;
				NewNode.GCost = TentativeGCost;
				NewNode.HCost = Heuristic(NeighborPos, EndGrid);
				NewNode.Parent = CurrentPos;
				NewNode.bIsWater = IsWater(NeighborPos.X, NeighborPos.Y, HeightData, Config);

				OpenSet.Add(NeighborPos, NewNode);
				OpenHeap.HeapPush(NeighborPos, [&OpenSet](const FIntPoint& A, const FIntPoint& B)
				{
					return OpenSet[A].FCost() < OpenSet[B].FCost();
				});
			}
		}
	}

	OutResult.ErrorMessage = TEXT("No path found");
	return false;
}

bool PathfindingHelper::FindRoadPath(
	const FVector& Start,
	const FVector& End,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config,
	int32 MaxBridgeLength,
	FPathfindingResult& OutResult)
{
	// First find the basic path
	if (!FindPath(Start, End, HeightData, Config, OutResult))
	{
		return false;
	}

	// Detect water crossings
	TArray<TPair<int32, int32>> BridgeSegments;
	OutResult.BridgeCount = DetectWaterCrossings(OutResult.PathPoints, HeightData, Config, BridgeSegments);

	// Create segments with bridge information
	OutResult.Segments.Reserve(OutResult.PathPoints.Num() - 1);

	for (int32 i = 0; i < OutResult.PathPoints.Num() - 1; ++i)
	{
		FPathSegment Segment;
		Segment.StartPoint = OutResult.PathPoints[i];
		Segment.EndPoint = OutResult.PathPoints[i + 1];
		Segment.Length = FVector::Dist(Segment.StartPoint, Segment.EndPoint);

		// Check if this segment is part of a bridge
		for (const TPair<int32, int32>& Bridge : BridgeSegments)
		{
			if (i >= Bridge.Key && i <= Bridge.Value)
			{
				Segment.bIsBridge = true;
				break;
			}
		}

		// Calculate slope
		const float HeightDiff = FMath::Abs(Segment.EndPoint.Z - Segment.StartPoint.Z);
		const float HorizontalDist = FVector::Dist2D(Segment.StartPoint, Segment.EndPoint);
		Segment.AverageSlope = HorizontalDist > 0.f ?
			FMath::RadiansToDegrees(FMath::Atan(HeightDiff / HorizontalDist)) : 0.f;

		OutResult.Segments.Add(Segment);
	}

	return true;
}

int32 PathfindingHelper::DetectWaterCrossings(
	const TArray<FVector>& PathPoints,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config,
	TArray<TPair<int32, int32>>& OutBridgeSegments)
{
	OutBridgeSegments.Empty();

	int32 WaterCrossings = 0;
	int32 WaterStartIndex = -1;
	bool bInWater = false;

	for (int32 i = 0; i < PathPoints.Num(); ++i)
	{
		const FIntPoint GridPos = WorldToGrid(PathPoints[i]);

		// Check bounds
		if (GridPos.X < 0 || GridPos.X >= Config.SizeX ||
			GridPos.Y < 0 || GridPos.Y >= Config.SizeY)
		{
			continue;
		}

		const bool bCurrentIsWater = IsWater(GridPos.X, GridPos.Y, HeightData, Config);

		if (bCurrentIsWater && !bInWater)
		{
			// Entering water
			bInWater = true;
			WaterStartIndex = i > 0 ? i - 1 : 0;
		}
		else if (!bCurrentIsWater && bInWater)
		{
			// Exiting water
			bInWater = false;
			OutBridgeSegments.Add(TPair<int32, int32>(WaterStartIndex, i));
			WaterCrossings++;
		}
	}

	// Handle case where path ends in water
	if (bInWater && WaterStartIndex >= 0)
	{
		OutBridgeSegments.Add(TPair<int32, int32>(WaterStartIndex, PathPoints.Num() - 1));
		WaterCrossings++;
	}

	return WaterCrossings;
}

void PathfindingHelper::SmoothPath(
	const TArray<FVector>& PathPoints,
	int32 Subdivision,
	TArray<FVector>& OutSmoothedPath)
{
	OutSmoothedPath.Empty();

	if (PathPoints.Num() < 2)
	{
		OutSmoothedPath = PathPoints;
		return;
	}

	// Reserve approximate size
	OutSmoothedPath.Reserve(PathPoints.Num() * Subdivision);

	for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
	{
		// Get four control points for Catmull-Rom
		const FVector& P0 = PathPoints[FMath::Max(0, i - 1)];
		const FVector& P1 = PathPoints[i];
		const FVector& P2 = PathPoints[i + 1];
		const FVector& P3 = PathPoints[FMath::Min(PathPoints.Num() - 1, i + 2)];

		// Add interpolated points
		for (int32 j = 0; j < Subdivision; ++j)
		{
			const float T = static_cast<float>(j) / static_cast<float>(Subdivision);
			OutSmoothedPath.Add(CatmullRom(P0, P1, P2, P3, T));
		}
	}

	// Add final point
	OutSmoothedPath.Add(PathPoints.Last());
}

float PathfindingHelper::CalculateMovementCost(
	const FIntPoint& From,
	const FIntPoint& To,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config)
{
	// Base cost (diagonal vs cardinal)
	const bool bDiagonal = (From.X != To.X) && (From.Y != To.Y);
	float Cost = bDiagonal ? DIAGONAL_COST : 1.0f;

	// Get heights
	const int32 FromIndex = From.Y * Config.SizeX + From.X;
	const int32 ToIndex = To.Y * Config.SizeX + To.X;

	if (FromIndex >= 0 && FromIndex < HeightData.Num() &&
		ToIndex >= 0 && ToIndex < HeightData.Num())
	{
		const float FromHeight = static_cast<float>(HeightData[FromIndex]) / 65535.f;
		const float ToHeight = static_cast<float>(HeightData[ToIndex]) / 65535.f;

		// Slope cost
		const float HeightDiff = FMath::Abs(ToHeight - FromHeight);
		Cost += HeightDiff * SLOPE_COST_MULTIPLIER * Config.MaxHeight / 100.f;

		// Water cost
		if (ToHeight <= Config.SeaLevel)
		{
			Cost *= WATER_COST_MULTIPLIER;
		}
	}

	return Cost;
}

bool PathfindingHelper::IsWater(
	int32 X,
	int32 Y,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config)
{
	const int32 Index = Y * Config.SizeX + X;
	if (Index >= 0 && Index < HeightData.Num())
	{
		const float Height = static_cast<float>(HeightData[Index]) / 65535.f;
		return Height <= Config.SeaLevel;
	}
	return false;
}

FIntPoint PathfindingHelper::WorldToGrid(const FVector& WorldPos, float TileSize)
{
	return FIntPoint(
		FMath::RoundToInt(WorldPos.X / TileSize),
		FMath::RoundToInt(WorldPos.Y / TileSize)
	);
}

FVector PathfindingHelper::GridToWorld(
	const FIntPoint& GridPos,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config,
	float TileSize)
{
	const int32 Index = GridPos.Y * Config.SizeX + GridPos.X;
	float Height = 0.f;

	if (Index >= 0 && Index < HeightData.Num())
	{
		Height = static_cast<float>(HeightData[Index]) / 65535.f * Config.MaxHeight;
	}

	return FVector(
		GridPos.X * TileSize,
		GridPos.Y * TileSize,
		Height + 10.f  // Slightly above terrain
	);
}

void PathfindingHelper::GetNeighbors(
	const FIntPoint& Current,
	const FWorldGeneratorConfig& Config,
	TArray<FIntPoint>& OutNeighbors)
{
	OutNeighbors.Empty();
	OutNeighbors.Reserve(8);

	// 8-directional movement
	static const FIntPoint Directions[] = {
		FIntPoint(-1, -1), FIntPoint(0, -1), FIntPoint(1, -1),
		FIntPoint(-1,  0),                   FIntPoint(1,  0),
		FIntPoint(-1,  1), FIntPoint(0,  1), FIntPoint(1,  1)
	};

	for (const FIntPoint& Dir : Directions)
	{
		const FIntPoint Neighbor = Current + Dir;

		// Check bounds
		if (Neighbor.X >= 0 && Neighbor.X < Config.SizeX &&
			Neighbor.Y >= 0 && Neighbor.Y < Config.SizeY)
		{
			OutNeighbors.Add(Neighbor);
		}
	}
}

float PathfindingHelper::Heuristic(const FIntPoint& A, const FIntPoint& B)
{
	// Euclidean distance heuristic
	const float DX = static_cast<float>(B.X - A.X);
	const float DY = static_cast<float>(B.Y - A.Y);
	return FMath::Sqrt(DX * DX + DY * DY);
}

void PathfindingHelper::ReconstructPath(
	const TMap<FIntPoint, FPathfindingNode>& ClosedSet,
	const FIntPoint& EndPos,
	TArray<FIntPoint>& OutPath)
{
	OutPath.Empty();

	FIntPoint CurrentPos = EndPos;

	while (CurrentPos.X >= 0 && CurrentPos.Y >= 0)
	{
		OutPath.Insert(CurrentPos, 0);

		const FPathfindingNode* CurrentNode = ClosedSet.Find(CurrentPos);
		if (CurrentNode)
		{
			CurrentPos = CurrentNode->Parent;
		}
		else
		{
			break;
		}
	}
}

FVector PathfindingHelper::CatmullRom(
	const FVector& P0,
	const FVector& P1,
	const FVector& P2,
	const FVector& P3,
	float T)
{
	const float T2 = T * T;
	const float T3 = T2 * T;

	return 0.5f * (
		(2.f * P1) +
		(-P0 + P2) * T +
		(2.f * P0 - 5.f * P1 + 4.f * P2 - P3) * T2 +
		(-P0 + 3.f * P1 - 3.f * P2 + P3) * T3
		);
}
