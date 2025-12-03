// Copyright 2025 Snow Game Studio.

/**
 * @file WorldGeneratorTypes_Additions.h
 * @brief 월드 생성 추가 타입 정의
 * 
 * WorldGeneratorTypes.h의 추가 타입들:
 * - 청크 생성 델리게이트
 * - Poisson Disk Sampling 설정
 * - 파라미터 고급 설정
 */

#pragma once
#include "WorldGeneratorTypes_Additions.generated.h"

//=============================================================================
// Delegate Additions
//=============================================================================

/**
 * Delegate for chunk generation
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChunkGenerated, FIntPoint, ChunkCoordinates, bool, bFromCache);

/**
 * Delegate for terrain modification
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTerrainModified, FVector, Location, float, Radius);

/**
 * Poisson Disk Sampling Settings
 */
USTRUCT(BlueprintType)
struct FPoissonDiskSettings
{
	GENERATED_BODY()

	// Minimum distance between samples
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoissonDisk", meta = (ClampMin = "10.0"))
	float MinDistance = 100.f;

	// Maximum samples to generate (0 = unlimited)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoissonDisk", meta = (ClampMin = "0"))
	int32 MaxSamples = 0;

	// Random seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoissonDisk")
	int32 Seed = 0;

	// Number of candidates per sample attempt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoissonDisk", meta = (ClampMin = "1", ClampMax = "100"))
	int32 CandidatesPerSample = 30;
};

/**
 * World Generation Statistics
 */
USTRUCT(BlueprintType)
struct FWorldGenerationStats
{
	GENERATED_BODY()

	// Heightmap generation time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float HeightmapTimeMs = 0.f;

	// Object generation time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float ObjectTimeMs = 0.f;

	// Biome generation time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float BiomeTimeMs = 0.f;

	// Erosion simulation time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float ErosionTimeMs = 0.f;

	// Total generation time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float TotalTimeMs = 0.f;

	// Number of objects generated
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 ObjectCount = 0;

	// Number of tiles processed
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int32 TilesProcessed = 0;

	// Peak memory usage (bytes)
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	int64 PeakMemoryUsage = 0;
};

/**
 * Terrain Analysis Data
 */
USTRUCT(BlueprintType)
struct FTerrainAnalysisData
{
	GENERATED_BODY()

	// Grid position
	UPROPERTY(BlueprintReadOnly)
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	// Height (normalized 0-1)
	UPROPERTY(BlueprintReadOnly)
	float Height = 0.f;

	// Slope angle (degrees)
	UPROPERTY(BlueprintReadOnly)
	float Slope = 0.f;

	// Slope direction (radians)
	UPROPERTY(BlueprintReadOnly)
	float SlopeDirection = 0.f;

	// Curvature (-1 to 1, negative = concave, positive = convex)
	UPROPERTY(BlueprintReadOnly)
	float Curvature = 0.f;

	// Is water
	UPROPERTY(BlueprintReadOnly)
	bool bIsWater = false;

	// Is buildable (flat enough for structures)
	UPROPERTY(BlueprintReadOnly)
	bool bIsBuildable = false;
};

