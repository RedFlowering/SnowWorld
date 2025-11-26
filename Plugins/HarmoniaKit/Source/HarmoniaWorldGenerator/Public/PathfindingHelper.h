// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "WorldGeneratorTypes.h"
#include "PathfindingHelper.generated.h"

/**
 * Pathfinding node for A* algorithm
 */
struct FPathfindingNode
{
	FIntPoint Position;
	float GCost = 0.f;  // Cost from start
	float HCost = 0.f;  // Heuristic cost to end
	float FCost() const { return GCost + HCost; }
	FIntPoint Parent = FIntPoint(-1, -1);
	bool bIsWater = false;

	bool operator<(const FPathfindingNode& Other) const
	{
		return FCost() > Other.FCost(); // Min-heap (lower cost = higher priority)
	}
};

/**
 * Path segment with metadata
 */
USTRUCT(BlueprintType)
struct HARMONIAWORLDGENERATOR_API FPathSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBridge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Length = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSlope = 0.f;
};

/**
 * Pathfinding result
 */
USTRUCT(BlueprintType)
struct HARMONIAWORLDGENERATOR_API FPathfindingResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> PathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPathSegment> Segments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BridgeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;
};

/**
 * Pathfinding Helper
 * Provides A* pathfinding for road and river generation
 */
class HARMONIAWORLDGENERATOR_API PathfindingHelper
{
public:
	/**
	 * Find path using A* algorithm with terrain awareness
	 * @param Start - Start world position
	 * @param End - End world position
	 * @param HeightData - Terrain heightmap
	 * @param Config - World generator config
	 * @param OutResult - Pathfinding result
	 * @return True if path found
	 */
	static bool FindPath(
		const FVector& Start,
		const FVector& End,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config,
		FPathfindingResult& OutResult
	);

	/**
	 * Find path avoiding water (for roads)
	 * Will create bridges if necessary
	 * @param Start - Start world position
	 * @param End - End world position
	 * @param HeightData - Terrain heightmap
	 * @param Config - World generator config
	 * @param MaxBridgeLength - Maximum bridge length (in tiles)
	 * @param OutResult - Pathfinding result
	 * @return True if path found
	 */
	static bool FindRoadPath(
		const FVector& Start,
		const FVector& End,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config,
		int32 MaxBridgeLength,
		FPathfindingResult& OutResult
	);

	/**
	 * Detect water crossings in a path
	 * @param PathPoints - Path points to check
	 * @param HeightData - Terrain heightmap
	 * @param Config - World generator config
	 * @param OutBridgeSegments - Indices of path segments that cross water
	 * @return Number of water crossings
	 */
	static int32 DetectWaterCrossings(
		const TArray<FVector>& PathPoints,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config,
		TArray<TPair<int32, int32>>& OutBridgeSegments
	);

	/**
	 * Smooth path using Catmull-Rom spline
	 * @param PathPoints - Original path points
	 * @param Subdivision - Number of subdivisions between points
	 * @param OutSmoothedPath - Smoothed path points
	 */
	static void SmoothPath(
		const TArray<FVector>& PathPoints,
		int32 Subdivision,
		TArray<FVector>& OutSmoothedPath
	);

	/**
	 * Calculate path cost for A* heuristic
	 * @param From - Start position
	 * @param To - End position
	 * @param HeightData - Terrain heightmap
	 * @param Config - World generator config
	 * @return Movement cost (higher = harder to traverse)
	 */
	static float CalculateMovementCost(
		const FIntPoint& From,
		const FIntPoint& To,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config
	);

	/**
	 * Check if position is water
	 * @param X - Grid X coordinate
	 * @param Y - Grid Y coordinate
	 * @param HeightData - Terrain heightmap
	 * @param Config - World generator config
	 * @return True if water
	 */
	static bool IsWater(
		int32 X,
		int32 Y,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config
	);

	/**
	 * Convert world position to grid coordinates
	 */
	static FIntPoint WorldToGrid(const FVector& WorldPos, float TileSize = 100.f);

	/**
	 * Convert grid coordinates to world position
	 */
	static FVector GridToWorld(
		const FIntPoint& GridPos,
		const TArray<int32>& HeightData,
		const FWorldGeneratorConfig& Config,
		float TileSize = 100.f
	);

private:
	/**
	 * Get valid neighbors for A* expansion
	 */
	static void GetNeighbors(
		const FIntPoint& Current,
		const FWorldGeneratorConfig& Config,
		TArray<FIntPoint>& OutNeighbors
	);

	/**
	 * Heuristic function (Euclidean distance)
	 */
	static float Heuristic(const FIntPoint& A, const FIntPoint& B);

	/**
	 * Reconstruct path from closed set
	 */
	static void ReconstructPath(
		const TMap<FIntPoint, FPathfindingNode>& ClosedSet,
		const FIntPoint& EndPos,
		TArray<FIntPoint>& OutPath
	);

	/**
	 * Catmull-Rom spline interpolation
	 */
	static FVector CatmullRom(
		const FVector& P0,
		const FVector& P1,
		const FVector& P2,
		const FVector& P3,
		float T
	);

	// Cost multipliers
	static constexpr float SLOPE_COST_MULTIPLIER = 2.0f;
	static constexpr float WATER_COST_MULTIPLIER = 10.0f;
	static constexpr float DIAGONAL_COST = 1.41421356f;
};
