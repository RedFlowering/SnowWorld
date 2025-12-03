// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "WorldGeneratorTypes.h"

/**
 * Object Placement Helper
 * Provides utility functions for validating and optimizing object placement
 */
class HARMONIAWORLDGENERATOR_API ObjectPlacementHelper
{
public:
	/**
	 * Check if placement is valid (no overlap with existing objects)
	 * @param Location - Target placement location
	 * @param ExistingObjects - Array of already placed objects
	 * @param MinDistance - Minimum required distance from other objects
	 * @return True if placement is valid
	 */
	static bool IsPlacementValid(
		const FVector& Location,
		const TArray<FWorldObjectData>& ExistingObjects,
		float MinDistance
	);

	/**
	 * Get minimum distance to any existing object
	 * @param Location - Target location
	 * @param ExistingObjects - Array of existing objects
	 * @return Minimum distance (MAX_FLT if no objects exist)
	 */
	static float GetMinDistanceToObjects(
		const FVector& Location,
		const TArray<FWorldObjectData>& ExistingObjects
	);

	/**
	 * Find optimal placement location using Poisson Disk Sampling
	 * @param Center - Center of search area
	 * @param SearchRadius - Radius to search within
	 * @param MinDistance - Minimum distance between objects
	 * @param ExistingObjects - Array of existing objects
	 * @param Random - Random stream for deterministic results
	 * @param OutLocation - Output optimal location
	 * @param MaxAttempts - Maximum placement attempts
	 * @return True if valid location found
	 */
	static bool FindOptimalPlacement(
		const FVector& Center,
		float SearchRadius,
		float MinDistance,
		const TArray<FWorldObjectData>& ExistingObjects,
		FRandomStream& Random,
		FVector& OutLocation,
		int32 MaxAttempts = 30
	);

	/**
	 * Generate Poisson Disk distributed points
	 * @param MinX - Minimum X bound
	 * @param MaxX - Maximum X bound
	 * @param MinY - Minimum Y bound
	 * @param MaxY - Maximum Y bound
	 * @param MinDistance - Minimum distance between points
	 * @param Seed - Random seed
	 * @param OutPoints - Generated points
	 * @param MaxPoints - Maximum number of points to generate (0 = unlimited)
	 */
	static void GeneratePoissonDiskPoints(
		float MinX,
		float MaxX,
		float MinY,
		float MaxY,
		float MinDistance,
		int32 Seed,
		TArray<FVector2D>& OutPoints,
		int32 MaxPoints = 0
	);

	/**
	 * Check if location is within bounds
	 * @param Location - Location to check
	 * @param MinBounds - Minimum bounds
	 * @param MaxBounds - Maximum bounds
	 * @return True if within bounds
	 */
	static bool IsWithinBounds(
		const FVector& Location,
		const FVector& MinBounds,
		const FVector& MaxBounds
	);

	/**
	 * Get objects within radius of a location
	 * @param Location - Center location
	 * @param Radius - Search radius
	 * @param AllObjects - All objects to search
	 * @param OutNearbyObjects - Objects within radius
	 */
	static void GetObjectsInRadius(
		const FVector& Location,
		float Radius,
		const TArray<FWorldObjectData>& AllObjects,
		TArray<FWorldObjectData>& OutNearbyObjects
	);

	/**
	 * Calculate density of objects in an area
	 * @param Center - Center of area
	 * @param Radius - Radius of area
	 * @param Objects - Objects to count
	 * @return Density (objects per square unit)
	 */
	static float CalculateObjectDensity(
		const FVector& Center,
		float Radius,
		const TArray<FWorldObjectData>& Objects
	);

	/**
	 * Jitter placement location for natural variation
	 * @param Location - Original location
	 * @param MaxJitter - Maximum jitter amount
	 * @param Random - Random stream
	 * @return Jittered location
	 */
	static FVector JitterLocation(
		const FVector& Location,
		float MaxJitter,
		FRandomStream& Random
	);

	/**
	 * Cluster objects around a center point
	 * @param Center - Cluster center
	 * @param ClusterRadius - Radius of cluster
	 * @param ObjectCount - Number of objects to place
	 * @param MinDistance - Minimum distance between objects
	 * @param ExistingObjects - Existing objects to avoid
	 * @param Random - Random stream
	 * @param OutLocations - Generated cluster locations
	 * @return Number of successfully placed objects
	 */
	static int32 GenerateCluster(
		const FVector& Center,
		float ClusterRadius,
		int32 ObjectCount,
		float MinDistance,
		const TArray<FWorldObjectData>& ExistingObjects,
		FRandomStream& Random,
		TArray<FVector>& OutLocations
	);

private:
	/** Grid cell size for spatial hashing (Poisson Disk) */
	static constexpr float GRID_CELL_DIVISOR = 1.41421356f; // sqrt(2)
};
